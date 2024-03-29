#pragma once

#include <string>
#include <vector>

#include "geo.h"
#include "graph.h"

using std::string, std::vector, std::string_view;
using namespace std::literals;

struct Stop{
    Stop(string name, geo::Coordinates&& coord): 
        name_(std::move(name)),
        coord_(std::move(coord))
    {  }
    string name_;
    geo::Coordinates coord_;
};

struct Bus {
    Bus(string&& name, vector<Stop*>&& stops, bool is_round):
        name_(std::move(name)), stops_(std::move(stops)), is_round_(is_round)
    {}
    string name_;
    vector<Stop*> stops_;
    bool is_round_;
    bool IsRound() const;
    size_t GetLastStopIndex() const;
    Stop* GetLastStop() const;
    bool IsEndStop(Stop* stop) const;
    size_t GetStopIndex(Stop* stop) const;
};

using StopPtr = Stop*;
using BusPtr = Bus*;

struct StopStat {
    string name;
    vector<string_view> buses;
};

struct BusStat {
    string name;
    size_t stops;
    size_t unique_stops;
    double lenght_geo;
    int lenght;
};

struct RoutingSettings{
    unsigned int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct VertexData{
    size_t index;
    const Stop* stop = nullptr;
    const Bus* bus = nullptr;
};

std::string_view Trim(string_view string);
vector<string_view> Split(string_view string, char delim);
size_t GetMeters(string_view data);