#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using std::string, std::string_view, std::vector, std::cout, std::endl;
using namespace std::literals;
using namespace ctlg;
using namespace renderer;


int main() {
    TransportCatalogue catalogue;
    RenderSettings sett;
    RequestHandler handler{catalogue};
    ctlg::jreader::JsonReader jreader(handler, std::cin);
    jreader.ApplyCommands();
    MapRenderer rend{jreader.GetRendererSettings()};
    auto routes = handler.GetRoutes();
    rend.SetRoutes(routes);
    svg::Document doc = rend.RenderMap();
    doc.Render(std::cout);
    cout << jreader.GetStats();
    return 0;
}
