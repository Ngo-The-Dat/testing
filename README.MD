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

## Demo

After run the client, Ex: ./client.exe 192.168.20.103 8888, and assumes that the server is listening on port 8888, you will see the client interface

```powershell

[Connected to 192.168.20.103:8888]


                                ---   WELCOME   ---

[Server]:       Welcome to the server! you are client 0

                                -------------------

Command: [list] or [download] (ex: list)
[Choice]:

```

**Client commands:**

- `list`: Get the list of files that server can serve;

- `download`: Request to download a file in the list by file name, to see the list of download-able files, you need to request the list first by run `list`. After that you can choose the file to download by specify the name of the file.

**Download option:**

After you chose download, some file will appear along with there size in byte. And you choose which file to download at this step.

```powershell
Command: [list] or [download] (ex: list)
[Choice]: download
Filename & size: cat.jpg                77456
Filename & size: file1.txt              9
Filename & size: image.png              51192
Filename & size: image2.png             64270
Filename & size: Matcha.pdf             47314780
[Choice]:

```

Suppose you downloaded `cat.jpq`, here is the output

```powershell
[Choice]: cat.jpg
recieving...
entered
216 216
getting the file: cat.jpg
send get list
i got the file
```

## Worker

Create 4 workers at client site

**Protocols:**

After create 4 workers -> let master sleep for some time -> after that check the progress -> throught file?    



---

## To do

- [x] complete 4 worker socket
- [x] find a way for worker to update progress to master
- [x] complete UI
- [ ] report the protocols and idea
- [ ] clean the server log