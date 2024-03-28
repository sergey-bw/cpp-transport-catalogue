#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

RequestHandler::RequestHandler(const TransportCatalogue& tc, const renderer::MapRenderer& renderer, const TransportRouter& router)
    : tc_(tc)
    , renderer_(renderer)
    , router_(router)
{
}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    if (auto bus_ptr = tc_.FindBus(bus_name); bus_ptr != nullptr) {
        return tc_.GetBusStat(bus_ptr);
    }
    return std::nullopt;
}

// Возвращает маршруты, проходящие через
std::optional <const std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    StopPtr stop_ptr = tc_.FindStop(stop_name);
    if (stop_ptr == nullptr) {
        return std::nullopt;
    }
    return tc_.GetBusesOnStop(tc_.FindStop(stop_name));
}

// Возвращает SVG документ с картой маршрутов
svg::Document RequestHandler::RenderMap() const
{
    return renderer_.GetSvgDocument(tc_.GetAllBusesSorted());
}

// Возвращает информацию о построенном маршруте
std::optional<RouteInfo> RequestHandler::GetRoute(const std::string_view from, const std::string_view to) const {
    return router_.BuildRoute(from, to);
}

}  // namespace transport_catalogue