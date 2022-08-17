/**
 * @file client.hpp
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

class Client : public Endpoint
{
private:
    void _mConnectionFunc();
    in_addr _mAddress;
    in_port_t _mPort;
public:
    Client( in_addr _aAddress, in_port_t _aPort=5002 );
    ~Client();
};

}
