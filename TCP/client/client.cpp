#include <iostream>
#include <string>
#include <winsock2.h>
#include <stdexcept>
#include <memory>
#include "../protocols.hpp"
// Winsock library
#pragma comment(lib, "ws2_32.lib")

#define RECEIVE_BUFFER_SIZE 1000

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
        char buffer[RECEIVE_BUFFER_SIZE];

        do {
            int bytesReceived = recv(socketHandle, buffer, RECEIVE_BUFFER_SIZE, 0);
            std::cout << "i take it\n";
            if (bytesReceived == SOCKET_ERROR) {
                throw std::runtime_error("Failed to receive message: " + std::to_string(WSAGetLastError()));
            }



            buffer[bytesReceived] = '\0';
            std::cout << "[Server]: " << buffer;

            std::cout << "[You]: ";
            std::cin >> message;
            Message* sendMessage = NULL;
            
            if (message == "list") {
                sendMessage = new RequestList("input.txt", 9);
            } else if (message == "download") {
                sendMessage = new RequestChunk("testfile.txt", 200, 200);
            } else {
                sendMessage = new Message();
            }

            std::vector <char> data = sendMessage->get_send_message();
            char* sendBuffer = new char[data.size() + 1];
            for (int i = 0; i < data.size(); i ++) sendBuffer[i] = data[i];
            sendBuffer[data.size()] = '\0';
            std::cout << "sending...\n";
            if (send(socketHandle, sendBuffer, static_cast<int>(data.size()), 0) == SOCKET_ERROR) {
                throw std::runtime_error("Failed to send message: " + std::to_string(WSAGetLastError()));
                std::cout << "shit\n";
            }

            std::cout << "message sent\n";
            delete sendMessage;
            delete[] sendBuffer;

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

