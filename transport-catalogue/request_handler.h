#pragma once

#include <optional>
#include <unordered_set>
#include "svg.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

// using namespace ctlg::jreader;
using namespace ctlg;
using std::optional, std::string_view;

class RequestHandler {
public:

    RequestHandler(TransportCatalogue& db);
    // optional<StopStat> GetStopStat(const string_view& stop_name);
    // optional<BusStat> GetBusStat(const string_view& bus_name) const;
    // const std::unordered_set<BusPtr>* GetBusesByStop(const string_view& stop_name) const;
    vector<std::pair<string_view, Bus *>> GetRoutes();
    void AddStop(const Stop& stop);
	Stop* GetStop(string_view stop_name) const;
	void AddBus(const Bus& bus);
	Bus* GetBus(string_view bus_name) const;
    void SetDistance(string_view stop_from_name, string_view stop_to_name, size_t dist);
    BusStat GetBusStat(const Bus& bus) const;
    StopStat GetStopStat(const Stop& stop) const;

private:
    TransportCatalogue& db_;
    // renderer::MapRenderer& renderer_;

    // const renderer::MapRenderer& renderer_;
};