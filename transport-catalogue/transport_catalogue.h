#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace transport {

// Остановки
struct Stop {
	std::string id;
	geo::Coordinates coordinates;
};

// Маршруты
struct Bus {
	std::string id;
	std::vector<const Stop*> stops;
};

// Информация о маршруте
struct BusInfo {
	size_t stop_count = 0;
	size_t unique_stop_count = 0;
	double length = 0;
};

class TransportCatalogue {
public:
	/**
	 * Добавляет остановку в базу данных
	 */
	void AddStop(std::string id, geo::Coordinates coordinate);


	/**
	 * Поиск остановки в базе данных
	 */
	const Stop* FindStop(std::string_view id) const noexcept;

	/**
	 * Добавляет маршрут в базу данных
	 */
	void AddBus(std::string id, std::vector<const Stop*> stops);

	/**
	 * Поиск маршрута в базе данных
	 */
	const Bus* FindBus(std::string_view id) const noexcept;

	/**
	 * Возвращает информацию о маршруте
	 */
	const BusInfo GetBusInfo(const Bus* bus) const noexcept;

	std::set<std::string_view> GetBusesOnStop(const Stop*) const noexcept;

private:
	// Остановки
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

	// Маршруты
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

	// Информация о маршрутах
	std::unordered_map <const Bus*, BusInfo> buses_info_;

	// Список маршрутов через остановку
	std::unordered_map<const Stop*, std::set<std::string_view>> buses_on_stop_;

	/**
	 * Сбор информации о маршрутах
	 */
	void CollectionBusInfo(const Bus* bus);
};

} // namespace transport