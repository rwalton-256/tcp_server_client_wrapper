/**
 * @file test.cpp
 * @author Ryan Walton
 * @brief 
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <tcp_server_client_wrapper/server.hpp>
#include <tcp_server_client_wrapper/client.hpp>

#include <csignal>
#include <iostream>
#include <functional>
#include <cstring>

#include <netdb.h>

std::condition_variable cv;

int main()
{
    tcp_wrapper::Server server( 5005 );

    struct hostent* server_name;
    in_addr server_addr;
    server_name = gethostbyname( "localhost" );

    memcpy( &server_addr, server_name->h_addr, sizeof( server_addr ) );

    tcp_wrapper::Client client( server_addr, 5005 );

    server.wait_for_connection( std::chrono::seconds( 15 ) );

    std::cout << "Attempting write..." << std::endl;

    client.write( (void*)"Hello world\n", 12 );

    std::cout << "Attempting read..." << std::endl;

    char buf[5];
    buf[4] = '\0';
    std::cout << server.read( buf, 4 ) << std::endl;
    std::cout << buf << std::endl;

    signal( SIGINT, []( int signal ){ cv.notify_one(); } );

    std::mutex m;
    std::unique_lock<std::mutex> ul( m );
    cv.wait( ul );
    std::cout << std::endl;

    return 0;
}
