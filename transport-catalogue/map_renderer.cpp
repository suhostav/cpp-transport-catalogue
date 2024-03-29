#include <set>

#include "map_renderer.h"

namespace renderer {

using namespace svg;

void MapRenderer::SetRoutes(vector<std::pair<string_view, Bus*>> routes){
    routes_ = routes;
}

vector<vector<geo::Coordinates>> MapRenderer::GetAllPoints() {
    vector<vector<geo::Coordinates>> lines;
    for( auto route : routes_){
        Bus *bus = route.second;
        vector<geo::Coordinates> line;
        for( auto stop : bus->stops_){
            line.push_back(stop->coord_);
        }
        lines.push_back(line);
    }
    return lines;
}

svg::Color MapRenderer::RouteNumerToColor(size_t number) {
    size_t index = number % settings_.color_palette.size();
    return settings_.color_palette[index];
}

SphereProjector MapRenderer::SetCoeffs(vector<vector<geo::Coordinates>>& routes){
    vector<geo::Coordinates> points;
    for( auto route : routes){
        for( auto p : route){
            points.push_back(p);
        }
    }
    SphereProjector proj{points.begin(), 
        points.end(), 
        settings_.width, 
        settings_.height, 
        settings_.padding};

    return proj;
}

svg::Document& MapRenderer::RenderRouts(
                vector<vector<geo::Coordinates>>& routes, 
                SphereProjector& proj, 
                svg::Document& svg_doc) {
    vector<svg::Polyline> lines;
    size_t colors = settings_.color_palette.size();
    size_t color_index = 0;
    for( auto route : routes){
        Polyline line;
        for(auto p : route){
            line.AddPoint(proj(p));
        }
        lines.push_back(line
            .SetStrokeColor(settings_.color_palette[color_index])
            .SetStrokeWidth(settings_.line_width)
            .SetFillColor("none")
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        ++color_index;
        if(color_index >= colors){
            color_index = 0;
        }
    }
        for(auto line : lines){
        svg_doc.Add(line);
    }
    return svg_doc;
}

svg::Document& MapRenderer::RenderBusNames(SphereProjector& proj, svg::Document& svg_doc){
    size_t route_num = 0;
    for(auto [bus_name, bus] : routes_){
        if(bus->stops_.size() == 0) {
            continue;
        }
        svg::Text first_underline;
        first_underline.SetData(string(bus_name))
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetPosition(proj(bus->stops_[0]->coord_))
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s);

        svg::Text fist_label;
        fist_label.SetData(string(bus_name))
        .SetFillColor(RouteNumerToColor(route_num))
        .SetPosition(proj(bus->stops_[0]->coord_))
        .SetOffset(settings_.bus_label_offset)
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s);

        svg_doc.Add(first_underline);
        svg_doc.Add(fist_label);

        if(!bus->IsRound()){
            Stop *last_stop = bus->GetLastStop();
            if(last_stop != bus->stops_[0]){
                svg::Text last_underline;
                last_underline.SetData(string(bus_name))
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(proj(last_stop->coord_))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);

                svg::Text last_label;
                last_label.SetData(string(bus_name))
                .SetFillColor(RouteNumerToColor(route_num))
                .SetPosition(proj(last_stop->coord_))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s);

                svg_doc.Add(last_underline);
                svg_doc.Add(last_label);
            }
        }
        ++route_num;
    }
    return svg_doc;
}

vector<Stop*> MapRenderer::SortedRoutesStops() {
    std::set<Stop*> routes_stops;
    for(auto route : routes_){
        routes_stops.insert(route.second->stops_.begin(), route.second->stops_.end());
    }
    vector<Stop*> stops{routes_stops.begin(), routes_stops.end()};
    std::sort(stops.begin(), stops.end(),[](auto lhs, auto rhs){ return lhs->name_ < rhs->name_;});
    return stops;
}

svg::Document& MapRenderer::RenderStops(SphereProjector& proj, svg::Document& svg_doc) {
    auto stops = SortedRoutesStops();
    for( auto stop : stops){
        svg::Circle stop_circle;
        stop_circle.SetCenter(proj(stop->coord_))
        .SetRadius(settings_.stop_radius)
        .SetFillColor("white");
        svg_doc.Add(stop_circle);
    }
    for( auto stop : stops){
        svg::Text stop_underline;
        stop_underline.SetData(string(stop->name_))
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetPosition(proj(stop->coord_))
        .SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s);

        svg::Text stop_label;
        stop_label.SetData(string(stop->name_))
        .SetFillColor("black"s)
        .SetPosition(proj(stop->coord_))
        .SetOffset(settings_.stop_label_offset)
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s);

        svg_doc.Add(stop_underline);
        svg_doc.Add(stop_label);
    }
    return svg_doc;
}

svg::Document MapRenderer::RenderMap() {
    svg::Document svg_doc;
    auto points = GetAllPoints();
    SphereProjector proj = SetCoeffs(points);
    RenderRouts(points, proj, svg_doc);
    RenderBusNames(proj, svg_doc);
    RenderStops(proj, svg_doc);
    return svg_doc;
}

}   //renderer