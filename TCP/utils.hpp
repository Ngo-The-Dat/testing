/* protocols

request type:
    message size = 1 bytes

    get file list
    download chunk file

*/


#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <vector>

// Winsock library
#pragma comment(lib, "ws2_32.lib")

const int MAX_BUFFER_SIZE = 1 << 12;

using namespace std;

char* getFile(std::string filename, int off_begin, int len) {
    const std::string filePath = "Files/" + filename;
    ifstream file;
    file.open(filePath.c_str(), ios::binary | ios::in);
    if (!file.is_open()) {
        return NULL;
    }
    

    if (len > MAX_BUFFER_SIZE) len = MAX_BUFFER_SIZE;
    char* ret = new char[len];
    file.seekg(len, ios::beg);
    file.read(ret, len);

    return ret;
}

