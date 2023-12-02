#include <cassert>
#include <regex>

#include "input_reader.h"

namespace transport::io {

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
* Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
*/
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

/**
* Парсит строку и возвращает словарь (имя остановки, дистанция)
*/
Distances ParseDistances(const std::string& str) {
    Distances distances;

    /*
    * Нашел на Habr статью по sregex_iterator
    * Актуально для данной задачи?
    */
    using namespace std::literals;
    const std::regex directive("([0-9]+)m to (.+?)(?=,|$)"s);
    const std::vector<std::smatch> matches{
        std::sregex_iterator{cbegin(str), cend(str), directive},
        std::sregex_iterator{}
    };

    /*
    * match[i].str(0) - Строка вида "7500m to Rossoshanskaya ulitsa"
    * match[i].str(1) - Расстояние - "7500"
    * match[i].str(2) - Название улицы - "Rossoshanskaya ulitsa"
    */
    for (const auto& match : matches) {
        assert(match.ready());
        distances[std::move(match.str(2))] = static_cast<uint32_t>(std::stoi(match.str(1)));
    }
    return distances;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    using namespace std::literals;

    std::unordered_map<std::string_view, Distances> distances;

    // Остановки
    for (auto& item : commands_) {
        assert(item);
        if (item.command == "Stop"s) {
            catalogue.AddStop(item.id, std::move(ParseCoordinates(item.description)));
            distances[item.id] = ParseDistances(item.description);
        }
    }

    // Дистанции
    for (auto& [from, data] : distances) {
        for (auto& [to, distance] : data) {
            if (catalogue.FindStop(from) == nullptr
                || catalogue.FindStop(to) == nullptr) {
                    throw(std::invalid_argument("Stop not found"));
                }
            assert(catalogue.FindStop(from) != nullptr);
            assert(catalogue.FindStop(to) != nullptr);
            catalogue.AddDistance(catalogue.FindStop(from), catalogue.FindStop(to), distance);
        }
    }

    // Маршруты
    for (auto& item : commands_) {
        if (item.command != "Bus"s) {
            continue;
        }
        std::vector<const Stop*> stops;
        for (auto& stop : ParseRoute(item.description)) {
            stops.push_back(catalogue.FindStop(stop));
        }
        catalogue.AddBus(item.id, std::move(stops));
    }
}

} // namespace transport::io