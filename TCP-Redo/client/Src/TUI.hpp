#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <io.h>

using namespace std;

// adapted from https://stackoverflow.com/questions/14295570/why-is-console-animation-so-slow-on-windows-and-is-there-a-way-to-improve-spee
#define     ROWS        50
#define     COLS        70

static CHAR_INFO disp[ROWS][COLS];
static HANDLE console;
static COORD window_size = { COLS, ROWS };
static COORD src = { 0, 0};
static SMALL_RECT  dest = { 0, 0, COLS, ROWS };

int civ1[ROWS+2][COLS+2], civ2[ROWS+2][COLS+2];

void ClrScrn(char attrib) {
    COORD pos = { 0, 0};
    DWORD written;
    unsigned size;

    size = ROWS * COLS;

    FillConsoleOutputCharacter(console, ' ', size, pos, &written);
    FillConsoleOutputAttribute(console, attrib, size, pos, &written);
    SetConsoleCursorPosition(console, pos);
}

void fill_edges(int civ1[ROWS+2][COLS+2]) {
    int i, j;

    for (i=1; i<=ROWS; ++i) {
        civ1[i][0] = civ1[i][COLS];
        civ1[i][COLS+1] = civ1[i][1];
    }
    for (j=1; j<=COLS; ++j) {
        civ1[0][j] = civ1[ROWS][j];
        civ1[ROWS+1][j] = civ1[1][j];
    }
    civ1[0][0] = civ1[ROWS][COLS];
    civ1[ROWS+1][COLS+1] = civ1[1][1];
    civ1[0][COLS+1] = civ1[ROWS][1];
    civ1[ROWS+1][0] = civ1[1][COLS];
}


void update_generation(vector <string>& buffer, vector<vector<int>> &bufferAtribute, int civ1[ROWS+2][COLS+2])
{
    int i, j, count;
    int tmp = 0;
    for (int i = 0; i < buffer.size(); i ++) {
        for (int j = 0; j < buffer[i].size(); j ++) {
            disp[i + 1][j + 1].Char.AsciiChar = buffer[i][j];
            if (bufferAtribute[i][j] != 0) {
                disp[i + 1][j + 1].Attributes = bufferAtribute[i][j];
            } else disp[i + 1][j + 1].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        }
    }

    WriteConsoleOutput(console, (CHAR_INFO *)disp, window_size, src, &dest);
    fill_edges(civ1);
}


void initialize(void)
{
    int i, j;

    ClrScrn(40);
    srand(((unsigned int)time(NULL))|1);

    for (i = 1; i <= ROWS; ++i)
    {
        for (j = 1; j <= COLS; ++j)
        {
            civ1[i][j] = (int)(((__int64)rand()*2)/RAND_MAX);
            disp[i-1][j-1].Char.AsciiChar = civ1[i][j] ? ' ' : ' ';
            disp[i-1][j-1].Attributes = 0;
        }
    }
    WriteConsoleOutput(console, (CHAR_INFO *)disp, window_size, src, &dest);
    fill_edges(civ1);
}

class Window {
    int width, height;
    vector<string> buffer;
    vector<vector<int>> bufferAtribute;

public:

    Window& operator=(const Window& other) {
        width = other.width;
        height = other.height;
        buffer = other.buffer;
        return *this;
    }

    int get_height() {
        return height;
    }

    Window(int width, int height) : width(width), height(height) {
        buffer.resize(height);        
        bufferAtribute.resize(height);
        for (int i = 0; i < height; i++) {
            buffer[i] = "";
            bufferAtribute[i].clear();
            for (int j = 0; j < width; j++) {
                buffer[i] += " ";
                bufferAtribute[i].push_back(0);
            }
        }
    }

    void resize(int width, int height) {
        this->width = width;
        this->height = height;
        buffer.resize(height);        
        draw();
    }
    
    void draw() {
    //    cout << "Window: " << width << "x" << height << endl;


        // cout << "+" << string(width + 5, '-') << "+" << endl;
        // for (int i = 0; i < height; i++) {
        //     cout << "|" << setw(width + 5) <<  buffer[i] <<  "|" << endl;
        // }
        // cout << "+" << string(width + 5, '-') << "+" << endl;
        clear();
        buffer[0] = "+" + string(width, '-') + "+";
        buffer[height - 1] = "+" + string(width, '-') + "+";
        for (int i = 1; i < height - 1; i++) {
            buffer[i] = "|" + buffer[i] + "|";
        }
    }

