#pragma once

#include <iostream>

#include "domain.h"
#include "svg.h"
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace ctlg::jreader {

    using json::Document, renderer::RenderSettings;

    class JsonReader{
    public:
        JsonReader(RequestHandler& handler, std::istream& input);
        void ApplyCommands();
        json::Array GetStatsRequests() const;
        bool IsStopRequest(const json::Dict& req) const;
        bool IsBusRequest(const json::Dict& req) const;
        bool IsMapRequest(const json::Dict& req) const;
        json::Node GetStopStat(const json::Dict& req) const;
        json::Node GetBusStat(const json::Dict& req) const;
        json::Node GetMapStat(const json::Dict& req) const;
        std::string GetStats() const;
        RenderSettings GetRendererSettings() const;
    private:
        RequestHandler& handler_;
        json::Document doc_;

        
        json::Node GetUpLevelNode(const string& key_name) const;
        void AddStop(const json::Dict& stop);
        void AddStopDistances(const json::Dict& stop);
        void AddBus(const json::Dict& bus);
        string ConvertcolorToString(json::Node jColor) const;
    };
} //ctlg::jreader