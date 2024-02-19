#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

#include <optional>
#include <unordered_set>

namespace transport_catalogue {

using BusPtr = const Bus*;
using StopPtr = const Stop*;

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& tc, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::optional <const std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;
};

} //  namespace transport_catalogue