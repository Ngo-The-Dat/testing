#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <exception>
#include <thread>
#include <cstdlib>
#include <fstream>

// Winsock library
#pragma comment(lib, "ws2_32.lib")
using namespace std;

class Worker {
public:

    Worker(const std::string& serverIp, unsigned short serverPort)
        : serverIp(serverIp), serverPort(serverPort), socketHandle(INVALID_SOCKET) {
           // std::cout << "New worker to connect: " << serverIp << ' ' << serverPort << '\n'; 

        }

    ~Worker() {
        if (socketHandle != INVALID_SOCKET) {
            closesocket(socketHandle);
        }
        WSACleanup();
    }

    void initialize() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock: " + std::to_string(WSAGetLastError()));
        }

        socketHandle = socket(AF_INET, SOCK_STREAM, 0);
        if (socketHandle == INVALID_SOCKET) {
            throw std::runtime_error("Error creating socket: " + std::to_string(WSAGetLastError()));
        }
    }

    void connectToServer() {
        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(serverIp.c_str());
        server.sin_port = htons(serverPort);
        if (connect(socketHandle, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
            throw std::runtime_error("Failed to connect to server: " + std::to_string(WSAGetLastError()));
        }
        //std::cout << "\n[Connected to " << serverIp << ":" << serverPort << "]\n";
    }


    void run();
    void get_file(string filename, unsigned long long offset, unsigned long long len, unsigned long long filesize, int part, unsigned long long& progress, ofstream& fout);


private:
    std::string serverIp;
    unsigned short serverPort;
    SOCKET socketHandle;
    WSADATA wsaData;
};
