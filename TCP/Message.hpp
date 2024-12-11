#pragma once
#include <vector>
#include <string>
#include <exception>

enum MESSAGE_TYPE {
    RESPOND_LIST = 'L',
    RESPOND_DOWNLOAD = 'D',
    REQUEST_LIST = 'l',
    REQUEST_DOWNLOAD = 'd',
    DEFAULT = '#'
};

const std::string END_MESSAGE = "TEN~GROUP~23CLC03";

class Message {
public:

    std::vector <char> convert(int t) { // convert int to 4 bytes
        std::vector <char> res(4);
        unsigned int tt = t;
        for (int i = 0; i < 4; i ++) {
            res[3 - i] = 255 & (tt >> (8 * i));
        }

        return res;
    }

    int set_byte_int(std::vector<char> vi) {
        int res = 0;
        for (int j = 0; j < 4; j ++)
            res = (res << 8) | (unsigned char)vi[j];
        return res;
    }

    int set_byte_int(const char* data, int l) {
        int res = 0;
        for (int j = 0; j < 4; j ++)
            res = (res << 8) | (unsigned char)data[l + j];
        return res;
    }

    bool check_validity(const char* data, int& len) {
        if (len < END_MESSAGE.size()) return false;
        int n = END_MESSAGE.size();
        for (int i = 0; i < n; i ++) if (data[len - n + i] != END_MESSAGE[i]) 
            return false;

        len -= n;
        return true;
    }

    std::vector <char> m_data;
    virtual MESSAGE_TYPE type() {
        return DEFAULT;
    };

    virtual std::vector<char> get_send_message() {
        std::vector <char> res(5);
        char req = DEFAULT;
        res[0] = req;
        for (auto c: END_MESSAGE) res.push_back(c);
        auto ctLen = convert(res.size());
        for (int i = 0; i < 4; i ++) res[i + 1] = ctLen[i];
        return res;
    };  

    void set_data(std::vector <char> data) {
        m_data = data;
    }

    virtual std::string to_string() {
        std::string ret = "base message\n";
        return ret;
    };

};