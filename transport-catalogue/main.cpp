#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>
#include <string>

using namespace std;

int main() {
    using namespace transport_catalogue;
    TransportCatalogue tc;
    renderer::MapRenderer map;
    RequestHandler request_handler{ tc , map };

    // Читаем json
    io::JsonReader json_reader{ std::cin };

    // Формируем транспортный справочник из json
    json_reader.FillCatalogue(tc);

    // Получаем настройки Рендеринга
    map.SetRenderSettings(json_reader.GetRendererSettings());

    // Парсим запросы к БД, получаем ответ в формате json и выводим в поток
    json_reader.StatRequest(request_handler, std::cout);

    return 0;
}