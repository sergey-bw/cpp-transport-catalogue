#pragma once

#include "geo.h"

#include <cstdint>
#include <string>
#include <vector>

namespace transport_catalogue {

// Остановки
struct Stop {
    std::string id;
    geo::Coordinates coordinates;
};

// Маршруты
struct Bus {
    std::string id;
    std::vector<const Stop*> stops;
    bool is_roundtrip = false;
    const Stop* last_stop = nullptr;
};

// Информация о маршруте
struct BusStat {
    size_t stop_count = 0;
    size_t unique_stop_count = 0;
    uint32_t length = 0;
    double curvature = 0;
};

// Построение маршрута

enum class ItemType {
	BUS,
	WAIT,
};

struct RouteItem {
    ItemType type;
    double time;
    std::string_view name;
    int span_count = 0;
};

struct RouteInfo {
	std::vector<RouteItem> items;
	double route_time = 0;
};

} // namespace transport_catalogue