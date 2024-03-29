#include "transport_router.h"

size_t transport_router::GetStopVertexW(const Stop* stop) const {
    return stops_indexes_.at(stop) * 2;
}

size_t transport_router::GetGraphSize(){
    return all_stops_.size() * 2;
}

void transport_router::CreateStopIndexes(){
    stops_indexes_.clear();
    for(size_t i = 0; i < all_stops_.size(); ++i){
        stops_indexes_[&all_stops_[i]] = i;
    }
}

transport_router::BusGraph transport_router::BuildGraph() const{
    const std::deque<Bus>& buses = cat_.GetAllBuses();
    //i * 2 - вершина начала ожидания wait для остановки i. i * 2 + 1 - вершина остановки после ожидания, и т.д.
    //где i - индекс в all_stops_
    size_t N = all_stops_.size() * 2;
    BusGraph graph(N);
    //stopW -> stop
    for(const Stop& stop : all_stops_){
        graph::VertexId vertexW = GetStopVertexW(&stop);
        graph::VertexId vertex = vertexW + 1;
        graph.AddEdge({vertexW, vertex, stop.name_, 0, (double)rs_.bus_wait_time});
    }
    for(size_t i = 0; i < buses.size(); ++i){
        if(buses[i].IsRound()){
            AddRoundBus(&buses[i], graph);
        } else {
            AddPlainBus(&buses[i], graph);
        }
    }
    return graph;
}

void transport_router::CreateRouter(){
    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

void transport_router::CreateAllData(){
    CreateStopIndexes();
    graph_ = BuildGraph();
    CreateRouter();
}


inline void transport_router::AddRoute(const Bus* bus, BusGraph& graph, size_t j, size_t k, double dist) const{
    size_t vertex_fromW = GetStopVertexW(bus->stops_[j]);
    size_t vertex_from = vertex_fromW + 1;
    size_t vertex_toW = GetStopVertexW(bus->stops_[k]);
    graph.AddEdge({vertex_from, vertex_toW, bus->name_, (k>j)? k-j : j-k, dist / meters_per_minute_av});
}

void transport_router::AddRoundBus(const Bus* bus, BusGraph& graph) const{
    for(size_t j = 0; j < bus->stops_.size(); ++j){
        double dist = 0.0;
        for(size_t k = j + 1; k < bus->stops_.size(); ++k){
            dist += cat_.GetDistance(bus->stops_[k-1], bus->stops_[k]);
            AddRoute(bus, graph, j, k, dist);
        }
    }
}

void transport_router::AddPlainBus(const Bus* bus, BusGraph& graph) const{
    size_t last_stop_ind = bus->GetLastStopIndex();
    for(size_t j = 0; j < last_stop_ind; ++j){
        double dist = 0.0;
        for(size_t k = j + 1; k <= last_stop_ind; ++k){
            dist += cat_.GetDistance(bus->stops_[k-1], bus->stops_[k]);
            AddRoute(bus, graph, j, k, dist);
        }
    }
    for(size_t j = last_stop_ind; j < bus->stops_.size(); ++j){
        double dist = 0.0;
        for(size_t k = j + 1; k < bus->stops_.size(); ++k){
            dist += cat_.GetDistance(bus->stops_[k-1], bus->stops_[k]);
            AddRoute(bus, graph, j, k, dist);
        }
    }
}


std::optional<transport_router::Route> transport_router::CreateRoute(string_view stop_from, string_view stop_to) const{
    Stop* from = cat_.GetStop(stop_from);
    Stop* to = cat_.GetStop(stop_to);
    auto route_info = router_->BuildRoute(GetStopVertexW(from),GetStopVertexW(to));
    if(!route_info){
        return {};
    }
    Route route;
    route.total_time = (*route_info).weight;
    for(graph::EdgeId edge_id : (*route_info).edges){
        route.edges.push_back(graph_->GetEdge(edge_id));
    }
    return {route};
}