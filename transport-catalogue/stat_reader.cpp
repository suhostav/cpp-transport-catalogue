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

size_t CalcUniqueStops(std::vector<Stop*> stops){
    std::set<std::string_view> uniques;
    for(auto stop : stops){
        uniques.insert(stop->name_);
    }
    return uniques.size();
}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto fields = Split(request, ' ');
    if(fields[0] == "Bus"){
        string_view bus_name = ExtructName(request);
        try{
            auto bus = tansport_catalogue.GetBus(bus_name);
            size_t n_unique_stops = 0;
            size_t n_stops = 0;
            double L = 0.0;
            n_stops = bus->stops_.size();
            n_unique_stops = CalcUniqueStops(bus->stops_);
            for(size_t i = 1; i < bus->stops_.size(); ++i){
                Stop *s1 = bus->stops_[i-1];
                Stop *s2 = bus->stops_[i];
                L += geo::ComputeDistance(s1->coord_, s2->coord_);
            }
            output << fields[0] << " " << bus_name << ": "s << n_stops
                << " stops on route, "s << n_unique_stops << " unique stops, "s << L << " route length\n"s;
        } catch(std::invalid_argument& e){
            output << fields[0] << " "s << bus_name << ": not found\n";
        }
    } else if(fields[0] == "Stop"){
        string_view stop_name = ExtructName(request);
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
}