#pragma once
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <time.h>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <exception>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <map>

#include "TUI.hpp"
#include "../message.hpp"

bool get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size, ofstream& lout, clientUI& ui);

void recieve_file(SOCKET server, const string& filename, const string& rename, ofstream& lout) {
    
    char buffer[RECIEVE_BUFFER_SIZE];
    char sendBuffer[SEND_BUFFER_SIZE];
    start_file_transfer need_file;
    lout << "recieving...\n";

    int rByte = recv(server, buffer, RECIEVE_BUFFER_SIZE - 10, 0);
    if (rByte < 0) {
        throw std::runtime_error("Failed to recv message: " + std::to_string(WSAGetLastError()));
    }
    

    lout << "entered\n" << rByte << ' ' << sizeof(start_file_transfer) << '\n';

    if (!copy_buffer_to_message(buffer, rByte, need_file)) {
        throw std::runtime_error("Wrong file transfer protocol");
    }
    lout << "getting the file: ";
    lout << get_content(need_file.filename, need_file.len) << '\n';

    if (get_content(need_file.filename, need_file.len) != filename) {
        lout << "Not the required file\n";
        return;
    }

    
    short_message ok = make_short_message("OK");
    if (send(server, reinterpret_cast<char*>(&ok), sizeof(ok), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot get List asdfja;sdfjalksdf heheehehe\n");
    }    

    lout << "send get list\n";

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

    lout << "i got the file" << '\n';
}

void get_file_list(SOCKET server, ofstream& lout) {
    short_message getlist = make_short_message("GET_LIST");
    if (send(server, reinterpret_cast<char*>(&getlist), sizeof(short_message), 0) == SOCKET_ERROR) {
        throw runtime_error("Cannot get List\n");
    }
    recieve_file(server, "input.txt", "ready.txt", lout);
}

void get_any_file(SOCKET server, const string& filename, long long filesize, ofstream& lout) {
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
        lout << "server rejected\n";
        return ;
    }

    recieve_file(server, filename, filename, lout);
}

void handle_each_file(SOCKET server, string serverIp, unsigned short serverPort, set <string>& downloaded_files, map<string, long long>& filelist, string name, ofstream& lout, clientUI& ui) {
    if (!filelist.count(name)) {
        lout << "No such file\n";
        return;
    }

    if (downloaded_files.count(name)) {
        lout << "File already downloaded\n";
        return;
    }

    short_message worker_hello = make_short_message("DOWNLOAD_FILE");
    send(worker_hello, server, "worker_send");

    short_message ack;
    int rbyt = recv(ack, server, "get server ack");

    if (get_content_short(ack) != "OK") {
        lout << "Server rejected at request\n";
        return;
    }

    start_file_transfer start_sending;
    strcpy(start_sending.filename, name.c_str());
    start_sending.len = name.size();
    start_sending.file_size = filelist[name];

    send(start_sending, server, "worker asked for file");

    rbyt = recv(ack, server, "get server ack");

    if (get_content_short(ack) != "OK") {
        lout << "Server rejected at get file\n";
        return;
    }


    // void get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size)

    //get_any_file(server, name, filelist[name]);
    bool status = get_download(name, serverIp, serverPort, 0, filelist[name], lout, ui);
    
    if (!status) {
        lout << "Failed to download\n";
        return;
    }
    downloaded_files.insert(name);
}

void handle_download(SOCKET server, string serverIp, unsigned short serverPort, set <string>& downloaded_files, ofstream& lout, clientUI& ui) {
    ifstream fin("ready.txt");
    if (!fin.is_open()) {
        lout << "No current data from server\n";
        return;
    }

    map <string, long long> filelist;

    string name;
    long long size;
    while (fin >> name >> size) {
        filelist[name] = size;
        lout << "Filename & size: " << name << "\t\t" << size << '\n';
    }
    fin.close();

    vector <pair<string, unsigned long long>> filelist_ui;
    for (auto& it : filelist) {
        filelist_ui.push_back({it.first, it.second});
    }
    ui.set_file_list(filelist_ui);

    fin.open("input.txt");
    if (!fin.is_open()) {
        return;
    }

    while (fin >> name) {
        handle_each_file(server, serverIp, serverPort, downloaded_files, filelist, name, lout, ui);
    }
}


#include "worker.hpp"

void Worker::run() {
    
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
}

