#include <iostream>

#include "stat_reader.h"

namespace transport::io {

/**
 * Парсит строку запроса
 */
Query ParseQuery(std::string_view query) {
    // command
    query = query.substr(query.find_first_not_of(' '), std::string::npos);
    if (query.size() == 0) {
        return  {};
    }
    auto pos_end = query.find(' ');
    auto command = query.substr(0, pos_end);
    query = query.substr(pos_end, std::string::npos);

    // id
    query = query.substr(query.find_first_not_of(' '), std::string::npos);
    if (query.size() == 0) {
        return  {};
    }
    pos_end = query.find_last_not_of(' ');
    auto id = query.substr(0, pos_end + 1);


    return { std::string(command), std::string(id) };
}

/**
 * Выводит информацию о маршруте
 */
void PrintBusStat(const TransportCatalogue& tansport_catalogue, const Query& query, std::ostream& output) {
    const Bus* bus = tansport_catalogue.FindBus(query.id);

    using namespace std::literals;
    if (!bus) {
        output << "Bus "s << query.id << ": not found"s << std::endl;
        return;
    }

    output << "Bus "s << bus->id << ": "
        << tansport_catalogue.GetBusInfo(bus).stop_count << " stops on route, "s
        << tansport_catalogue.GetBusInfo(bus).unique_stop_count << " unique stops, "s
        << tansport_catalogue.GetBusInfo(bus).length << " route length, "s
        << tansport_catalogue.GetBusInfo(bus).curvature << " curvature"s << std::endl;
}

/**
 * Выводит информацию о маршрутах через остановку
 */
void PrintStopStat(const TransportCatalogue& tansport_catalogue, const Query& query, std::ostream& output) {
    const Stop* stop = tansport_catalogue.FindStop(query.id);

    using namespace std::literals;
    if (!stop) {
        output << "Stop "s << query.id << ": not found"s << std::endl;
        return;
    }

    if (tansport_catalogue.GetBusesOnStop(stop).size() == 0) {
        output << "Stop "s << query.id << ": no buses"s << std::endl;
        return;
    }

    output << "Stop "s << query.id << ": buses"s;
    for (const auto& stop_name : tansport_catalogue.GetBusesOnStop(stop)) {
        output << ' ' << stop_name;
    }
    output << std::endl;
}

/**
 * Парсит строку запроса и выдает запрашиваемые данные
 */
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {
    using namespace std::literals;

    Query query = ParseQuery(request);

    if (!query) {
        return;
    }

    if (query.command == "Bus"s) {
        PrintBusStat(tansport_catalogue, query, output);
        return;
    }

    if (query.command == "Stop"s) {
        PrintStopStat(tansport_catalogue, query, output);
        return;
    }

}

} // namespace transport::io