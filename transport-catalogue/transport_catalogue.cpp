#include <stdexcept>

#include "transport_catalogue.h"

namespace transport {

/**
 * Добавляет остановку в базу данных
 */
void TransportCatalogue::AddStop(std::string id, geo::Coordinates coordinate) {
    stops_.push_back({ std::move(id), std::move(coordinate) });
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
void TransportCatalogue::AddBus(std::string id, std::vector<const Stop*> stops) {
    buses_.push_back({ std::move(id), std::move(stops) });
    busname_to_bus_[buses_.back().id] = &buses_.back();

    // Собираем информацию о маршруте
    CollectionBusInfo(&buses_.back());
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
const BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const noexcept {
    if (buses_info_.count(bus) == 0) {
        return {};
    }
    return buses_info_.at(bus);
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
void TransportCatalogue::CollectionBusInfo(const Bus* bus) {
    BusInfo bus_info;
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
    buses_info_[bus] = std::move(bus_info);
}

} // namespace transport