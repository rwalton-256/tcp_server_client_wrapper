/**
 * @file error.hpp
 * @author Ryan Walton
 * @brief 
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <exception>
#include <string>

namespace tcp_wrapper
{

class Error : public std::exception
{
    std::string _mWhat;
public:
    Error( const std::string& _aWhat ) : _mWhat( _aWhat ) {}
    const char* what() const _GLIBCXX_USE_NOEXCEPT
    {
        return _mWhat.c_str();
    }
};

class ErrorWithCode : public Error
{
public:
    ErrorWithCode( const std::string& _aWhat, int _aErrno ) : Error( _aWhat + ", errno: " + std::to_string( _aErrno ) ) {}
};

}//tcp_wrapper
