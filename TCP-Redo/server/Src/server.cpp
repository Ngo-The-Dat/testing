#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <thread>
#include <cstdlib>

#include "message.hpp"
#include "serve_list.hpp"

// Winsock library
#pragma comment(lib, "ws2_32.lib")

int serverNumber = 0;
#define MAX_PENDING 10

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
    char buffer[RECIEVE_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];

    std::string message = "Welcome to the server! you are client " + std::to_string(clientId) + '\n';
    int receive_message_size;


    // only short message allowed here
    short_message wellcome = make_short_message(message);
    char* data = reinterpret_cast<char*>(&wellcome);
    if (send(client_socket, data, sizeof(short_message), 0) == SOCKET_ERROR) {
        std::cout << "Failed to send message: " << WSAGetLastError() << std::endl;
        return;
    }
    
    while (true)
    {
        // only short message allowed here
        receive_message_size = recv(client_socket, buffer, RECIEVE_BUFFER_SIZE, 0);
        if (receive_message_size < 0) {
            std::cerr << "Error receiving data\n";
            break;
        }

        short_message req;
        bool ok = copy_buffer_to_message(buffer, receive_message_size, req);

        if (!ok) continue;
        string cont = get_content_short(req);
        if (!is_valid_message(cont)) continue;

        if (cont == "GET_LIST") {
            std::cout << "serving list...\n";
            serve_list(client_socket);
        }

        if (cont == "DOWNLOAD_FILE") {
            check_file_to_download(client_socket);
        }

        if (cont == "WORKER_GET_CHUNK") { // getting the file chunk from worker socket
            serve_chunk(client_socket);
        }

        if (cont == "QUIT") {
            break;
        }

        std::cout << "hehe\n"; 

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

        std::cout << "New connection accepted from " << inet_ntoa(client.sin_addr) << std::endl;

        // Handle client in a separate thread
        std::thread(handle_client, client_socket, numClient ++).detach();
    }

    close_socket(server_socket);
    return 0;
}
