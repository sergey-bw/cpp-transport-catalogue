#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport::io {

    struct Query {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;
        std::string id;
    };

    /**
     * Парсит строку запроса
     */
    Query ParseQuery(std::string_view query);

    /**
     * Выводит информацию о маршруте
     */
    void PrintBusStat(const TransportCatalogue& tansport_catalogue, const Query& query, std::ostream& output);

    /**
     * Выводит информацию о маршрутах через остановку
     */
    void PrintStopStat(const TransportCatalogue& tansport_catalogue, const Query& query, std::ostream& output);

    /**
     * Парсит строку запроса и выдает запрашиваемые данные
     */
    void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output);

} // namespace transport::io