    void clear() {
        for (int i = 0; i < height; i++) {
            buffer[i] = "";
            for (int j = 0; j < width; j++) {
                buffer[i] += " ";
            }
        }
    }

    void draw_line(const string& line, int row) {
        int len = line.size();
        len = min(len, width);
        bool isDif = false;
        for (int i = 0; i < len; i++) {
            if (buffer[row][i + 2] != line[i]) {
                isDif = true;
                buffer[row][i + 2] = line[i];
            }
        }
    }

    void set_buffer_atribute(int row, const vector<int>& atribute) {
        for (int i = 0; i < min(bufferAtribute[row].size(), atribute.size()); i++) {
            bufferAtribute[row][i] = atribute[i];
        }
    }

    void render() {
        update_generation(buffer, bufferAtribute, civ1);
    }

};


/*
+---------------------------------------+
|            Downloading File           |
+---------------------------------------+
| File: [filename.ext]                  |
|                                       |
| Progress:                             |
|  Chunk 1: [#######      ] 50%         |
|  Chunk 2: [#########    ] 75%         |
|  Chunk 3: [######       ] 40%         |
|  Chunk 4: [###########  ] 90%         |
|                                       |
| Total Progress: [##########   ] 65%   |
|                                       |
| [Press 'c' to cancel download]        |
+---------------------------------------+

*/

class DownloadPage {
public:
    int base_row = 0;
    Window& window;
    vector <pair<string, int>> buffer;
    
    int cur_sep_id = 0; 

    void set_window(Window& w) {
        window = w;
    }

    DownloadPage(Window& w) : window(w) {
        
        
        buffer.push_back({
            "                                        ", 
            0
        });

        buffer.push_back({
            "             Download Progress Tracker               ", 
            1
        });
        

        buffer.push_back({
            "-----------------------------------------------------",
            2
        });

        buffer.push_back({
            "Downloading File: [filename.ext]", 
            3
        });

        buffer.push_back({
            "", 
            4
        });

        buffer.push_back({
            "Progress: ", 
            5
        });

        buffer.push_back({
            "Chunk 1: [###########         ] 50%", 
            6
        });

        buffer.push_back({
            "Chunk 2: [###########         ] 75%", 
            7
        });

        buffer.push_back({
            "Chunk 3: [###########         ] 40%", 
            8
        });

        buffer.push_back({
            "Chunk 4: [###########         ] 90%", 
            9
        });

        
        buffer.push_back({
            "                                         ", 
            10
        });

        buffer.push_back({
            "Total Progress:   [###########         ] 65%", 
            11
        });

        buffer.push_back({
            "Combine Progress: [###########         ] 65%", 
            12
        });

        buffer.push_back({
            "", 
            13
        });

        buffer.push_back({
            "[Press 'ctrl' + 'c' to cancel download]", 
            14
        });        
    }

    void set_base_row(int row) {
        base_row = row;
    }
    
    void display() {
        
        vector <int> atribute;
        for (int i = 0; i < 70; i ++) atribute.push_back(134);
        atribute[0] = atribute[1] = 0;

        window.set_buffer_atribute(base_row + 1, atribute);
        for (auto& line: buffer) {
            string r = line.first;
            while (r.size() < 50) {
                r += " ";
            }
            window.draw_line(r, line.second + base_row);
        }
    }

    void set_file_name(string filename) {
        buffer[3].first = "Downloading File: [" + filename + "]";

        vector <int> atribute(70);
        for (int i = 21; i < 21 + filename.size(); i ++) {
            atribute[i] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
        }

        while (buffer[3].first.size() < 40) {
            buffer[3].first += " ";
        }

        window.set_buffer_atribute(3 + base_row, atribute);
    }

