#pragma once
#include "Message.hpp"
#include "client/request.hpp"
#include "server/respond.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

Message* getMessage(const char* data) {
    try {
        if (REQUEST_LIST == data[0]) 
            return new RequestList(data);
        if (REQUEST_DOWNLOAD == data[0])
            return new RequestChunk(data);
        if (DEFAULT == data[0]) 
            return new Message();
    } catch (std::exception &error) {
        std::cerr << error.what() << '\n'; 
    }
    return NULL;
}