void Worker::get_file(string filename, unsigned long long offset, unsigned long long len, unsigned long long filesize, int part, unsigned long long& progress, ofstream& lout) {

    short_message worker_hello = make_short_message("WORKER_GET_CHUNK");
    send(worker_hello, socketHandle, "worker_send");

    int rbyt = recv(worker_hello, socketHandle, "worker recv from server");

    if (get_content(worker_hello.content, worker_hello.len) != "OK") {
        lout << "Content: " << get_content(worker_hello.content, worker_hello.len) << '\n';
        lout << "Server rejected at hello\n";
        return;
    }

    start_chunk_transfer start_sending;
    strcpy(start_sending.filename, filename.c_str());
    start_sending.len = filename.size();
    start_sending.file_size = filesize;
    start_sending.offset = offset;
    start_sending.offset_lenght = len;

    send(start_sending, socketHandle, "worker asked for file");
    
    short_message ack;
    
    rbyt = recv(ack, socketHandle, "get server ack");

    if (get_content_short(ack) != "OK") {
        lout << "Server rejected at get chunk\n";
        return;
    }

    ack = make_short_message("OK");
    
    string file_part_name = "[" + to_string(part) + "]#" + filename;
    ofstream fout;
    fout.open(file_part_name.c_str(), ios::binary);

    if (!fout.is_open()) {
        ack = make_short_message("NO");
        fout << "error open file " << file_part_name << '\n';
        send(ack, socketHandle, "not ok");
        return;
    }

    send(ack, socketHandle, "ok");

    unsigned long long total_size = 0;
    unsigned long long need_size = len;

    data_message rec;

    while (total_size < need_size) {
        rbyt = recv(rec, socketHandle, to_string(part) + " file transfer");
        if (rbyt < 0) {
            fout.close();
            fout << "error recv on" + file_part_name << '\n';
            return;
        }
        fout.write(rec.content, rec.len);
        total_size += rec.len;
        progress += rec.len;
    }
    
    fout.close();
    lout << file_part_name << " Successully recieved\n";
    
}


bool get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size, ofstream& lout, clientUI& ui) {

    ui.set_file_name(filename);

    vector <Worker*> workers;
    const int num_worker = 4;

    for (int i = 0; i < num_worker; i ++) {
        Worker* worker = new Worker(ipAddress, port);
        worker->initialize();
        worker->connectToServer();
        worker->run();

        workers.push_back(worker);
    }

    unsigned long long chunkSize = size / num_worker;
    unsigned long long offset_assigned = 0;
 
    vector <thread> threads;
    vector <unsigned long long> progress(num_worker, 0);
    vector <unsigned long long> target_size(num_worker, 0);

    for (int i = 0; i < num_worker; i ++) {
        unsigned long long chunk = chunkSize;
        if (i == num_worker - 1) {
            chunk = size - offset_assigned;
        }
        // workers[i]->get_file(filename, offset_assigned, chunk, size, i + 1, progress[i]);

        target_size[i] = chunk;

        threads.push_back(thread(&Worker::get_file, workers[i], filename, offset_assigned, chunk, size, i + 1, ref(progress[i]), ref(lout)));
 
        offset_assigned += chunk;

    }

    while (true) {
        ui.in_progress = true;
        bool done = true;
        unsigned long long total_progress = 0;
        for (int i = 0; i < num_worker; i ++) {
            if (progress[i] < target_size[i]) {
                done = false;
            }

            total_progress += progress[i];

            int percent = ((double)progress[i] / (double)target_size[i]) * 100;
            lout << "Progress " << i + 1 << ": " << percent << 
            "%\t\t size: " << progress[i] << '/' << target_size[i] << '\n';
            ui.set_chunk_progress(i + 1, percent);
        }

        ui.set_total_progress((double)total_progress / (double)size * 100);

        ui.in_progress = false;
        if (done) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    for (int i = 0; i < num_worker; i ++) threads[i].join();
    for (int i = 0; i < num_worker; i ++) delete workers[i];

    ofstream result;
    string destination = "Files/" + filename;
    result.open(destination.c_str(), ios::binary);

    for (int i = 1; i <= num_worker; i ++) {
        string file_part_name = "[" + to_string(i) + "]#" + filename;
        ifstream fin(file_part_name.c_str(), ios::binary);
        if (!fin.is_open()) {
            lout << "Cannot open file " << file_part_name << '\n';
            return false;
        }

        result << fin.rdbuf();

        fin.close();
        remove(file_part_name.c_str());
    }

    result.close();

    lout << "File " << filename << " downloaded\n";

    return true;

}