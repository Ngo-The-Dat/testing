#pragma once
#include <set>
#include <fstream>
#include <string>

using namespace std;

bool compare_file_set(const set <string>& list, const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        return false;
    }

    string name;

    bool result = true;

    while (fin >> name) {
        if (!list.count(name)) {
            result = false;
            break;
        }
    }

    fin.close();

    return result;
}