#include "transport_catalogue.h"

#include <stdexcept>

namespace transport_catalogue {

size_t DistanceHasher::operator()(std::pair<const Stop*, const Stop*> key) const {
    return hasher(key.first) + hasher(key.second) * 37;
}

/**
 * Добавляет остановку в базу данных
 */
void TransportCatalogue::AddStop(std::string id, geo::Coordinates coordinates) {
    stops_.push_back({ std::move(id), coordinates });
    stopname_to_stop_[stops_.back().id] = &stops_.back();
}

/**
 * Поиск остановки в базе данных
 * Возвращает указатель на структуру Stop для пользовательской реализации
 */
const Stop* TransportCatalogue::FindStop(std::string_view id) const noexcept {
    if (stopname_to_stop_.count(id) == 0) {
        return nullptr;
    }
    return stopname_to_stop_.at(id);
}


/**
 * Добавляет дистанцию между остановками в базу данных
 */
void TransportCatalogue::AddDistance(const Stop* from, const Stop* to, uint32_t distance) {
    distanses_[{ from, to }] = distance;
}

/**
 * Добавляет маршрут в базу данных
 */
void TransportCatalogue::AddBus(std::string id, std::vector<const Stop*> stops, bool is_roundtrip /* false */, const Stop* last_stop /* nullptr */) {
    buses_.push_back({ std::move(id), std::move(stops), is_roundtrip, last_stop});
    busname_to_bus_[buses_.back().id] = &buses_.back();

    // Собираем информацию о маршруте
    CollectionBusStat(&buses_.back());
}

/**
 * Поиск маршрута в базе данных
 * Возвращает указатель на структуру Bus для пользовательской реализации
 */
const Bus* TransportCatalogue::FindBus(std::string_view id) const noexcept {
    if (busname_to_bus_.count(id) == 0) {
        return nullptr;
    }
    return busname_to_bus_.at(id);
}

/**
 * Возвращает информацию о маршруте
 * Возвращает ссылку на структуру BusInfo для пользовательской реализации
 */
const BusStat TransportCatalogue::GetBusStat(const Bus* bus) const noexcept {
    if (buses_stat_.count(bus) == 0) {
        return {};
    }
    return buses_stat_.at(bus);
}

/**
 * Возвращает информацию о маршрутах через остановку
 */
std::set<std::string_view> TransportCatalogue::GetBusesOnStop(const Stop* stop) const noexcept {
    if (buses_on_stop_.count(stop) == 0) {
        return {};
    }
    return buses_on_stop_.at(stop);
}

/**
* Возвращает все маршруты в сортированном контейнере
*/
const std::map<std::string_view, const Bus*> TransportCatalogue::GetAllBusesSorted() const {
    return { busname_to_bus_.begin(), busname_to_bus_.end() };
}

/**
* Возвращает дистанцию между остановками
*/
uint32_t TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    if (distanses_.count({ from, to }) != 0) {
        return distanses_.at({ from, to });
    }

    if (distanses_.count({ to, from }) != 0) {
        return distanses_.at({ to, from });
    }

    using namespace std::literals;
    throw(std::invalid_argument("Distance not found"s));
}

/**
 * Сбор информации о маршрутах
 */
void TransportCatalogue::CollectionBusStat(const Bus* bus) {
    BusStat bus_info;
    bus_info.stop_count = bus->stops.size();
    std::set<std::string_view> unique_stop;
    const Stop* prev_stop = nullptr;
    for (const Stop* stop : bus->stops) {
        unique_stop.insert(stop->id);
        if (prev_stop != nullptr) {
            bus_info.length += GetDistance(prev_stop, stop);
            bus_info.curvature += ComputeDistance(prev_stop->coordinates, stop->coordinates);
        }
        prev_stop = stop;

        // Собираем маршруты проходящие через остановку
        buses_on_stop_[stop].insert(bus->id);
    }

    bus_info.unique_stop_count = unique_stop.size();
    bus_info.curvature = bus_info.length / bus_info.curvature;
    buses_stat_[bus] = std::move(bus_info);
}

} // namespace transport