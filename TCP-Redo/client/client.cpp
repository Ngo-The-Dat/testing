#include <iostream>
#include <string>
#include <winsock2.h>
#include <stdexcept>
#include <memory>
#include <map>
#include "receive_file.hpp"
#include "../message.hpp"
// Winsock library
#pragma comment(lib, "ws2_32.lib")

class Client {
public:

    Client(const std::string& serverIp, unsigned short serverPort)
        : serverIp(serverIp), serverPort(serverPort), socketHandle(INVALID_SOCKET) {}

    ~Client() {
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

        std::cout << "Connected to " << serverIp << ":" << serverPort << "\n";
    }

    void run() {
        std::string message;
        char buffer[RECIEVE_BUFFER_SIZE];
        char sendBuffer[SEND_BUFFER_SIZE];

        
        int bytesReceived = recv(socketHandle, buffer, RECIEVE_BUFFER_SIZE, 0);
        std::cout << bytesReceived << '\n';
        
        if (bytesReceived == SOCKET_ERROR) {
            throw std::runtime_error("Failed to receive message: " + std::to_string(WSAGetLastError()));
        }   

        if (bytesReceived != sizeof(short_message)) {
            throw std::runtime_error("fail protocol");
        }

        for (int i = 0; i < bytesReceived; i ++) cout << (int) buffer[i] << ' ';
        cout << '\n';

        short_message wellcome;
        bool ok = copy_buffer_to_message(buffer, bytesReceived, wellcome);
        if (!ok) {
            
            std::cout << "not ok\n";
            throw std::runtime_error("not ok get server wellcome");
        }
        std::cout << "[Server]: " << wellcome.len << ' ' << get_content_short(wellcome) << '\n';
        std::cout << "--- end wellcome ---\n";
        do {

            std::cout << "Command: [list] or [download] (ex: list)\n";
            std::cout << "[Choice]: ";
            getline(cin, message);
            if (message == "list") {
                get_file_list(socketHandle);
            }

            if (message == "download") {
                handle_download(socketHandle);
            }
        } while (message != "QUIT");
    }

private:
    std::string serverIp;
    unsigned short serverPort;
    SOCKET socketHandle;
    WSADATA wsaData;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>\n";
        return 1;
    }

    try {
        std::string serverIp = argv[1];
        unsigned short serverPort = static_cast<unsigned short>(std::stoi(argv[2]));

        Client client(serverIp, serverPort);
        client.initialize();
        client.connectToServer();
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

