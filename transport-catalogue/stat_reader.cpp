#include <iomanip>
#include <set>
#include <string>
#include "stat_reader.h"
#include "geo.h"

std::string_view Trim(std::string_view string);
std::vector<std::string_view> Split(std::string_view string, char delim);

using std::string, std::string_view;
using namespace std::literals;
using namespace ctlg;

//first word of request is command, others are name
string_view ExtructName(string_view request){
    request = Trim(request);
    size_t pos = 0;
    if ((pos = request.find_first_of(' ', pos)) < request.length()){
        if( (pos = request.find_first_not_of(' ', pos)) < request.length() ){
            return request.substr(pos, request.length() - pos);
        }
    }
    return {};
}

void PrintBusStat(const TransportCatalogue& tansport_catalogue, string_view bus_name,
                       std::ostream& output){
    try{
        auto bus = tansport_catalogue.GetBus(bus_name);
        size_t n_stops = bus->stops_.size();
        auto [n_unique_stops, L] = tansport_catalogue.CalcUniqueStopsAndRouteLenght(*bus);
        size_t dist = tansport_catalogue.GetRouteLenght(bus);
        double curvature = static_cast<double>(dist) / L;
        output << "Bus " << bus_name << ": "s << n_stops
            << " stops on route, "s << n_unique_stops 
            << " unique stops, "s 
            << dist << " route length, "s
            << curvature << " curvature\n"s;
    } catch(std::invalid_argument& e){
        output << "Bus " << bus_name << ": not found\n";
    }
}

void PrintStopStat(const TransportCatalogue& tansport_catalogue, string_view stop_name,
                       std::ostream& output){
    output << "Stop " << stop_name << ":";
    try{
        auto buses = tansport_catalogue.GetStopBuses(stop_name);
        if(buses.size() == 0){
            output << " no buses\n";
        }else {
            output << " buses";
            for(auto bus : buses){
                output << " " << bus;
            }
            output << '\n';
        }
    }catch(std::invalid_argument& e){
        output << " not found\n";
    }

}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, string_view request,
                       std::ostream& output) {
    auto fields = Split(request, ' ');
    if(fields[0] == "Bus"){
        string_view bus_name = ExtructName(request);
        PrintBusStat(tansport_catalogue, bus_name, output);
    } else if(fields[0] == "Stop"){
        string_view stop_name = ExtructName(request);
        PrintStopStat(tansport_catalogue, stop_name, output);
    }
}