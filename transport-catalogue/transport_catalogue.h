#pragma once
#include <cassert>
#include <deque>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"
// #include "geo.h"


namespace ctlg{

using std::string, std::string_view, std::vector;

class TransportCatalogue {
public:
    using StopPair = std::pair<Stop*, Stop*>;
    
    void AddStop(const Stop& stop);
    Stop* GetStop(string_view stop_name) const;
    void AddBus(const Bus& bus);
    Bus* GetBus(string_view bus_name) const;
    vector<string_view> GetStopBuses(string_view stop_name) const;
    bool ContainStop(string_view stop_name, const Bus&  bus) const;
    void SetDistance(string_view stop_from_name, string_view stop_to_name, size_t dist);
    // RouteInfo GetRouteInfo(const Bus& bus) const;
    BusStat GetBusStat(const Bus& bus) const;
    StopStat GetStopStat(const Stop& stop) const;
    vector<string_view> GetBuses() const;
private:

    struct PairPointerHasher{
        size_t operator()(const StopPair& elem) const {
            return (size_t)(elem.first) + 37 * (size_t)(elem.second);
        }
    };

    std::deque<Stop> all_stops_;
    std::unordered_map<std::string_view, Stop*>stops_index_;
    std::deque<Bus> all_buses_;
    std::unordered_map<std::string_view, Bus*> buses_index_;
    std::unordered_map<StopPair,size_t,PairPointerHasher> distances_;

    std::pair<size_t,double> CalcUniqueStopsAndRouteLenght(const Bus& bus) const;
    size_t GetRouteLenght(const Bus& bus) const;
};

} //ctlg
