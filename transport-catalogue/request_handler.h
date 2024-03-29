#pragma once

#include <optional>
#include <memory>
#include <unordered_set>
#include "svg.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"

// using namespace ctlg::jreader;
using namespace ctlg;
using std::optional, std::string_view;

class RequestHandler {
public:

    RequestHandler(TransportCatalogue& db);
    vector<std::pair<string_view, Bus *>> GetRoutes();
    void AddStop(const Stop& stop);
	Stop* GetStop(string_view stop_name) const;
	void AddBus(const Bus& bus);
	Bus* GetBus(string_view bus_name) const;
    void SetDistance(string_view stop_from_name, string_view stop_to_name, int dist);
    BusStat GetBusStat(const Bus& bus) const;
    StopStat GetStopStat(const Stop& stop) const;
    const TransportCatalogue& GetCatalogue() const { return db_;}
private:
    TransportCatalogue& db_;
};