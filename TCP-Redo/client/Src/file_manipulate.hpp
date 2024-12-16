#pragma once
#include <set>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

using namespace std;

bool compare_file_set(const vector <string>& list, const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        return false;
    }

    string name;
    bool result = true;
    int i = 0;
    while (fin >> name) {
        if (i >= list.size()) {
            result = false;
            break;
        }

        i++;
    }

    fin.close();

    return result;
}

bool check_download_file(const string& filename) {
    std::filesystem::path p("Files");
    for (const auto & entry : std::filesystem::directory_iterator(p)) {
        string file = entry.path().filename().string();
        if (file == filename) {
            return true;
        }
    }
    return false;
}

bool get_filename_size(string& name, unsigned long long& size, ifstream& fin) {
    if (!fin.is_open()) {
        return false;
    }

    string line;
    getline(fin, line);

    if (line.empty()) {
        return false;
    }

    size_t pos = line.find_last_of(' ');
    name = line.substr(0, pos);
    size = stoull(line.substr(pos + 1));

    return true;

}