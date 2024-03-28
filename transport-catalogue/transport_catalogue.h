#pragma once

#include "geo.h"
#include "domain.h"

#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace transport_catalogue {

class DistanceHasher {
public:
	size_t operator()(std::pair<const Stop*, const Stop*> key) const;

private:
	std::hash<const void*> hasher;
};

class TransportCatalogue {
public:

	/**
	 * Добавляет остановку в базу данных
	 */
	void AddStop(std::string id, geo::Coordinates coordinates);


	/**
	 * Поиск остановки в базе данных
	 */
	const Stop* FindStop(std::string_view id) const noexcept;

	/**
	 * Добавляет дистанцию между остановками в базу данных
	 */
	void AddDistance(const Stop* from, const Stop* to, uint32_t distance);

	/**
	 * Добавляет маршрут в базу данных
	 */
	void AddBus(std::string id, std::vector<const Stop*> stops, bool is_roundtrip = false, const Stop* last_stop = nullptr);

	/**
	 * Поиск маршрута в базе данных
	 */
	const Bus* FindBus(std::string_view id) const noexcept;

	/**
	 * Возвращает информацию о маршруте
	 */
	const BusStat GetBusStat(const Bus* bus) const noexcept;

	/**
	 * Возвращает информацию о маршрутах через остановку
	 */
	std::set<std::string_view> GetBusesOnStop(const Stop*) const noexcept;

	/**
	* Возвращает количество остановок в справочнике
	*/
	size_t GetStopsCount() const noexcept;

	/**
	* Возвращает все маршруты в сортированном контейнере
	*/
	const std::map<std::string_view, const Bus*> GetAllBusesSorted() const;

	/**
	* Возвращает все маршруты в сортированном контейнере
	*/
	std::unordered_map<std::string_view, const Bus*> GetAllBuses() const;

	/**
	* Возвращает все остановки
	*/
	const std::unordered_map<std::string_view, const Stop*> GetAllStops() const;

	/**
	* Возвращает дистанцию между остановками
	*/
	uint32_t GetDistance(const Stop* from, const Stop* to) const;

private:
	// Остановки
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

	// Маршруты
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

	// Дистанции
	std::unordered_map<std::pair<const Stop*, const Stop*>, uint32_t, DistanceHasher> distanses_;

	// Информация о маршрутах
	std::unordered_map <const Bus*, BusStat> buses_stat_;

	// Список маршрутов через остановку
	std::unordered_map<const Stop*, std::set<std::string_view>> buses_on_stop_;

	/**
	 * Сбор информации о маршрутах
	 */
	void CollectionBusStat(const Bus* bus);
};

} // namespace transport