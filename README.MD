# NETWORKING WITH WINSOCK

---

## Build and Run

**Build:**

```bat

cd ./TCP-Redo
./build all

// or ./build client
// or ./build server
```

**Run server:**

```bat

// from TCP
cd ./server
./server.exe <ip> <port>

```

**Run client:**

```bat

// from TCP
cd ./client
./client.exe <ip> <port>
```

## Client UI

![alt text](Picture/UI.png)

The UI is a simple Terminal UI, it will show the progress of the download and the download speed. Also the time remaining for next time it scans the `input.txt`.

When the client start the program, set says using the command `./client 192.168.20.103 8888`, it first fetch the available files from the server, shows to the user. then it will start downloading the files. The files will be saved in the `./client/Files` folder.

![alt text](Picture/noti.png)

When the file is dowloaded, notify will tell the user can press ctrl + c to exit the program. While it download, it will tell the user wait for the program to download.

## Protocols

---

## To do

- [x] complete 4 worker socket
- [x] find a way for worker to update progress to master
- [x] complete UI
- [ ] report the protocols and idea
- [ ] clean the server log
- [x] progress bar for concating files
- [x] read small size chunk -> utilize the most of cache -> currently use << rbuff