#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <exception>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <map>

#include "../message.hpp"

void recieve_file(SOCKET server, const string& filename, const string& rename) {
    
    char buffer[RECIEVE_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
    start_file_transfer need_file;
    std::cout << "recieving...\n";

    int rByte = recv(server, buffer, RECIEVE_BUFFER_SIZE - 10, 0);
    if (rByte < 0) {
        throw std::runtime_error("Failed to recv message: " + std::to_string(WSAGetLastError()));
    }
    

    std::cout << "entered\n" << rByte << ' ' << sizeof(start_file_transfer) << '\n';

    if (!copy_buffer_to_message(buffer, rByte, need_file)) {
        throw std::runtime_error("Wrong file transfer protocol");
    }
    cout << "getting the file: ";
    cout << get_content(need_file.filename, need_file.len) << '\n';

    if (get_content(need_file.filename, need_file.len) != filename) {
        std::cout << "Not the required file\n";
        return;
    }

    
    short_message ok = make_short_message("OK");
    if (send(server, reinterpret_cast<char*>(&ok), sizeof(ok), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot get List asdfja;sdfjalksdf heheehehe\n");
    }    

    std::cout << "send get list\n";

    ofstream fout;
    fout.open(rename.c_str(), ios::binary);
    long long total_size = 0;
    long long need_size = need_file.file_size;
    int rec_size = sizeof(data_message);

    data_message rec;

    while (total_size < need_size) {
        rByte = recv(server, reinterpret_cast<char*>(&rec), rec_size, 0);
        fout.write(rec.content, rec.len);
        total_size += rec.len;
    }

    fout.close();

    std::cout << "i got the file" << '\n';
}

void get_file_list(SOCKET server) {
    short_message getlist = make_short_message("GET_LIST");
    if (send(server, reinterpret_cast<char*>(&getlist), sizeof(short_message), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot get List\n");
    }
    recieve_file(server, "input.txt", "ready.txt");
}

void get_any_file(SOCKET server, const string& filename, long long filesize) {
    short_message getlist = make_short_message("DOWNLOAD_FILE");
    if (send(server, reinterpret_cast<char*>(&getlist), sizeof(short_message), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot download\n");
    }

    start_file_transfer start;
    start.file_size = filesize;
    start.len = filename.size();
    strcpy(start.filename, filename.c_str());

    if (send(server, reinterpret_cast<char*>(&start), sizeof(start_file_transfer), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot download file\n");
    }

    short_message res;
    int rbyte = recv(server, reinterpret_cast<char*>(&res), sizeof(short_message), 0);
    if (rbyte < 0) {
        throw runtime_error("Cannot recieve respond\n");
    }

    if (get_content(res.content, res.len) != "OK") {
        cout << "server rejected\n";
        return ;
    }

    recieve_file(server, filename, filename);
}


void handle_download(SOCKET server) {
    ifstream fin("ready.txt");
    if (!fin.is_open()) {
        std::cout << "No current data from server\n";
        return;
    }

    map <string, long long> filelist;

    string name;
    long long size;
    while (fin >> name >> size) {
        filelist[name] = size;
        cout << "Filename & size: " << name << "\t\t" << size << '\n';
    }
    fin.close();

    std::cout << "[Choice]: ";
    getline(cin, name);

    get_any_file(server, name, filelist[name]);
}
