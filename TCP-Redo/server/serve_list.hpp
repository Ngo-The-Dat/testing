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
}

map <string, unsigned long long> get_available_file() {
    map <string, unsigned long long> filelist;
    
    ifstream fin("input.txt");
    if (!fin.is_open()) {
        std::cout << "No current data from server\n";
    }

    string name;
    unsigned long long size;
    while (fin >> name >> size) filelist[name] = size;    
    fin.close();

    return filelist;
}

void serve_list(SOCKET client_socket) {
    const string filename = "input.txt";
    serve_file(client_socket, filename, filename);
}

void check_file_to_download(SOCKET client_socket) {
    short_message server_hello = make_short_message("OK");
    send(server_hello, client_socket, "server_send");
    std::cout << "server hello sent\n";

    start_file_transfer target;
    int rbyte = recv(target, client_socket, "cannot get the requirment");

    string filename = get_content(target.filename, target.len);
    unsigned long long filesize = target.file_size;

    auto filelist = get_available_file();
    
    short_message ack;
    if (!filelist.count(filename) || filelist[filename] != filesize) {
        ack = make_short_message("NO");
        send(ack, client_socket, "Cannot send reject\n");
        return;
    }

    ack = make_short_message("OK");
    send(ack, client_socket, "Cannot accept\n");
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

void serve_chunk(SOCKET client_socket) {
    short_message server_hello = make_short_message("OK");
    send(server_hello, client_socket, "server_send");
    std::cout << "server chunk ok sent\n";

    start_chunk_transfer target;
    int rbyte = recv(target, client_socket, "cannot get the requirment");

    string filename = get_content(target.filename, target.len);
    unsigned long long filesize = target.file_size;
    unsigned long long offset = target.offset;
    unsigned long long length = target.offset_lenght;

    auto filelist = get_available_file();
    
    short_message ack;
    if (!filelist.count(filename) || filelist[filename] != filesize || offset + length > filesize) {
        ack = make_short_message("NO");
        send(ack, client_socket, "Cannot send reject\n");
        return;
    }


    ack = make_short_message("OK");
    filename = "Files/" + filename;
    ifstream fin;
    fin.open(filename.c_str(), ios::binary);
    if (!fin.is_open()) {
        ack = make_short_message("NO");
        std::cout << "error open file " + filename << '\n';
        send(ack, client_socket , "Cannot accept\n");
        return;
    }

    try {
        fin.seekg(offset, ios::beg);
    } catch (exception &e) {
        std::cout << e.what() << '\n';
        ack = make_short_message("NO");
        fin.close();
        send(ack, client_socket , "Cannot accept\n");
        return;
    }

    send(ack, client_socket , "Cannot accept\n");
    rbyte = recv(ack, client_socket, "ok");
    if (get_content_short(ack) != "OK") {
        return;
    }

    int cur = 0;
    int need_send = length;

    data_message send_data;

    while (cur < need_send) {
        int next = DATA_LEN;
        if (cur + next > need_send) next = need_send - cur;

        send_data.len = next;
        fin.read(send_data.content, next);
        send(send_data, client_socket, "fail to send");
        cur += next;
    }

    std::cout << "sent successfully\n";
    fin.close();
}