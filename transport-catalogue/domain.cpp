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


bool Bus::IsRound() const{
    return is_round_;
}

size_t Bus::GetLastStopIndex() const {
    return stops_.size() / 2;
}

Stop* Bus::GetLastStop() const {
    return stops_[GetLastStopIndex()];
}

bool Bus::IsEndStop(Stop* stop) const{
    if(IsRound()){
        return stop == stops_[0];
    } else {
        Stop* last_stop = GetLastStop();
        return stop == stops_[0] || stop == last_stop;
    }
}

size_t Bus::GetStopIndex(Stop* stop) const{
    for(size_t i = 0; i < stops_.size(); ++i){
        if( stop == stops_[i]){
            return i;
        }
    }
    throw std::runtime_error("Bus::GetStopIndex: no stop found."s);
}