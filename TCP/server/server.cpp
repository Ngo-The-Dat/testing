#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <thread>
#include <cstdlib>
#include "../protocols.hpp"
// Winsock library
#pragma comment(lib, "ws2_32.lib")

#define MAX_PENDING 50
#define RECEIVE_BUFFER_SIZE 1024
const int SEND_BUFFER_SIZE = 8 * (1 << 10); // 8KB

const int MAX_BUFFER_SIZE = 1 << 12;

int serverNumber = 0;

void error(const std::string &message)
{
    std::cerr << message << ": " << WSAGetLastError() << std::endl;
    WSACleanup();
    exit(1);
}

void close_socket(SOCKET so)
{
    closesocket(so);
    WSACleanup();
}

void handle_client(SOCKET client_socket, int clientId)
{
    char buffer[RECEIVE_BUFFER_SIZE];
    std::string message = "Welcome to the server! you are " + std::to_string(clientId) + '\n';
    int receive_message_size;

    // Send welcome message
    send(client_socket, message.c_str(), message.length() + 1, 0);

    while (message != "QUIT")
    {
        receive_message_size = recv(client_socket, buffer, RECEIVE_BUFFER_SIZE, 0);
        message = buffer;
        if (receive_message_size < 0) {
            std::cout << receive_message_size << '\n';
            error("Error receiving data");
        }
        std::cout << "recieved from client\n";
        Message* recieved = getMessage(buffer);
        std::string sendMsg = "default\n";
        
        if (recieved != NULL) { 
            sendMsg = recieved->to_string();
            if (recieved->type() == REQUEST_DOWNLOAD) {
                std::cout << "Client want to download\n";
            }

            if (recieved->type() == REQUEST_LIST) {
                std::cout << "Client want to have list\n";
            }
        }

        std::cout << "To send " + sendMsg << '\n';
        send(client_socket, sendMsg.c_str(), sendMsg.length() + 1, 0);
        std::cout << "sent\n";
    }

    std::cout << "Closing client socket..." << clientId << std::endl;
    closesocket(client_socket);
}

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    unsigned short port;
    sockaddr_in server{}, client{}; // : A socket address structure specifically for IPv4 addresses.
    int client_length;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    port = static_cast<unsigned short>(std::stoi(argv[2]));

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        error("Failed WSAStartup");

    std::cout << "Winsock Initialised." << std::endl;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
        error("Failed to create socket");

    std::cout << "Socket created." << std::endl;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    server.sin_port = htons(port);

    if (bind(server_socket, reinterpret_cast<sockaddr *>(&server), sizeof(server)) == SOCKET_ERROR)
        error("Bind failed");

    if (listen(server_socket, MAX_PENDING) < 0)
        error("Listen failed");

    std::cout << "Waiting for incoming connections..." << std::endl;
    int numClient = 0;
    while (true)
    {
        client_length = sizeof(client);
        client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&client), &client_length);
        if (client_socket == INVALID_SOCKET)
        {
            error("Error on accept");
        }

        std::cout << "New connection accepted from fuck " << inet_ntoa(client.sin_addr) << std::endl;

        // Handle client in a separate thread
        std::thread(handle_client, client_socket, numClient ++).detach();
    }

    close_socket(server_socket);
    return 0;
}
