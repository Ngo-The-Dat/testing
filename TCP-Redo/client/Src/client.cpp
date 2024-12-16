#include <signal.h>
#include <iostream>
#include <atomic>
#include <string>
#include <set>
#include <winsock2.h>
#include <stdexcept>
#include <memory>
#include <map>
#include "receive_file.hpp"
#include "file_manipulate.hpp"
#include "message.hpp"
#include "TUI.hpp"
// Winsock library
#pragma comment(lib, "ws2_32.lib")

enum client_state {
    RUNNING,
    STOP
};

static client_state currstate = RUNNING;

void exit_on_signal(int signum) {
    if (signum == 2) currstate = STOP;
}

void update_clock(clientUI& ui) {
    int next_update = 5000;
    int interval = 10;
    while (next_update > 0 && currstate == RUNNING) {
        ui.set_next_update(next_update);
        next_update -= interval;
        Sleep(interval);
    }
}

clientUI ui;

void call_render_ui(clientUI& ui) {
    while (currstate == RUNNING) {
        if (!ui.in_progress) ui.display();
        Sleep(100);
    }
}

class Client {

    vector <string> downloaded_files;

public:

    Client(const std::string& serverIp, unsigned short serverPort)
        : serverIp(serverIp), serverPort(serverPort), socketHandle(INVALID_SOCKET) {}

    ~Client() {
        
        short_message quit = make_short_message("QUIT");
        if (socketHandle != INVALID_SOCKET) {
            send(socketHandle, reinterpret_cast<char*>(&quit), sizeof(short_message), 0);
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

        std::cout << "\n[Connected to " << serverIp << ":" << serverPort << "]\n";
    }

    void run() {
        std::string message;
        char buffer[RECIEVE_BUFFER_SIZE];
        char sendBuffer[SEND_BUFFER_SIZE];

        
        int bytesReceived = recv(socketHandle, buffer, RECIEVE_BUFFER_SIZE, 0);
        
        if (bytesReceived == SOCKET_ERROR) {
            throw std::runtime_error("Failed to receive message: " + std::to_string(WSAGetLastError()));
        }   

        if (bytesReceived != sizeof(short_message)) {
            throw std::runtime_error("fail protocol");
        }

        short_message wellcome;
        bool ok = copy_buffer_to_message(buffer, bytesReceived, wellcome);
        if (!ok) {
            throw std::runtime_error("not ok get server wellcome");
        }
        
        ui.set_server_info(serverIp, serverPort); 
        ui.set_chunk_progress(1, 0);
        ui.set_chunk_progress(2, 0);
        ui.set_chunk_progress(3, 0);
        ui.set_chunk_progress(4, 0);
        ui.set_total_progress(0);
        ui.set_combine_progress(0);
        ui.set_file_name("  None  ");

        ofstream lout("Logs/system.log");

        get_file_list(socketHandle, lout, ui);

        thread render_ui(call_render_ui, ref(ui));        
        signal(SIGINT, exit_on_signal);



        do {
            thread clock(update_clock, ref(ui));
            if (!compare_file_set(downloaded_files, "input.txt")) {
                // get_file_list(socketHandle, lout, ui);
                ui.set_message("[Please wait for the file to download]");   
                handle_download(socketHandle, serverIp, serverPort, downloaded_files, lout, ui);
                ui.set_message("[Press 'ctrl' + 'c' to disconnect]");
                lout << "Press ctrl + c to exit\n";
            }
            clock.join();
        } while (currstate == RUNNING);

        lout.close();
        render_ui.join();
    }

private:
    std::string serverIp;
    unsigned short serverPort;
    SOCKET socketHandle;
    WSADATA wsaData;
};



int main(int argc, char* argv[]) {

    system("cls");
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