    void set_chunk_progress(int chunk, int progress) {
        buffer[chunk + 5].first = "Chunk " + to_string(chunk) + ": [";
        vector <int> atribute(70);
        for (int i = 0; i < 20; i++) {
            if (i * 100 / 20 < progress) {
                buffer[chunk + 5].first += "=";
                atribute[i + 12] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
            } else {
                buffer[chunk + 5].first += " ";
            }
        }
        buffer[chunk + 5].first += "] " + to_string(progress) + "%";
        for (int i = 34; i < 70; i ++) atribute[i] = (progress < 100 ? FOREGROUND_RED : FOREGROUND_GREEN) |  FOREGROUND_INTENSITY;

        window.set_buffer_atribute(chunk + 5 + base_row, atribute);

    }

    void set_total_progress(int progress) {
        buffer[11].first = "Total Progress:   [";
        vector <int> atribute(70);
        for (int i = 0; i < 20; i++) {
            if (i  * 100 / 20 < progress) {
                buffer[11].first += "=";
                //buffer[chunk + 5].first += "=";
                atribute[i + 21] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
            
            } else {
                buffer[11].first += " ";
            }
        }

        buffer[11].first += "] " + to_string(progress) + "%";
        
        for (int i = 42; i < 70; i ++) atribute[i] = (progress < 100 ? FOREGROUND_RED : FOREGROUND_GREEN) |  FOREGROUND_INTENSITY;

        window.set_buffer_atribute(11 + base_row, atribute);
    }

    void set_recv_over_total(unsigned long long recv, unsigned long long total) {
        vector <char> separator = {'|', '/', '-', '\\'}; 
        
        auto convert_unit = [](unsigned long long size) {
            string unit = "B";
            double s = size;

            if (s >= 1024) {
                s /= 1024;
                unit = "KB";
            }

            if (s >= 1024) {
                s /= 1024;
                unit = "MB";
            }

            if (s >= 1024) {
                s /= 1024;
                unit = "GB";
            }

            unsigned long long x = s * 100;

            return to_string(x / 100) + "." + to_string(x % 100) + " " + unit;
        };
        
        int id = cur_sep_id / 5;
        if (id >= 4) id = 0, cur_sep_id = 0;
        
        string pre = buffer[5].first;
        buffer[5].first = "Progress: " + convert_unit(recv) + " " + separator[id] + " " + convert_unit(total);
        cur_sep_id ++;
        
    }

    void set_combine_progress(int progress) {

        vector <int> atribute(70);
        buffer[12].first = "Combine Progress: [";
        for (int i = 0; i < 20; i++) {
            if (i  * 100 / 20 < progress) {
                buffer[12].first += "=";
                atribute[i + 21] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
            } else {
                buffer[12].first += " ";
            }
        }
        buffer[12].first += "] " + to_string(progress) + "%";
        for (int i = 42; i < 70; i ++) atribute[i] = (progress < 100 ? FOREGROUND_RED : FOREGROUND_GREEN) |  FOREGROUND_INTENSITY;
        window.set_buffer_atribute(12 + base_row, atribute);

        for (int i = 2; i < 70; i ++) atribute[i] = FOREGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_BLUE;

        window.set_buffer_atribute(14 + base_row, atribute);

    }

    void set_message(const string& message) {
        buffer[14].first = message;
    }
};

/*
+---------------------------------------+
|         Available Files on Server     |
+---------------------------------------+
| Page [1/3]                            |
| File Name               | Size (KB)   |
| ------------------------------------- |
| file1.txt               | 1024        |
| file2.jpg               | 2048        |
| file3.pdf               | 512         |
| file4.docx              | 256         |
| file5.zip               | 4096        |
| file6.png               | 8192        |
| file7.mp4               | 10240       |
| file8.mov               | 12288       |
| file9.exe               | 15370       |
| file10.iso              | 20480       |
|                                       |
| [n] Next Page     [b] Previous Page   |
| [Press 'm' to return to main menu]    |
+---------------------------------------+

*/
class AvailableFilesPage {
public:
    Window& window;
    vector <pair<string, unsigned long long>> files;
    string IP;
    int port;
    int next_update = 0;

    void set_window(Window& w) {
        window = w;
    }

    void set_next_update(float time) {
        next_update = time;
    }
    
    int get_height() {
        return files.size();
    }

