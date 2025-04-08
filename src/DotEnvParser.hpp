#include <iostream>
#include <fstream>
#include <unordered_map>
#include <exception>
#include <optional>


class exception_with_message : public std::exception {
public:
    const std::string message;
    exception_with_message(const std::string message)
    : message(message) { } 
};
class io_fail : public std::exception { };
class no_value : public exception_with_message {
public:
    no_value(std::string message): exception_with_message(message) {}
    const char* what() const noexcept override { return message.c_str(); }
};


int min(int a, int b) { return a < b ? a : b; }


class DotEnvParser {

    std::unordered_map<std::string, std::string> records;

    void parse_line(const std::string& line) {
        std::string key, value;
        int eq_index = -1;
        int ht_index = 99999;
        
        for (int i = 0; i < line.size(); i++) {
            if (line[i] == '#') { ht_index = i; break; }
            if (line[i] == '=') { eq_index = i; }
        }

        if (ht_index < eq_index || eq_index == -1) {
            return; // equality commented or not presented
        }

        key = line.substr(0, eq_index);
        value = line.substr(eq_index+1, min(line.size(), ht_index));

        if (key.size() > 0 && value.size() > 0) {
            records[key] = value;
        }
    }

    std::optional<std::string> get(const std::string& key) {
        auto record = records.find(key);
        if (record != records.end()) {
            return record->second;
        }
        return std::nullopt;
    }

public:
    const std::string file_path;

    std::string get_str(const std::string& key) {
        auto record = get(key);
        if (record.has_value())
            return record.value();
        throw no_value(key);
    }

    std::string get_str(const std::string& key, const std::string default_) {
        auto record = get(key);
        if (record.has_value())
            return record.value();
        return default_;
    }

    unsigned long get_ulong(const std::string& key) {
        auto record = get(key);
        if (record.has_value())
            return std::stoul(record.value());
        throw no_value(key);
    }

    unsigned long get_ulong(const std::string& key, const unsigned long default_) {
        auto record = get(key);
        if (record.has_value())
            return std::stoul(record.value());
        return default_;
    }

    DotEnvParser(const std::string& file_path)
    : file_path(file_path) 
    {
        std::ifstream in;
        in.open(file_path);

        if (in.fail()) { throw io_fail(); }

        std::string line;

        while (!in.eof()) {
            std::getline(in, line);
            parse_line(line);
        }

        in.close();
    }
};
