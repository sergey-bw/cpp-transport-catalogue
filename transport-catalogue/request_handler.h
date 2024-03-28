#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <optional>
#include <unordered_set>

namespace transport_catalogue {

using BusPtr = const Bus*;
using StopPtr = const Stop*;

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& tc, const renderer::MapRenderer& renderer, const TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::optional<const std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

    std::optional<RouteInfo> GetRoute(const std::string_view from, const std::string_view to) const;

private:
    const TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& router_;
};

} //  namespace transport_catalogue