#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;


const char * CSI = "\033[";

void gotoxy( int x, int y ) {
  std::cout << CSI << (y+1) << ";" << (x+1) << "H";
}

class Window {
    int width, height;
    vector<string> buffer;


public:

    Window(int width, int height) : width(width), height(height) {
        gotoxy(0, 0);
        buffer.resize(height);        
        for (int i = 0; i < height; i++) {
            buffer[i] = "";
            for (int j = 0; j < width; j++) {
                buffer[i] += " ";
            }
        }
    }

    void resize(int width, int height) {
        buffer.clear();
        this->width = width;
        this->height = height;
        buffer.resize(height);        
        for (int i = 0; i < height; i++) {
            buffer[i] = "";
            for (int j = 0; j < width; j++) {
                buffer[i] += " ";
            }
        }
    }
    
    void draw() {
        gotoxy(0, 0);
    //    cout << "Window: " << width << "x" << height << endl;
        cout << "+" << string(width + 5, '-') << "+" << endl;
        for (int i = 0; i < height; i++) {
            cout << "|" << setw(width + 5) <<  buffer[i] <<  "|" << endl;
        }
        cout << "+" << string(width + 5, '-') << "+" << endl;
    }

    void clear() {
        for (int i = 0; i < height; i++) {
            buffer[i] = "";
            for (int j = 0; j < width; j++) {
                buffer[i] += " ";
            }
        }

        draw();
    }

    void draw_line(string& line, int row) {
        gotoxy(0, 0);

        int len = line.size();
        len = min(len, width);
        for (int i = 0; i < len; i++) {
            if (buffer[row][i] != line[i]) {
                buffer[row][i] = line[i];
                gotoxy(i + 2, row);
                cout << line[i];
                buffer[row][i] = line[i];
            }
        }

        gotoxy(width + 5, height + 5);
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
private:
    Window& window;
    vector <pair<string, int>> buffer;
public:

    DownloadPage(Window& w) : window(w) {

        buffer.push_back({
            "            Downloading File           ", 
            1
        });

        buffer.push_back({
            "---------------------------------------",
            2
        });

        buffer.push_back({
            "File: [filename.ext]", 
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
            "", 
            10
        });

        buffer.push_back({
            "Total Progress: [###########         ] 65%", 
            11
        });

        buffer.push_back({
            "", 
            12
        });

        buffer.push_back({
            "[Press 'ctrl' + 'c' to cancel download]", 
            13
        });        


    }
    
    void display() {
        gotoxy(0, 0);

        for (auto& line: buffer) {
            window.draw_line(line.first, line.second);
        }
    }

    void set_file_name(string filename) {
        buffer[2].first = "File: [" + filename + "]";
        while (buffer[2].first.size() < 40) {
            buffer[2].first += " ";
        }
    }

    void set_chunk_progress(int chunk, int progress) {
        chunk --;
        buffer[chunk + 5].first = "Chunk " + to_string(chunk + 1) + ": [";
        for (int i = 0; i < 20; i++) {
            if (i * 100 / 20 < progress) {
                buffer[chunk + 5].first += "#";
            } else {
                buffer[chunk + 5].first += " ";
            }
        }
        buffer[chunk + 5].first += "] " + to_string(progress) + "%";
    }

    void set_total_progress(int progress) {
        buffer[10].first = "Total Progress: [";
        for (int i = 0; i < 20; i++) {
            if (i  * 100 / 20 < progress) {
                buffer[10].first += "#";
            } else {
                buffer[10].first += " ";
            }
        }
        buffer[10].first += "] " + to_string(progress) + "%";
    }
};
