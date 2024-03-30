#pragma once

#include <memory>
#include <optional>
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

using namespace graph;

class transport_router{
public:
    using BusGraph = graph::DirectedWeightedGraph<double>;

    transport_router(const ctlg::TransportCatalogue& cat, RoutingSettings rs): 
        catalogue_(cat), 
        // all_stops_(cat.GetAllStops()),
        rs_(rs),
        meters_per_minute_av{rs_.bus_velocity / to_meters_per_minutes} {
    }

    struct Route{
        double total_time;
        vector<Edge<double>> edges;
    };

    void CreateAllData();
    std::optional<Route> CreateRoute(string_view stop_from, string_view stop_to) const ;

private:
    const ctlg::TransportCatalogue& catalogue_;
    // const std::deque<Stop>& all_stops_;
    std::optional<transport_router::BusGraph> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<const Stop*, size_t> stops_indexes_;
    RoutingSettings rs_;
    const double to_meters_per_minutes = 1000. / 60;
    double meters_per_minute_av;
    size_t GetStopVertexW(const Stop* stop) const;
    size_t GetGraphSize();
    BusGraph BuildGraph() const;
    void CreateStopIndexes();
    void CreateRouter();
    void AddRoute(const Bus* bus, BusGraph& graph, size_t j, size_t k, double dist) const;
    void AddRoundBus(const Bus* bus, BusGraph& graph) const;
    void AddPlainBus(const Bus* bus, BusGraph& graph) const;
};