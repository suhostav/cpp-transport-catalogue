#include <algorithm>
#include <set>
#include <stdexcept>
#include "transport_catalogue.h"
#include "input_reader.h"

namespace ctlg{

void TransportCatalogue::AddStop(Stop&& stop){
    all_stops_.push_back(std::move(stop));
    stops_index_.insert({all_stops_.back().name_, &all_stops_.back()});
}

Stop* TransportCatalogue::GetStop(std::string_view stop_name) const{
    if(stops_index_.count(stop_name) == 0){
        std::string msg{"invalid bus stop name: "};
        msg += stop_name;
        throw std::invalid_argument(msg);
    }
    return stops_index_.at(stop_name);
}

void TransportCatalogue::AddBus(Bus&& bus){
    all_buses_.push_back(std::move(bus));
    auto& bus_name = all_buses_.back().name_;
    buses_index_.insert({bus_name, &all_buses_.back()});
}

Bus* TransportCatalogue::GetBus(std::string_view bus_name) const{
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

std::pair<size_t,double> TransportCatalogue::CalcUniqueStopsAndRouteLenght(const Bus& bus) const{
    std::set<std::string_view> uniques;
    double L = 0.0;
    for(size_t i = 0; i < bus.stops_.size(); ++i){
        uniques.insert(bus.stops_[i]->name_);
        if(i > 0){
            const Stop *s1 = bus.stops_[i - 1];
            const Stop *s2 = bus.stops_[i];
            L += geo::ComputeDistance(s1->coord_, s2->coord_);
        }
    }
    return {uniques.size(), L};
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

void TransportCatalogue::SetDistance(string_view stop_from_name, string_view stop_to_name, size_t dist){
    Stop* stop_from{GetStop(stop_from_name)};
    Stop* stop_to{GetStop(stop_to_name)};
    if(!stop_from || !stop_to){
        throw std::runtime_error("TransportCatalogue::SetDistance: Unknown stop name.");
    }
    StopPair key{stop_from, stop_to};
    distances_[key] = dist;
}

size_t TransportCatalogue::GetRouteLenght(const Bus* bus) const{
    size_t L = 0;
    for(size_t i = 0; i < bus->stops_.size() - 1; ++i){
        StopPair key{bus->stops_[i], bus->stops_[i+1]};
        if(distances_.count(key)){
            L += distances_.at(key);
        } else {
            StopPair back{bus->stops_[i+1], bus->stops_[i]};
                    if(distances_.count(back)){
            L += distances_.at(back);
        }
        }
    }
    return L;
}

} //ctlg