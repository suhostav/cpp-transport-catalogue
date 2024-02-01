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
    auto root = doc_.GetRoot();
    if(!root.IsMap()){
        return {};
    }
    auto requests = root.AsMap();
    if(requests.count(string{key_name})) {
        return requests.at(key_name);
    }
    return {};
}

void JsonReader::ApplyCommands(){
    auto base_requests = GetUpLevelNode("base_requests"s);
    if(base_requests == json::Node{}){
        return;
    }
    auto commands = base_requests.AsArray();
    //add all Stops
    for(auto command : commands){
        auto req = command.AsMap();
        if(IsStopRequest(req)){
            AddStop(req);
        }
    }
    // add stops distances
    for(auto command : commands){
        auto req = command.AsMap();
        if(IsStopRequest(req)){
            AddStopDistances(req);
        }
    }
    //add buses
    for(auto command : commands){
        auto req = command.AsMap();
        if(IsBusRequest(req)){
            AddBus(req);
        }
    }
}

bool JsonReader::IsStopRequest(const json::Dict& req) const{
    return req.at("type"s).AsString() ==  "Stop"s;
    // if(req.at("type"s).AsString() ==  "Stop"s){
    //     return true;
    // }
    // return false;
}
bool JsonReader::IsBusRequest(const json::Dict& req) const{
    return req.at("type"s).AsString() ==  "Bus"s;
    // if(req.at("type"s).AsString() ==  "Bus"s){
    //     return true;
    // }
    // return false;
}

bool JsonReader::IsMapRequest(const json::Dict& req) const {
    return req.at("type"s).AsString() ==  "Map"s;
    // if(req.at("type"s).AsString() ==  "Map"s){
    //     return true;
    // }
    // return false;
}


void JsonReader::AddStop(const json::Dict& stop){
    Stop new_stop{stop.at("name").AsString(), {stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble()}};
    handler_.AddStop(new_stop);
}

void JsonReader::AddStopDistances(const json::Dict& stop){
    auto dists = stop.at("road_distances").AsMap();
    for(auto [stop_to, dist] : dists){
        handler_.SetDistance(stop.at("name").AsString(), stop_to, dist.AsInt());
    }
}

void JsonReader::AddBus(const json::Dict& bus){
    std::string bus_name = bus.at("name").AsString();
    std::vector<std::string> stops_names;
    auto stops = bus.at("stops").AsArray();
    bool is_roundtrip = bus.at("is_roundtrip").AsBool();
    Stop* last_stop = nullptr;
    for(auto name : stops){
        stops_names.push_back(name.AsString());
    }
    if(!is_roundtrip && stops_names.size() > 0){
        last_stop = handler_.GetStop(stops_names.back());
    }
    vector<string_view> results(stops_names.begin(), stops_names.end());
    if(!is_roundtrip){
        results.insert(results.end(), std::next(stops_names.rbegin()), stops_names.rend());
    }
    std::vector<Stop*> route_stops;
    for(auto stop : results){
        route_stops.push_back(handler_.GetStop(stop));
    }
    handler_.AddBus(Bus{std::move(bus_name), std::move(route_stops), last_stop});
}

json::Node JsonReader::GetStopStat(const json::Dict& req) const {
    auto id_node = req.at("id").AsInt();
    auto stop_name = req.at("name").AsString();
    StopPtr stop = handler_.GetStop(stop_name);
    json::Dict answer;
    answer["request_id"s] = id_node;
    if(!stop){
        answer["error_message"s] = "not found"s;
    } else {
        StopStat stat = handler_.GetStopStat(*stop);
        json::Array buses;
        for(auto bus_name : stat.buses){
            buses.push_back(string{bus_name});
        }
        answer["buses"s] = buses;
    }
    return {answer};
}

json::Node JsonReader::GetBusStat(const json::Dict& req) const {
    auto id_node = req.at("id").AsInt();
    auto bus_name = req.at("name").AsString();
    BusPtr bus = handler_.GetBus(bus_name);
    json::Dict answer;
    answer["request_id"s] = id_node;
    if(!bus){
        answer["error_message"s] = "not found"s;
    } else {
        BusStat stat = handler_.GetBusStat(*bus);
        double curvature = static_cast<double>(stat.lenght) / stat.lenght_geo;
        answer["stop_count"] = (int)stat.stops;
        answer["unique_stop_count"] = (int)stat.unique_stops;
        answer["route_length"] = stat.lenght;
        answer["curvature"] = curvature;
    }
    return {answer};
}

json::Node JsonReader::GetMapStat(const json::Dict& req) const {
    auto id = req.at("id").AsInt();

    std::stringstream ss;
    auto sett = GetRendererSettings();
    renderer::MapRenderer renderer{sett};
    auto routes = handler_.GetRoutes();
    renderer.SetRoutes(routes);
    svg::Document doc = renderer.RenderMap();
    doc.Render(ss);
    // auto map_str = AddEscapes(ss);
    json::Dict answer;
    answer["request_id"s] = id;
    answer["map"s] = ss.str();

    return {answer};
}

std::string JsonReader::GetStats() const{
    std::stringstream out;
    json::Array answers;
    auto stat_requests = GetUpLevelNode("stat_requests"s);
    if(!stat_requests.IsArray()){
        return ""s;
    }
    auto reqs = stat_requests.AsArray();
    for(auto req : reqs){
        if(IsBusRequest(req.AsMap())){
            answers.push_back(GetBusStat(req.AsMap()));
        } else if(IsStopRequest(req.AsMap())){
            answers.push_back(GetStopStat(req.AsMap()));
        } else if(IsMapRequest(req.AsMap())){
            answers.push_back(GetMapStat(req.AsMap()));
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
    auto render_settings = GetUpLevelNode("render_settings"s);
    if(render_settings == json::Node{}){
        return {};
    }
    auto sn = render_settings.AsMap();
    sett.width = sn.at("width"s).AsDouble();
    sett.height = sn.at("height").AsDouble();
    sett.padding = sn.at("padding"s).AsDouble();
    
    sett.line_width = sn.at("line_width"s).AsDouble();
    sett.stop_radius = sn.at("stop_radius"s).AsDouble();
    sett.bus_label_font_size = (uint32_t)sn.at("bus_label_font_size").AsInt();

    auto boffset = sn.at("bus_label_offset"s).AsArray();
    sett.bus_label_offset = {boffset[0].AsDouble(), boffset[1].AsDouble()};

    sett.stop_label_font_size = (uint32_t)sn.at("stop_label_font_size"s).AsInt();
    auto soffset = sn.at("stop_label_offset"s).AsArray();
    sett.stop_label_offset = {soffset[0].AsDouble(), soffset[1].AsDouble()};

    sett.underlayer_color = ConvertcolorToString(sn.at("underlayer_color"));
    sett.underlayer_width = sn.at("underlayer_width").AsDouble();

    sett.color_palette.clear();
    auto sp = sn.at("color_palette"s).AsArray();
    for( auto nc : sp){
        sett.color_palette.push_back(ConvertcolorToString(nc));
    }

    return sett;
}

string JsonReader::ConvertcolorToString(json::Node jColor) const{
    if(jColor.IsString()){
        return jColor.AsString();
    }
    std::stringstream ss;
    auto arr = jColor.AsArray();
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