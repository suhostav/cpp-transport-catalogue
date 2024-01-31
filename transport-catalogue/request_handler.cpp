#include <algorithm>
# include "request_handler.h"

RequestHandler::RequestHandler(TransportCatalogue& db):
    db_(db) {}

void RequestHandler::AddStop(const Stop& stop) {
    db_.AddStop(std::move(stop));
}

Stop* RequestHandler::GetStop(string_view stop_name) const {
    return db_.GetStop(stop_name);
}

void RequestHandler::AddBus(const Bus& bus){
    db_.AddBus(std::move(bus));
}

Bus* RequestHandler::GetBus(string_view bus_name) const{
    return db_.GetBus(bus_name);
}

void RequestHandler::SetDistance(string_view stop_from_name, string_view stop_to_name, size_t dist) {
    db_.SetDistance(stop_from_name, stop_to_name, dist);
}

BusStat RequestHandler::GetBusStat(const Bus& bus) const {
    return db_.GetBusStat(bus);
}

StopStat RequestHandler::GetStopStat(const Stop& stop) const {
    return db_.GetStopStat(stop);
}

vector<std::pair<string_view, Bus *>> RequestHandler::GetRoutes() {
    svg::Document svg_doc;
    auto bus_names = db_.GetBuses();
    vector<std::pair<string_view, Bus *>> routes;
    for( auto bus_name: bus_names){
        routes.push_back({bus_name, db_.GetBus(bus_name)});
    }
    return routes;
}

