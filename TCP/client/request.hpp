#pragma once
#include "../protocols.hpp"

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