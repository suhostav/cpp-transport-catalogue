#pragma once

#include <string>
#include <vector>

#include "geo.h"

using std::string, std::vector, std::string_view;

struct Stop{
    Stop(string name, geo::Coordinates&& coord): 
        name_(std::move(name)),
        coord_(std::move(coord))
    {  }
    string name_;
    geo::Coordinates coord_;
};

struct Bus {
    Bus(string&& name, vector<Stop*>&& stops, Stop* last_stop = nullptr):
        name_(std::move(name)), stops_(std::move(stops)), last_stop_(last_stop)
    {}
    string name_;
    vector<Stop*> stops_;
    Stop* last_stop_;
    bool IsRound();
    Stop* GetLastStop();
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
    size_t lenght;
};

std::string_view Trim(string_view string);
vector<string_view> Split(string_view string, char delim);
size_t GetMeters(string_view data);
string AddEscapes(std::istream& input);