    AvailableFilesPage(Window& w) : window(w) {
    }

    void add_file(string filename, int size) {
        for (auto file: files) {
            if (file.first == filename) {
                return;
            }
        }

        files.push_back({filename, size});
    
        sort(files.begin(), files.end());

    }

    void set_file_list(const vector<pair<string, unsigned long long>>& files) {
        this->files = files;
    }

    void set_server_info(string ip, int port) {
        IP = ip;
        this->port = port;
    }

    void display() {
        int base = 6;

        window.draw_line("              Available Files on Server              ", 1);

        vector <int> atribute;
        for (int i = 0; i < 70; i ++) atribute.push_back(129);
        atribute[0] = atribute[1] = 0;
        window.set_buffer_atribute(1, atribute);

        window.draw_line("-----------------------------------------------------", 2);
        string seconds = to_string(next_update / 1000) + ".";
        string miliseconds = to_string(next_update % 1000);
        while (miliseconds.size() < 3) {
            miliseconds = "0" + miliseconds;
        }

        string serverInfo = "Server: " + IP + ":" + to_string(port) + " | Next Scan: " + seconds + miliseconds + "s";

        window.draw_line(serverInfo, 3);
        
        for (int i = 1; i < 30; i ++) {
            if (i > 8) atribute[i] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
            else atribute[i] = 0;
        }

        for (int i = 30; i < 70; i ++) atribute[i] = 0;
        window.set_buffer_atribute(3, atribute);

        window.draw_line("File Name                               | Size (byte)", 4);
        for (auto &i: atribute) i = 0;
        for (int i = 1; i < 30; i ++) atribute[i] = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
        for (int i = 59; i >= 44; i --) atribute[i] = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        window.set_buffer_atribute(4, atribute);
        
        window.draw_line("-----------------------------------------------------", 5);
        
        int i = 0;
        for (auto& file: files) {
            string line = file.first;
            while (line.size() < 40) {
                line += " ";
            }

            line += "| " + to_string(file.second);
            window.draw_line(line, base + i);
            window.set_buffer_atribute(base + i, atribute);
            i ++;
        }
    }
};

class clientUI {
private:
    Window w;
    DownloadPage d;
    AvailableFilesPage a;
    string IP = "0.0.0.0";
    int port = 8888;

public:

    bool in_progress = false;
    clientUI& operator=(const clientUI& other) {
        w = other.w;
        d.set_window(w);
        a.set_window(w);

        a.files = other.a.files;
        a.IP = other.a.IP;
        a.port = other.a.port;

        d.base_row = other.d.base_row;
        d.buffer = other.d.buffer;

        IP = other.IP;
        port = other.port;
        //w.draw();



        return *this;
    }

    clientUI() : w(55, 30), d(w), a(w) {
        //w.draw();
        console = GetStdHandle(STD_OUTPUT_HANDLE);
        initialize();
    }

    void set_server_info(string ip, int port) {
        IP = ip;
        this->port = port;

        a.set_server_info(ip, port);
    }

    void display_available_files() {
        a.display();
    }

    void add_file(string filename, int size) {
        a.add_file(filename, size);
    }

    void display_download_page() {
        d.display();
    }

    void set_file_name(string filename) {
        d.set_file_name(filename);
    }

    void set_chunk_progress(int chunk, int progress) {
        d.set_chunk_progress(chunk, progress);
    }

    void set_total_progress(int progress) {
        d.set_total_progress(progress);
    }

    void set_file_list(const vector<pair<string, unsigned long long>>& files) {
        a.set_file_list(files);
    }

    void set_base_row(int row) {
        d.set_base_row(row);
    }

    void set_message(const string& message) {
        d.set_message(message);
    }

    void set_combine_progress(int progress) {
        d.set_combine_progress(progress);
    }

    void set_next_update(int time) {
        a.set_next_update(time);
    }

    void set_recv_over_total(unsigned long long recv, unsigned long long total) {
        d.set_recv_over_total(recv, total);
    }

    void display() {
        int file_count = a.get_height();
        d.set_base_row(file_count + 6);
        w.resize(55, 22 + file_count);

        a.display();
        d.display();
        if (!in_progress) {
           w.render();
        }
    }
};