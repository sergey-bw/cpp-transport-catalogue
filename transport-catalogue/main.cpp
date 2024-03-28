#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

#include <iostream>
#include <string>

using namespace std;

int main() {
    using namespace transport_catalogue;

    // Читаем json
    io::JsonReader json_reader{ std::cin };

    // Формируем транспортный справочник из json
    TransportCatalogue tc;
    json_reader.FillCatalogue(tc);

    renderer::MapRenderer map(json_reader.GetRendererSettings());
    TransportRouter router(tc, json_reader.GetRoutingSettings());
    RequestHandler request_handler{ tc, map, router };
    json_reader.StatRequest(request_handler, std::cout);

    return 0;
}