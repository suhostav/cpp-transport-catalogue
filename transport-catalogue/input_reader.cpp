#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <iterator>

using namespace geo;
/**
 * Удаляет пробелы в начале и конце строки
 */
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

string_view GetWord(string_view line, string_view& word){
    size_t pos = line.find_first_of(' ');
    if(pos == line.npos){
        return line;
    }
    word = line.substr(0, pos);
    return Trim(line.substr(pos, line.size() - pos));
}


size_t GetMeters(string_view data){
    data.remove_suffix(1);
    return std::stoull(string(data));
}

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(string(str.substr(not_space2)));

    return {lat, lng};
}

Coordinates ParseCoordinates(string_view lat_view, string_view lng_view) {
    static const double nan = std::nan("");

    lat_view = Trim(lat_view);
    lng_view = Trim(lng_view);

    if (lat_view.empty() || lng_view.empty()) {
        return {nan, nan};
    }

    double lat = std::stod(lat_view.data());
    double lng = std::stod(lng_view.data());

    return {lat, lng};
}

namespace ctlg{
namespace input{

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
vector<string_view> ParseRoute(string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    vector<string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }
    CommandDescription command {string(line.substr(0, space_pos)),
            string(line.substr(not_space, colon_pos - not_space)),
            {},
            string(line.substr(colon_pos + 1)),
            Split(command.description, ',')};
    if(command.command == "Stop"){
        command.coords = ParseCoordinates(command.fields[0], command.fields[1]);
        command.fields.erase(command.fields.begin(), command.fields.begin() + 2);
    }

    return command;
}

void InputReader::ParseLine(string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

using namespace ctlg;

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for(auto command : commands_){
        if(command.command == "Stop"){
            Stop stop{command.id, command.coords};
            catalogue.AddStop(std::move(stop));
        } 
    }
    for(auto command : commands_){
        if(command.command == "Bus"){
            auto stops = ParseRoute(command.description);
            std::vector<Stop*> route_stops;
            for(auto stop : stops){
                route_stops.push_back(catalogue.GetStop(stop));
            }
            catalogue.AddBus(Bus{std::move(command.id), std::move(route_stops)});
        } 
    }
    //translate distances
    for(auto command : commands_){
        if(command.command == "Stop"){
            SetDistances(command, catalogue);
        } 
    }
}

void InputReader::SetDistances(CommandDescription& command, [[maybe_unused]] TransportCatalogue& catalogue) const{
    for(auto field : command.fields){
        string_view dist_str;
        field = GetWord(field, dist_str);
        size_t dist = GetMeters(dist_str);
        string_view to;
        string_view stop_to = GetWord(field, to);
        catalogue.SetDistance(command.id, stop_to, dist);
    }
}

}   //input
}   //ctlg