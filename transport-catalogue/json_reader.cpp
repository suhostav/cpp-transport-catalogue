#include <algorithm>
#include <stdexcept>
#include <sstream>
#include "json_reader.h"
#include "map_renderer.h"

namespace ctlg::jreader {

using namespace std::literals;

JsonReader::JsonReader(RequestHandler& handler, std::istream& input):
    handler_(handler), doc_(json::Load(input)){

}

json::Node JsonReader::GetUpLevelNode(const string& key_name) const {
    const json::Node& root = doc_.GetRoot();
    if(!root.IsDict()){
        return {};
    }
    const json::Dict& requests = root.AsDict();
    if(requests.count(string{key_name})) {
        return requests.at(key_name);
    }
    return {};
}

void JsonReader::ApplyCommands(){
    json::Node base_requests = GetUpLevelNode("base_requests"s);
    if(base_requests == json::Node{}){
        return;
    }
    const json::Array& commands = base_requests.AsArray();
    //add all Stops
    for(const json::Node& command : commands){
        const json::Dict& req = command.AsDict();
        if(IsStopRequest(req)){
            AddStop(req);
        }
    }
    // add stops distances
    for(const json::Node& command : commands){
        const json::Dict& req = command.AsDict();
        if(IsStopRequest(req)){
            AddStopDistances(req);
        }
    }
    //add buses
    for(const json::Node& command : commands){
        const json::Dict& req = command.AsDict();
        if(IsBusRequest(req)){
            AddBus(req);
        }
    }
}

bool JsonReader::IsStopRequest(const json::Dict& req) const{
    return req.at("type"s).AsString() ==  "Stop"s;
}
bool JsonReader::IsBusRequest(const json::Dict& req) const{
    return req.at("type"s).AsString() ==  "Bus"s;
}

bool JsonReader::IsMapRequest(const json::Dict& req) const {
    return req.at("type"s).AsString() ==  "Map"s;
}

bool JsonReader::IsRouteRequest(const json::Dict& req) const {
    return req.at("type"s).AsString() ==  "Route"s;
}

void JsonReader::AddStop(const json::Dict& stop){
    Stop new_stop{stop.at("name").AsString(), {stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble()}};
    handler_.AddStop(new_stop);
}

void JsonReader::AddStopDistances(const json::Dict& stop){
    const json::Dict& dists = stop.at("road_distances").AsDict();
    for(auto& [stop_to, dist] : dists){
        handler_.SetDistance(stop.at("name").AsString(), stop_to, dist.AsInt());
    }
}

void JsonReader::AddBus(const json::Dict& bus){
    std::string bus_name = bus.at("name").AsString();
    std::vector<std::string> stops_names;
    const json::Array& stops = bus.at("stops").AsArray();
    bool is_roundtrip = bus.at("is_roundtrip").AsBool();
    // Stop* last_stop = nullptr;
    for(const json::Node& name : stops){
        stops_names.push_back(name.AsString());
    }
    // if(!is_roundtrip && stops_names.size() > 0){
    //     last_stop = handler_.GetStop(stops_names.back());
    // }
    vector<string_view> results(stops_names.begin(), stops_names.end());
    if(!is_roundtrip){
        results.insert(results.end(), std::next(stops_names.rbegin()), stops_names.rend());
    }
    std::vector<Stop*> route_stops;
    for(string_view& stop : results){
        route_stops.push_back(handler_.GetStop(stop));
    }
    handler_.AddBus(Bus{std::move(bus_name), std::move(route_stops), is_roundtrip});
}

json::Node JsonReader::GetStopStat(const json::Dict& req) const {
    int id_node = req.at("id").AsInt();
    const string& stop_name = req.at("name").AsString();
    StopPtr stop = handler_.GetStop(stop_name);
    if(!stop){
        return json::Builder()
            .StartDict()
                .Key("request_id"s).Value(id_node)
                .Key("error_message"s).Value("not found"s)
            .EndDict().Build();
    } else {
        StopStat stat = handler_.GetStopStat(*stop);
        json::Array buses;
        for(string_view& bus_name : stat.buses){
            buses.push_back(string{bus_name});
        }
        return json::Builder().StartDict()
            .Key("request_id"s).Value(id_node)
            .Key("buses"s).Value(buses)
            .EndDict().Build();
    }
}

json::Node JsonReader::GetBusStat(const json::Dict& req) const {
    int id_node = req.at("id").AsInt();
    const string& bus_name = req.at("name").AsString();
    BusPtr bus = handler_.GetBus(bus_name);
    if(!bus){
        return json::Builder().StartDict()
            .Key("request_id"s).Value(id_node)
            .Key("error_message"s).Value("not found"s)
            .EndDict().Build();
    } else {
        BusStat stat = handler_.GetBusStat(*bus);
        double curvature = static_cast<double>(stat.lenght) / stat.lenght_geo;
        return json::Builder().StartDict()
            .Key("request_id"s).Value(id_node)
            .Key("stop_count"s).Value((int)stat.stops)
            .Key("unique_stop_count"s).Value((int)stat.unique_stops)
            .Key("route_length"s).Value((double)stat.lenght)    //изменено здесь
            .Key("curvature"s).Value(curvature)
            .EndDict().Build();
    }
}

json::Node JsonReader::GetMapStat(const json::Dict& req) const {
    int id = req.at("id").AsInt();

    std::stringstream ss;
    renderer::RenderSettings sett = GetRendererSettings();
    renderer::MapRenderer renderer{sett};
    // auto routes = handler_.GetRoutes();
    renderer.SetRoutes(handler_.GetRoutes());
    // renderer.SetRoutes(routes);
    svg::Document doc = renderer.RenderMap();
    doc.Render(ss);
    return json::Builder()
        .StartDict()
            .Key("request_id"s).Value(id)
            .Key("map"s).Value(ss.str())
        .EndDict().Build();
}

json::Node JsonReader::GetRouteStat(const json::Dict& req, const transport_router& tr_router) const {
    int id = req.at("id").AsInt();
    const string& stop_from = req.at("from"s).AsString();
    const string& stop_to = req.at("to"s).AsString();
    auto route_info = tr_router.CreateRoute(stop_from, stop_to);
    if(!route_info){
        return json::Builder()
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s)
            .EndDict().Build();
    } else {
        json::Array items;
        for(graph::Edge<double>& edge : route_info->edges){
            if(edge.span_count == 0){
            items.push_back(
                json::Builder()
                    .StartDict()
                        .Key("stop_name"s).Value(string(edge.bus_name))
                        .Key("time"s).Value(edge.weight)
                        .Key("type"s).Value("Wait"s)
                    .EndDict().Build()
                );
            } else {
                items.push_back(
                    json::Builder()
                        .StartDict()
                            .Key("bus"s).Value(string(edge.bus_name))
                            .Key("span_count"s).Value((int)edge.span_count)
                            .Key("time"s).Value(edge.weight)
                            .Key("type"s).Value("Bus"s)
                        .EndDict().Build()
                    );
            }
        }

        return json::Builder()
            .StartDict()
                .Key("items"s).Value(items)
                .Key("request_id"s).Value(id)
                .Key("total_time"s).Value(route_info->total_time)
            .EndDict().Build();
    }
}

