/**
 * @file server.hpp
 * @author Ryan Walton
 * @brief 
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <tcp_server_client_wrapper/endpoint.hpp>

#include <netinet/in.h>

namespace tcp_wrapper
{

class Server : public Endpoint
{
private:
    int _mServerSock;
    void _mConnectionFunc();
    std::chrono::duration<double> _mTimeout;
public:
    Server( in_port_t _aPort, std::chrono::duration<double> _aTimeout );
    ~Server();
};

}
