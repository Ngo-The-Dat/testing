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

#include "../message.hpp"

void get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size);
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


void handle_download(SOCKET server, string serverIp, unsigned short serverPort) {
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

    // void get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size)

    //get_any_file(server, name, filelist[name]);
    get_download(name, serverIp, serverPort, 0, filelist[name]);
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
        
        std::cout << "not ok\n";
        throw std::runtime_error("not ok get server wellcome");
    }

    std::cout << "\n\n\t\t\t\t---   WORKER   ---\n";
    std::cout << "\n[Server]: \t" << get_content_short(wellcome) << '\n';
    std::cout << "\t\t\t\t-------------------\n\n";
}

void Worker::get_file(string filename, unsigned long long offset, unsigned long long len, unsigned long long filesize, int part, unsigned long long& progress) {

    short_message worker_hello = make_short_message("WORKER_GET_CHUNK");
    send(worker_hello, socketHandle, "worker_send");

    int rbyt = recv(worker_hello, socketHandle, "worker recv from server");

    if (get_content(worker_hello.content, worker_hello.len) != "OK") {
        std::cout << "Content: " << get_content(worker_hello.content, worker_hello.len) << '\n';
        std::cout << "Server rejected at hello\n";
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
        std::cout << "Server rejected at get chunk\n";
        return;
    }

    ack = make_short_message("OK");
    
    string file_part_name = "[" + to_string(part) + "]#" + filename;
    ofstream fout;
    fout.open(file_part_name.c_str(), ios::binary);

    if (!fout.is_open()) {
        ack = make_short_message("NO");
        std::cout << "error open file " << file_part_name << '\n';
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
            std::cout << "error recv on" + file_part_name << '\n';
            return;
        }
        fout.write(rec.content, rec.len);
        total_size += rec.len;
        progress += rec.len;
    }
    
    fout.close();
    std::cout << file_part_name << " Successully recieved\n";

}


void get_download(string filename, string ipAddress, unsigned short port, unsigned long long offset, unsigned long long size) {
    // Worker worker1(ipAddress, port);
    // worker1.initialize();
    // worker1.connectToServer();
    // worker1.run();

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

        threads.push_back(thread(&Worker::get_file, workers[i], filename, offset_assigned, chunk, size, i + 1, ref(progress[i])));
 
        offset_assigned += chunk;

    }

    while (true) {
        bool done = true;
        for (int i = 0; i < num_worker; i ++) {
            if (progress[i] < target_size[i]) {
                done = false;
            }

            int percent = ((double)progress[i] / (double)target_size[i]) * 100;
            std::cout << "Progress " << i + 1 << ": " << percent << 
            "%\t\t size: " << progress[i] << '/' << target_size[i] << '\n';

        }
        if (done) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    for (int i = 0; i < num_worker; i ++) threads[i].join();
    for (int i = 0; i < num_worker; i ++) delete workers[i];

    ofstream result;
    result.open(filename.c_str(), ios::binary);

    for (int i = 1; i <= num_worker; i ++) {
        string file_part_name = "[" + to_string(i) + "]#" + filename;
        ifstream fin(file_part_name.c_str(), ios::binary);
        if (!fin.is_open()) {
            std::cout << "Cannot open file " << file_part_name << '\n';
            return;
        }


        char buffer[RECIEVE_BUFFER_SIZE];
        unsigned long long target = target_size[i - 1];
        unsigned long long cur = 0;
        while (cur < target) {
            int next = RECIEVE_BUFFER_SIZE;
            if (cur + next > target) next = target - cur;
            fin.read(buffer, next);
            result.write(buffer, next);
            cur += next;
        }

        fin.close();
 
        remove(file_part_name.c_str());

    }

    result.close();

}