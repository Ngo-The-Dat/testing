#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <exception>

enum MESSAGE_TYPE {
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

class RequestList : public Message {
public:

    int m_len;
    std::string m_filename;

    RequestList() {}
    RequestList(std::string filename) : m_len(filename.size()), m_filename(filename) {};
    
    RequestList(const char* data) {
        int i = 1;
        int len = 0; // data len
        len = set_byte_int(data, i); i += 4;
        if (!check_validity(data, len)) {
            std::cout << "Loss of data\n";
        }

        m_len = set_byte_int(data, i); i += 4;
        m_filename = "";
        for (; i < len; i ++) m_filename += data[i];
    }

    RequestList(const char* data, int len) {
        int i = 0; // skip the fisrt byte
        m_filename = "";
        for (; i < len; i ++) m_filename += data[i];
        m_len = len;
    }

    MESSAGE_TYPE type() override {
        return REQUEST_LIST;
    }

    std::vector<char> get_send_message() override {
        std::vector <char> res(5);
        char req = REQUEST_LIST;
        res[0] = req;

        auto tmp = convert(m_len);
        for (auto ch:tmp) res.push_back(ch);
        for (auto ch: m_filename) res.push_back(ch);
        for (auto c: END_MESSAGE) res.push_back(c);

        auto ctLen = convert(res.size());
        for (int i = 0; i < 4; i ++) res[i + 1] = ctLen[i];
        return res;
    }

    std::string to_string() override {
        std::string res = "Listname: ";
        res += m_filename + "; list len: ";
        res += std::to_string(m_len);
        res += '\n';
        return res;
    }
};

class RequestChunk : public Message {
private:
public:

    std::string m_filename;
    int m_len;
    int m_offset;
    
    RequestChunk(std::string name, int len, int offset) : m_filename(name), m_len(len), m_offset(offset) {}
    RequestChunk(const char* data) {
        int i = 1; // skip the fisrt byte

        int len = set_byte_int(data, i); i += 4;
        if (!check_validity(data, len)) {
            std::cout << "Loss of data\n";
        }

        m_len = set_byte_int(data, i); i += 4;
        m_offset = set_byte_int(data, i); i += 4;

        m_filename = "";
        for (; i < len; i ++) m_filename += data[i];
    }
 
    MESSAGE_TYPE type() override  {
        return REQUEST_DOWNLOAD;
    };
    
    std::vector<char> get_send_message() override {
        std::vector <char> res(5);
        char req = REQUEST_DOWNLOAD;
        res[0] = req;

        std::vector <char> tmp;
        // get len
        tmp = convert(m_len);
        for (auto c: tmp) res.push_back(c);
        // get offset
        tmp = convert(m_offset);
        for (auto c: tmp) res.push_back(c);
        for (auto c: m_filename) res.push_back(c);

        for (auto c: END_MESSAGE) res.push_back(c);

        auto ctLen = convert(res.size());
        for (int i = 0; i < 4; i ++) res[i + 1] = ctLen[i];
        return res;
    }  

    
    std::string to_string() override {
        std::string res = "name: ";
        res += m_filename + "; len: ";
        res += std::to_string(m_len) + "; offset: ";
        res += std::to_string(m_offset);
        res += '\n';
        return res;
    }

};

Message* getMessage(const char* data) {
    try {
        if (REQUEST_LIST == data[0]) 
            return new RequestList(data);
        if (REQUEST_DOWNLOAD == data[0])
            return new RequestChunk(data);
        if (DEFAULT == data[0]) 
            return new Message();
    } catch (std::exception &error) {
        std::cerr << error.what() << '\n'; 
    }
    return NULL;
}


class Respone : public Message {

};