std::string JsonReader::GetStats() const{
    std::stringstream out;
    json::Array answers;
    json::Node stat_requests = GetUpLevelNode("stat_requests"s);
    if(!stat_requests.IsArray()){
        return ""s;
    }
    RoutingSettings route_settings = GetRoutingSettings();
    transport_router router{handler_.GetCatalogue(), route_settings};
    router.CreateAllData();
    const json::Array& reqs = stat_requests.AsArray();
    for(const json::Node& req : reqs){
        if(IsBusRequest(req.AsDict())){
            answers.push_back(GetBusStat(req.AsDict()));
        } else if(IsStopRequest(req.AsDict())){
            answers.push_back(GetStopStat(req.AsDict()));
        } else if(IsMapRequest(req.AsDict())){
            answers.push_back(GetMapStat(req.AsDict()));
        } else if(IsRouteRequest(req.AsDict())){
            answers.push_back(GetRouteStat(req.AsDict(), router));
        } else {
            throw std::runtime_error("JsonReader::GetStats: Unknown stat request type. "s);
        }
    }
    json::Document doc{answers};
    json::Print(doc, out);
    return out.str();
}

RenderSettings JsonReader::GetRendererSettings() const{
    RenderSettings sett;
    json::Node render_settings = GetUpLevelNode("render_settings"s);
    if(render_settings == json::Node{}){
        return {};
    }
    const json::Dict& sn = render_settings.AsDict();
    sett.width = sn.at("width"s).AsDouble();
    sett.height = sn.at("height").AsDouble();
    sett.padding = sn.at("padding"s).AsDouble();
    
    sett.line_width = sn.at("line_width"s).AsDouble();
    sett.stop_radius = sn.at("stop_radius"s).AsDouble();
    sett.bus_label_font_size = (uint32_t)sn.at("bus_label_font_size").AsInt();

    const json::Array& boffset = sn.at("bus_label_offset"s).AsArray();
    sett.bus_label_offset = {boffset[0].AsDouble(), boffset[1].AsDouble()};

    sett.stop_label_font_size = (uint32_t)sn.at("stop_label_font_size"s).AsInt();
    const json::Array& soffset = sn.at("stop_label_offset"s).AsArray();
    sett.stop_label_offset = {soffset[0].AsDouble(), soffset[1].AsDouble()};

    sett.underlayer_color = ConvertcolorToString(sn.at("underlayer_color"));
    sett.underlayer_width = sn.at("underlayer_width").AsDouble();

    sett.color_palette.clear();
    const json::Array& sp = sn.at("color_palette"s).AsArray();
    for( const json::Node& nc : sp){
        sett.color_palette.push_back(ConvertcolorToString(nc));
    }

    return sett;
}

RoutingSettings JsonReader::GetRoutingSettings() const{
    RoutingSettings settings{1001, 1001.0};
    json::Node routing_settings = GetUpLevelNode("routing_settings"s);
    if(routing_settings == json::Node{}){
        return settings;
    }
    const json::Dict& rs = routing_settings.AsDict();
    settings.bus_wait_time = (unsigned int)rs.at("bus_wait_time"s).AsInt();
    settings.bus_velocity = rs.at("bus_velocity"s).AsDouble();

    return settings;
}

string JsonReader::ConvertcolorToString(json::Node jColor) const{
    if(jColor.IsString()){
        return jColor.AsString();
    }
    std::stringstream ss;
    const json::Array& arr = jColor.AsArray();
    if(arr.size() == 3){
        ss << "rgb("s 
            << arr[0].AsInt() << ','
            << arr[1].AsInt() << ','
            << arr[2].AsInt() << ')'; 
    } else if(arr.size() == 4){
        ss << "rgba("s 
            << arr[0].AsInt() << ','
            << arr[1].AsInt() << ','
            << arr[2].AsInt() << ',' 
            << arr[3].AsDouble() << ')';
    }
    return ss.str();
}

}   //ctlg::jreader