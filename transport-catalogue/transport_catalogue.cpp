#include <algorithm>
#include <set>
#include <stdexcept>
#include "transport_catalogue.h"
#include "input_reader.h"

namespace ctlg{

void TransportCatalogue::AddStop(const Stop& stop){
    all_stops_.push_back(stop);
    stops_index_.insert({all_stops_.back().name_, &all_stops_.back()});
}

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const{
    if(stops_index_.count(stop_name) == 0){
        std::string msg{"invalid bus stop name: "};
        msg += stop_name;
        throw std::invalid_argument(msg);
    }
    return stops_index_.at(stop_name);
}

void TransportCatalogue::AddBus(const Bus& bus){
    all_buses_.push_back(bus);
    auto& bus_name = all_buses_.back().name_;
    buses_index_.insert({bus_name, &all_buses_.back()});
}

const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const{
    if(buses_index_.count(bus_name) == 0){
        std::string msg{"invalid bus name: "};
        msg += bus_name;
        throw std::invalid_argument(msg);
    }
    return buses_index_.at(bus_name);
}

bool TransportCatalogue::ContainStop(string_view stop_name, const Bus&  bus) const{
    for(auto stop : bus.stops_){
        if(stop->name_ == stop_name){
            return true;
        }
    }
    return false;
}

size_t TransportCatalogue::CalcUniqueStops(const Bus& bus) const{
    std::set<std::string_view> uniques;
    for(auto stop : bus.stops_){
        uniques.insert(stop->name_);
    }
    return uniques.size();

}

vector<string_view> TransportCatalogue::GetStopBuses(string_view stop_name) const{
    vector<string_view> buses;
    if(stops_index_.count(stop_name) == 0){
        std::string msg{"invalid bus stop name: "};
        msg += stop_name;
        throw std::invalid_argument(msg);
    }
    for(const auto& bus : all_buses_){
        if(ContainStop(stop_name, bus)){
            buses.push_back(bus.name_);
        }
    }
    std::sort(buses.begin(), buses.end());
    return buses;
}

} //ctlg