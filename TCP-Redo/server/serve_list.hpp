#pragma once
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <winsock2.h>
#include <exception>
#include <thread>
#include <cstdlib>
#include <fstream>
#include "../message.hpp"

void serve_file(SOCKET client_socket, const string& filename, const string& showname) {
    
    char buffer[RECIEVE_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
    
    std::ifstream fi(filename, std::ios::binary | std::ios::ate); // Open in binary mode and move to end of file
    int tmpS = fi.tellg(), cur = 0;

    start_file_transfer start_sending;
    start_sending.file_size = tmpS; 
    start_sending.len = showname.size();
    strcpy(start_sending.filename, showname.c_str());

    std::cout << "starting to send....\n";
    if (send(client_socket, reinterpret_cast<char*>(&start_sending), sizeof(start_file_transfer), 0) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to send message: " + std::to_string(WSAGetLastError()));
    }
    std::cout << "done send....\n";

    int rByte = recv(client_socket, buffer, RECIEVE_BUFFER_SIZE, 0);
    if (rByte < 0) {
        throw std::runtime_error("Failed to recv message: " + std::to_string(WSAGetLastError()));
    }

    short_message acc;
    copy_buffer_to_message(buffer, rByte, acc);

    if (!is_valid_message(get_content_short(acc)) || get_content_short(acc) != "OK") {
        throw std::runtime_error("Failed to accept message: " + std::to_string(WSAGetLastError()));
    }

    std::cout << "RECIEVED OK\n";

    data_message send_data;

    fi.seekg(ios::beg);
    while (cur < tmpS) {
        int next = DATA_LEN;
        if (cur + next > tmpS) next = tmpS - cur;

        send_data.len = next;
        fi.read(send_data.content, next);

        if (send(client_socket, reinterpret_cast<char*>(&send_data), sizeof(data_message), 0) == SOCKET_ERROR) {
            throw std::runtime_error("Failed to send message: " + std::to_string(WSAGetLastError()));
        }
        //std::cout << "sent: " << next << " bytes\n";

        cur += next;
    }

    fi.close();

    std::cout << "sent the file " + filename << '\n';
/*
    short_message last_ok;
    rByte = recv(client_socket, reinterpret_cast<char*>(&last_ok), sizeof(short_message), 0);
    if (rByte < 0) {
        throw std::runtime_error("Fail on last acc");
    }

    if (get_content(last_ok.content, last_ok.len) != "OK") {
        std::cout << "OPPS\n";
    } 
*/
}

void serve_list(SOCKET client_socket) {
    const string filename = "input.txt";
    serve_file(client_socket, filename, filename);
}

void get_file_to_download(SOCKET client_socket) {
    char buffer[RECIEVE_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
       
    
    start_file_transfer rec;
    int rByte = recv(client_socket, buffer, RECIEVE_BUFFER_SIZE, 0);
    if (rByte < 0) {
        throw std::runtime_error("Failed to recv message: " + std::to_string(WSAGetLastError()));
    }

    if (!copy_buffer_to_message(buffer, rByte, rec)) {
        std::cout << "Wrong protocols\n";
        return;
    }



    ifstream fin("input.txt");
    if (!fin.is_open()) {
        std::cout << "No current data from server\n";
        return;
    }

    map <string, long long> filelist;

    string name;
    long long size;
    while (fin >> name >> size) {
        filelist[name] = size;    
        std::cout << name << ' ' << size << '\n';
    }
    fin.close();


    string client_wants =  get_content(rec.filename, rec.len);
    std::cout << "File client wants: " << client_wants << ' ' << rec.file_size << '\n';
    if (!filelist.count(client_wants) || filelist[client_wants] != rec.file_size) {
        std::cout << "server have no file\n";
    
        short_message reject = make_short_message("NO");
        if (send(client_socket, reinterpret_cast<char*>(&reject), sizeof(short_message), 1) == SOCKET_ERROR) {
            throw runtime_error("Cannot reject");
        }
    
        return;
    }

    short_message acc = make_short_message("OK");
    if (send(client_socket, reinterpret_cast<char*>(&acc), sizeof(short_message), 1) == SOCKET_ERROR) {
        throw runtime_error("Cannot accept");
    }
 
    string path = "Files/" + client_wants;
    
    std::cout << "downloading " << client_wants << '\n';
    serve_file(client_socket, path, client_wants);
}

void get_file_chunk(SOCKET client_socket) {
    
}