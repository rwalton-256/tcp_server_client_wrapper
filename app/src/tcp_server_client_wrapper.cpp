/**
 * @file tcp_server_client_wrapper.cpp
 * @author Ryan Walton
 * @brief 
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <tcp_server_client_wrapper/endpoint.hpp>
#include <tcp_server_client_wrapper/server.hpp>
#include <tcp_server_client_wrapper/client.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <cassert>
#include <iostream>

namespace tcp_wrapper
{

Endpoint::Endpoint( std::function<void(void)> _aConnectionFunc )
    :
    _mConnectionThread( _aConnectionFunc ),
    _mState( State_Uninit )
{

}

Server::Server( in_port_t _aPort, std::chrono::duration<double> _aTimeout )
    :
    Endpoint( [&](){ _mConnectionFunc(); } ),
    _mTimeout( _aTimeout )
{
    std::lock_guard<std::mutex> lg( _mMutex );

    _mServerSock = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( _mServerSock <= 0 ) throw Initialization_Error( "Create", errno );

    int ret;
    const int enable = 1;
    ret = ::setsockopt( _mServerSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) );
    if( ret != 0 ) throw Initialization_Error( "Setsockopt SO_REUSEADDR", errno );

    struct sockaddr_in address =
    {
        .sin_family = AF_INET,
        .sin_port = htons( _aPort ),
        .sin_addr = 
        {
            .s_addr = INADDR_ANY
        }
    };

    ret = ::bind( _mServerSock, (const sockaddr*)(&address), sizeof( address ) );
    if( ret != 0 ) throw Initialization_Error( "Bind", errno );

    _mState.store( State_Init );
    _mStateCv.notify_all();
}

Server::~Server()
{
    _mState = State_Shutdown;
    _mStateCv.notify_all();

    ::shutdown( _mServerSock, SHUT_RDWR );
    _mConnectionThread.join();
    ::close( _mServerSock );
}

void Server::_mConnectionFunc()
{
    {
        std::unique_lock<std::mutex> ul( _mMutex );
        _mStateCv.wait( ul, [&](){ return _mState.load() != State_Uninit; } );
    }

    while( _mState.load() != State_Shutdown )
    {
        try
        {
            int ret = ::listen( _mServerSock, 1 );
            if( ret != 0 ) throw Connection_Error( "Listen", errno );

            sockaddr address;
            socklen_t length;

            std::cout << "Accepting connections..." << std::endl;
            _mConnectionSock = ::accept( _mServerSock, &address, &length );
            if( _mState.load() == State_Shutdown ) break;
            if( _mConnectionSock <= 0 ) throw Connection_Error( "Accept", errno );

            std::chrono::seconds timeout_sec = std::chrono::duration_cast<std::chrono::seconds>( _mTimeout );
            std::chrono::microseconds timeout_us = std::chrono::duration_cast<std::chrono::microseconds>( _mTimeout - timeout_sec );
            timeval tv =
            {
                .tv_sec = static_cast<time_t>( timeout_sec.count() ),
                .tv_usec =  static_cast<time_t>( timeout_us.count() )
            };

            ret = ::setsockopt( _mConnectionSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
            if( ret != 0 ) throw Initialization_Error( "Receive Timeout", errno );
            ret = ::setsockopt( _mConnectionSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof( tv ) );
            if( ret != 0 ) throw Initialization_Error( "Send Timeout", errno );

            _mState.store( State_Connected );
            _mStateCv.notify_all();
        }
        catch( const Error& e )
        {
            if( _mState.load() == State_Shutdown )
            {
                break;
            }
            else
            {
                throw e;
            }
        }

        std::cout << "Accepted client!" << std::endl;

        {
            std::unique_lock<std::mutex> ul( _mMutex );
            _mStateCv.wait( ul, [&](){ return ( _mState.load() == State_Shutdown ) || ( _mState.load() == State_Init ); } );
        }

        std::cout << "Client connection reset" << std::endl;

        ::shutdown( _mConnectionSock, SHUT_RDWR );
        ::close( _mConnectionSock );
    }
}

Client::Client( in_addr _aAddress, in_port_t _aPort, std::chrono::duration<double> _aTimeout )
    :
    Endpoint( [&](){ _mConnectionFunc(); } ),
    _mAddress( _aAddress ),
    _mPort( _aPort ),
    _mTimeout( _aTimeout )
{
    std::lock_guard<std::mutex> lg( _mMutex );

    _mState.store( State_Init );
    _mStateCv.notify_all();
}

Client::~Client()
{
    _mState = State_Shutdown;
    _mStateCv.notify_all();

    _mConnectionThread.join();
}

void Client::_mConnectionFunc()
{
    {
        std::unique_lock<std::mutex> ul( _mMutex );
        _mStateCv.wait( ul, [&](){ return _mState.load() != State_Uninit; } );
    }

    std::cout << "Beginning Connection..." << std::endl;

    while( _mState.load() != State_Shutdown )
    {
        try
        {
            _mConnectionSock = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
            if( _mConnectionSock <= 0 ) throw Initialization_Error( "Create", errno );

            struct sockaddr_in address =
            {
                .sin_family = AF_INET,
                .sin_port = htons( _mPort ),
                .sin_addr = _mAddress
            };

            int ret = ::connect( _mConnectionSock, (const sockaddr*)&address, sizeof( address ) );
            if( ret != 0 ) throw Connection_Error( "Connect", errno );

            std::chrono::seconds timeout_sec = std::chrono::duration_cast<std::chrono::seconds>( _mTimeout );
            std::chrono::microseconds timeout_us = std::chrono::duration_cast<std::chrono::microseconds>( _mTimeout - timeout_sec );
            timeval tv =
            {
                .tv_sec = static_cast<time_t>( timeout_sec.count() ),
                .tv_usec =  static_cast<time_t>( timeout_us.count() )
            };

            ret = ::setsockopt( _mConnectionSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
            if( ret != 0 ) throw Initialization_Error( "Receive Timeout", errno );
            ret = ::setsockopt( _mConnectionSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof( tv ) );
            if( ret != 0 ) throw Initialization_Error( "Send Timeout", errno );

            _mState.store( State_Connected );
            _mStateCv.notify_all();
        }
        catch( const Error& e )
        {
            if( _mState.load() == State_Shutdown )
            {
                break;
            }
            else
            {
                throw e;
            }
        }

        std::cout << "Connection Success!" << std::endl;

        {
            std::unique_lock<std::mutex> ul( _mMutex );
            _mStateCv.wait( ul, [&](){ return ( _mState.load() == State_Shutdown ) || ( _mState.load() == State_Init ); } );
        }

        ::shutdown( _mConnectionSock, SHUT_WR );
        ::close( _mConnectionSock );
    }
}

bool Endpoint::wait_for_connection( std::chrono::duration<double> _aTimeout )
{
    std::unique_lock<std::mutex> ul( _mMutex );

    return _mStateCv.wait_for( ul, _aTimeout, [&](){ return _mState.load() == State_Connected; } );
}

ssize_t Endpoint::read( void* _aBuf, size_t _aLen )
{
    uint8_t* buf = (uint8_t*)_aBuf;
    if( _mState.load() != State_Connected )
    {
        throw Connection_Reset( "Read" );
    }
    ssize_t num_read = 0;
    while( 1 )
    {
        ssize_t ret = ::read( _mConnectionSock, buf, _aLen );
        if( ret > 0 )
        {
            num_read += ret;
            buf += ret;
            _aLen -= ret;
            if( _aLen == 0 )
            {
                return num_read;
            }
        }
        else if( ret == 0 )
        {
            // Connection reset
            if( _mState.load() == State_Connected )
            {
                _mState.store( State_Init );
                _mStateCv.notify_all();
            }
            throw Connection_Reset( "Read" );
        }
        else
        {
            if( errno == EAGAIN )
            {
                throw Timeout( "Read" );
            }
            throw Connection_Error( "Read", errno );
        }
    }
}

ssize_t Endpoint::write( void* _aBuf, size_t _aLen )
{
    uint8_t* buf = (uint8_t*)_aBuf;
    if( _mState.load() != State_Connected )
    {
        throw Connection_Reset( "Write" );
    }
    ssize_t num_written = 0;
    while( 1 )
    {
        ssize_t ret = ::write( _mConnectionSock, buf, _aLen );
        if( ret > 0 )
        {
            num_written += ret;
            buf += ret;
            _aLen -= ret;
            if( _aLen == 0 )
            {
                return num_written;
            }
        }
        else if( ret == 0 )
        {
            throw Error( "Unknown error, " + std::string( __FILE__ ) + ":" + std::to_string( __LINE__ ) );
        }
        else
        {
            if( errno == EAGAIN )
            {
                throw Timeout( "Write" );
            }
            throw Connection_Error( "Write", errno );
        }
    }
}

}//tcp_wrapper
