#include <iostream>
#include <sstream>
#include "domain.h"

using  std::string_view, std::vector;

string_view Trim(string_view str) {
    const auto start = str.find_first_not_of(' ');
    if (start == str.npos) {
        return {};
    }
    return str.substr(start, str.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view str, char delim) {
    vector<string_view> result;

    size_t pos = 0;
    while ((pos = str.find_first_not_of(' ', pos)) < str.length()) {
        auto delim_pos = str.find(delim, pos);
        if (delim_pos == str.npos) {
            delim_pos = str.size();
        }
        if (auto substr = Trim(str.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}


size_t GetMeters(string_view data){
    data.remove_suffix(1);
    return std::stoull(std::string(data));
}

string AddEscapes(std::istream& input){
    std::stringstream out;
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    while(it != end){
        const char ch = *it;
        if(ch == '\\'){
            out << '\\' << '\\';
        } else if(ch == '\"'){
            out << '\\' << '\"';
        } else if(ch == '\n'){
            out << '\\' << 'n';
        } else if(ch == '\r'){
            out << '\\' << 'r';
        } else {
            out << ch;
        }
        ++it;
    }
    return out.str();
}

bool Bus::IsRound(){
    return last_stop_ == nullptr;
}

Stop* Bus::GetLastStop() {
    return last_stop_;
}