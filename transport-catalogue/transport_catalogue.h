#pragma once

#include <cstdint>
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
		uint32_t length = 0;
		double curvature = 0;
	};

	class DistanceHasher {
	public:
		size_t operator()(std::pair<const Stop*, const Stop*> key) const {
			return hasher(key.first) + hasher(key.first) * 37;
		}

	private:
		std::hash<const void*> hasher;
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
		 * Добавляет дистанцию между остановками в базу данных
		 */
		void AddDistance(const Stop* from, const Stop* to, uint32_t distance);

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

		/**
		 * Возвращает информацию о маршрутах через остановку
		 */
		std::set<std::string_view> GetBusesOnStop(const Stop*) const noexcept;

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
		std::unordered_map <const Bus*, BusInfo> buses_info_;

		// Список маршрутов через остановку
		std::unordered_map<const Stop*, std::set<std::string_view>> buses_on_stop_;

		/**
		* Возвращает дистанцию между остановками
		 */
		uint32_t GetDistance(const Stop* from, const Stop* to) const;

		/**
		 * Сбор информации о маршрутах
		 */
		void CollectionBusInfo(const Bus* bus);
	};

} // namespace transport