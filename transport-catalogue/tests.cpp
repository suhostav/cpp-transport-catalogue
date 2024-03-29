#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "log_duration.h"
#include "graph.h"
#include "router.h"

using std::string, std::string_view, std::vector, std::cout, std::endl;
using namespace std::literals;
using namespace ctlg;
using namespace renderer;

int TestJsonReader() {
    TransportCatalogue catalogue;
    RenderSettings sett;
    MapRenderer rend{sett};
    RequestHandler handler{catalogue};
    std::ifstream input{"input.json"};
    ctlg::jreader::JsonReader jreader(handler, input);
    // ctlg::jreader::JsonReader jreader(cin);
    jreader.ApplyCommands();
    cout << jreader.GetStats();

    return 0;
}

using namespace renderer;

void TestSphereProjector() {
    using namespace std;

    const double WIDTH = 600.0;
    const double HEIGHT = 400.0;
    const double PADDING = 50.0;
    
    // Точки, подлежащие проецированию
    vector<geo::Coordinates> geo_coords = {
        {43.587795, 39.716901}, {43.581969, 39.719848}, {43.598701, 39.730623},
        {43.585586, 39.733879}, {43.590317, 39.746833}
    };

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), WIDTH, HEIGHT, PADDING
    };

    // Проецируем и выводим координаты
    for (const auto &geo_coord: geo_coords) {
        const svg::Point screen_coord = proj(geo_coord);
        cout << '(' << geo_coord.lat << ", "sv << geo_coord.lng << ") -> "sv;
        cout << '(' << screen_coord.x << ", "sv << screen_coord.y << ')' << endl;
    }
}

void TestBusesSort(){
    TransportCatalogue catalogue;
    RenderSettings sett;
    MapRenderer rend{sett};
    RequestHandler handler{catalogue};
    std::ifstream input{"input.json"};
    ctlg::jreader::JsonReader jreader(handler, input);
    // ctlg::jreader::JsonReader jreader(cin);
    jreader.ApplyCommands();

}

void TestRenderRoutes(){
    TransportCatalogue catalogue;
    RenderSettings sett;
    RequestHandler handler{catalogue};
    try{
        LOG_DURATION("Total: "s);
        std::ifstream input{"in/in.json"};
        ctlg::jreader::JsonReader jreader(handler, input);
        jreader.ApplyCommands();
        MapRenderer rend{jreader.GetRendererSettings()};
        auto routes = handler.GetRoutes();
        rend.SetRoutes(routes);
        svg::Document doc = rend.RenderMap();
        std::ofstream out{"out/myout.svg"};
        // doc.Render(out);
        out << jreader.GetStats() << endl;
    } catch( std::exception& e){
        cout << e.what() << endl;
    }
}

void BuildRoute(transport_router::BusGraph& graph, size_t from, size_t to, double estimate, ostream& out){
    graph::Router router(graph);
    std::optional<graph::Router<double>::RouteInfo> route = router.BuildRoute(from,to);
    if(!route){
        out << "no route\n";
    } else {
        auto info = *route;
        out << "Time = " << info.weight << endl;
        for(auto& edgeId : info.edges){
            const graph::Edge<double>& edge = graph.GetEdge(edgeId);
            if(edge.span_count == 0){
                out << "wait: " << edge.bus_name << ", time = " << edge.weight << endl;
            } else {
                out << "bus: " << edge.bus_name << ", from: " << edge.from << ", to = " << edge.to << ", time = " << edge.weight << endl;
            }
        }
    }
    assert(std::abs((*route).weight - estimate) < 1.0E-6);
}

void TestGraphBuilding0(){
    std::ifstream in{"in.txt"};
    std::ofstream out{"out.txt", std::ios::trunc};
    TransportCatalogue catalogue;
    RenderSettings sett;
    RequestHandler handler{catalogue};
    ctlg::jreader::JsonReader jreader(handler, in);
    jreader.ApplyCommands();
    out << jreader.GetStats();
    // RoutingSettings rs =  jreader.GetRoutingSettings();
    // transport_router trouter{catalogue, rs};
    // auto N = trouter.GetGraphSize();
    // auto graph = trouter.BuildGraph();
    // graph::PrintGraph(graph, N, std::cout);
    // BuildRoute(graph, 0, 6, 24.21, cout);
    // BuildRoute(graph, 0, 2, 11.235, cout);
    // MapRenderer rend{jreader.GetRendererSettings()};
}

void TestGraphBuilding1(){
    std::ifstream in{"in1.txt"};
    TransportCatalogue catalogue;
    RenderSettings sett;
    RequestHandler handler{catalogue};
    ctlg::jreader::JsonReader jreader(handler, in);
    jreader.ApplyCommands();
    RoutingSettings rs =  jreader.GetRoutingSettings();
    transport_router trouter{catalogue, rs};
    trouter.CreateAllData();
    auto N = trouter.GetGraphSize();
    auto graph = trouter.BuildGraph();
    graph::PrintGraph(graph, N, std::cout);
    BuildRoute(graph, 0, 8, 7.42, cout);
    BuildRoute(graph, 0, 12, 11.44, cout);
    BuildRoute(graph, 4, 12, 10.7, cout);
    BuildRoute(graph, 4, 0, 8.56, cout);
    // BuildRoute(graph, 4, 12, 16.32, cout);
}


void TestsStart(){
    // TestSphereProjector();
    // TestRenderRoutes();
    // TestSimpleGraphCrearion();
    TestGraphBuilding0();
    // TestGraphBuilding1();
}