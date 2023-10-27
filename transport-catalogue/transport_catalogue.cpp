#include "transport_catalogue.h"

namespace transport {

/**
 * Добавляет остановку в базу данных
 */
void TransportCatalogue::AddStop(Stop stop) {
    stops_.push_back(std::move(stop));
    stopname_to_stop_[stops_.back().id] = &stops_.back();
}

/**
 * Поиск остановки в базе данных
 * Возвращает ссылку на структуру Stop для пользовательской реализации
 */
const Stop* TransportCatalogue::FindStop(std::string_view id) const {
    if (stopname_to_stop_.count(id) == 0) {
        return nullptr;
    }
    return stopname_to_stop_.at(id);
}

/**
 * Добавляет маршрут в базу данных
 */
void TransportCatalogue::AddBus(Bus bus) {
    buses_.push_back(std::move(bus));
    busname_to_bus_[buses_.back().id] = &buses_.back();

    // Собираем информацию о маршруте
    CollectionBusInfo(&buses_.back());
}

/**
 * Поиск маршрута в базе данных
 * Возвращает указатель на структуру Bus для пользовательской реализации
 */
const Bus* TransportCatalogue::FindBus(std::string_view id) const {
    if (busname_to_bus_.count(id) == 0) {
        return nullptr;
    }
    return busname_to_bus_.at(id);
}

/**
 * Возвращает информацию о маршруте
 * Возвращает ссылку на структуру Bus для пользовательской реализации
 */
const BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
    if (buses_info_.count(bus) == 0) {
        return {};
    }
    return buses_info_.at(bus);
}

/**
 * Возвращает информацию о маршрутах через остановку
 */
std::set<std::string_view> TransportCatalogue::GetBusesOnStop(const Stop * stop) const {
    if (buses_on_stop_.count(stop) == 0) {
        return {};
    }
    return buses_on_stop_.at(stop);
}

/**
 * Сбор информации о маршрутах
 */
void TransportCatalogue::CollectionBusInfo(Bus* bus) {
    BusInfo bus_info;
    bus_info.stop_count = bus->stops.size();
    std::set<std::string_view> unique_stop;
    const Stop* prev_stop = nullptr;
    for (const Stop* stop : bus->stops) {
        unique_stop.insert(stop->id);
        if (prev_stop) {
            bus_info.length += ComputeDistance(prev_stop->coordinates, stop->coordinates);
        }
        prev_stop = stop;

        // Собираем маршруты проходящие через остановку
        buses_on_stop_[stop].insert(bus->id);
    }
    bus_info.unique_stop_count = unique_stop.size();
    buses_info_[bus] = std::move(bus_info);
}

} // namespace transport