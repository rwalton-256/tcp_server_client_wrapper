/**
 * @file endpoint.hpp
 * @author Ryan Walton
 * @brief 
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <tcp_server_client_wrapper/error.hpp>

#include <linux/socket.h>
#include <netinet/in.h>

#include <thread>
#include <atomic>
#include <string>
#include <exception>
#include <condition_variable>
#include <mutex>
#include <functional>

namespace tcp_wrapper
{

class Endpoint
{
public:
    class Initialization_Error : public ErrorWithCode
    {
    public:
        Initialization_Error( const std::string& _aWhat, int _aErrno ) : ErrorWithCode( _aWhat, _aErrno ) {}
    };
    class Connection_Error : public ErrorWithCode
    {
    public:
        Connection_Error( const std::string& _aWhat, int _aErrno ) : ErrorWithCode( _aWhat, _aErrno ) {}
    };
    class Connection_Reset : public Error
    {
    public:
        Connection_Reset( const std::string& _aWhat ) : Error( _aWhat  ) {}
    };
protected:
    enum State
    {
        State_Uninit,
        State_Init,
        State_Connected,
        State_Shutdown
    };
    int _mConnectionSock;
    std::thread _mConnectionThread;
    std::atomic<State> _mState;
    std::condition_variable _mStateCv;
    std::mutex _mMutex;
    Endpoint( std::function<void(void)> _aConnectionFunc );
public:
    bool wait_for_connection( std::chrono::duration<long> _aTimeout );
    ssize_t read( void* _aBuf, size_t _aLen );
    ssize_t write( void* _aBuf, size_t _aLen );
};

}//tcp_wrapper
