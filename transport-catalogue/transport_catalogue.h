#pragma once
#include <cassert>
#include <deque>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

using std::vector, std::string_view;

namespace ctlg{

struct Stop{

    Stop(std::string name, geo::Coordinates coord): 
        name_(std::move(name)),
        coord_(coord)
    {  }
    std::string name_;
    geo::Coordinates coord_;
};

struct Bus {
    Bus(std::string&& name, vector<const Stop*>&& stops):
        name_(std::move(name)), stops_(std::move(stops))
    {}
    std::string name_;
    std::vector<const Stop*> stops_;
};

class TransportCatalogue {
public:
	void AddStop(const Stop& stop);
	const Stop* GetStop(std::string_view stop_name) const;
	void AddBus(const Bus& bus);
	const Bus* GetBus(std::string_view bus_name) const;
    std::vector<std::string_view> GetStopBuses(std::string_view stop_name) const;
    size_t CalcUniqueStops(const Bus& bus) const;
private:
	std::deque<Stop> all_stops_;
	std::unordered_map<std::string_view, const Stop*>stops_index_;
	std::deque<Bus> all_buses_;
	std::unordered_map<std::string_view, const Bus*> buses_index_;

    bool ContainStop(string_view stop_name, const Bus&  bus) const;
};

} //ctlg