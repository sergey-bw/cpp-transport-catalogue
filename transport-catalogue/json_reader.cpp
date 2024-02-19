#include "json_reader.h"

#include <sstream>

namespace transport_catalogue::io {

JsonReader::JsonReader(json::Document document)
    : document_(std::move(document))
{
}

JsonReader::JsonReader(std::istream& input)
    : document_(json::Load(input))
{
}

void JsonReader::FillCatalogue(TransportCatalogue& tc) const {
    using namespace std::literals;

    //Пасинг остановок
    for (const auto& node : JsonGetBaseRequest().AsArray()) {
        if (node.AsMap().at("type"s).AsString() == "Stop"sv) {
            FillCatalogueStop(tc, node);
        }
    }

    //Пасинг дистанций
    for (const auto& node : JsonGetBaseRequest().AsArray()) {
        if (node.AsMap().at("type"s).AsString() == "Stop"sv) {
            FillCatalogueDistances(tc, node.AsMap().at("name"s).AsString(), node.AsMap().at("road_distances"s));
        }
    }

    //Пасинг маршрутов
    for (const auto& node : JsonGetBaseRequest().AsArray()) {
        if (node.AsMap().at("type"s).AsString() == "Bus"sv) {
            FillCatalogueBus(tc, node);
        }
    }
}

void JsonReader::StatRequest(RequestHandler& request_handler, std::ostream& output) const {
    using namespace std::literals;
    json::Array result; // string -> node

    for (const auto& node : JsonGetStatRequest().AsArray()) {
        json::Dict dict;
        dict.emplace("request_id", node.AsMap().at("id"s).AsInt());

        // Остановки
        if (node.AsMap().at("type"s).AsString() == "Stop"sv) {
            auto stops_set = request_handler.GetBusesByStop(node.AsMap().at("name"s).AsString());
            if (stops_set) {
                json::Array stops;
                for (auto& item : *stops_set) {
                    stops.push_back(std::string(item));
                }
                dict.emplace("buses", stops);
            } else {
                dict.emplace("error_message"s, "not found"s);
            }
        }

        // Маршруты
        if (node.AsMap().at("type"s).AsString() == "Bus"sv) {
            auto bus_stat = request_handler.GetBusStat(node.AsMap().at("name"s).AsString());
            if (bus_stat) {
                dict.emplace("stop_count"s, static_cast<int>(bus_stat->stop_count));
                dict.emplace("unique_stop_count"s, static_cast<int>(bus_stat->unique_stop_count));
                dict.emplace("route_length"s, static_cast<int>(bus_stat->length));
                dict.emplace("curvature"s, bus_stat->curvature);
            } else {
                dict.emplace("error_message"s, "not found"s);
            }
        }

        // Карта
        if (node.AsMap().at("type"s).AsString() == "Map"sv) {
            std::ostringstream map;
            request_handler.RenderMap().Render(map);
            dict.emplace("map"s, map.str());
        }

        result.push_back(dict);
    }
    json::Print(json::Document{ result }, output);
}

renderer::RenderSettings JsonReader::GetRendererSettings() const {
    // Если в json нет настроек рендеринга, возврачаем структуру с настройками по умолчанию
    if (!ISJsonRenderSettings() || JsonGetRenderSettings().IsNull()) {
        return {};
    }

    renderer::RenderSettings render_settings = {};

    render_settings.width = JsonGetRenderSettings().AsMap().at("width").AsDouble();
    render_settings.height = JsonGetRenderSettings().AsMap().at("height").AsDouble();

    render_settings.padding = JsonGetRenderSettings().AsMap().at("padding").AsDouble();

    render_settings.line_width = JsonGetRenderSettings().AsMap().at("line_width").AsDouble();
    render_settings.stop_radius = JsonGetRenderSettings().AsMap().at("stop_radius").AsDouble();

    render_settings.bus_label_font_size = JsonGetRenderSettings().AsMap().at("bus_label_font_size").AsDouble();
    render_settings.bus_label_offset = {
        JsonGetRenderSettings().AsMap().at("bus_label_offset").AsArray()[0].AsDouble(),
        JsonGetRenderSettings().AsMap().at("bus_label_offset").AsArray()[1].AsDouble(),
    };

    render_settings.stop_label_font_size = JsonGetRenderSettings().AsMap().at("stop_label_font_size").AsDouble();
    render_settings.stop_label_offset = {
        JsonGetRenderSettings().AsMap().at("stop_label_offset").AsArray()[0].AsDouble(),
        JsonGetRenderSettings().AsMap().at("stop_label_offset").AsArray()[1].AsDouble(),
    };

    render_settings.underlayer_color = JsonParseColor(JsonGetRenderSettings().AsMap().at("underlayer_color"));
    render_settings.underlayer_width = JsonGetRenderSettings().AsMap().at("underlayer_width").AsDouble();

    for (const json::Node& item : JsonGetRenderSettings().AsMap().at("color_palette").AsArray()) {
        render_settings.color_palette.push_back(JsonParseColor(item));
    }

    return render_settings;
}

const json::Node& JsonReader::JsonGetBaseRequest() const {
    using namespace std::literals;
    if (document_.GetRoot().AsMap().count("base_requests"s) == 0) {
        throw(json::ParsingError("Base request not found"));
    }
    return document_.GetRoot().AsMap().at("base_requests"s);
}

const json::Node& JsonReader::JsonGetStatRequest() const {
    using namespace std::literals;
    if (document_.GetRoot().AsMap().count("stat_requests"s) == 0) {
        throw(json::ParsingError("Stat request not found"s));
    }
    return document_.GetRoot().AsMap().at("stat_requests"s);
}

const json::Node& JsonReader::JsonGetRenderSettings() const {
    using namespace std::literals;
    if (document_.GetRoot().AsMap().count("render_settings"s) == 0) {
        throw(json::ParsingError("Render settings not found"));
    }
    return document_.GetRoot().AsMap().at("render_settings"s);
}

void JsonReader::FillCatalogueStop(TransportCatalogue& tc, const json::Node& stop_node) const {
    using namespace std::literals;
    tc.AddStop(stop_node.AsMap().at("name"s).AsString()
        , { stop_node.AsMap().at("latitude"s).AsDouble(), stop_node.AsMap().at("longitude"s).AsDouble() }
    );
}

void JsonReader::FillCatalogueDistances(TransportCatalogue& tc, const std::string_view from, const json::Node& distances_node) const {
    using namespace std::literals;
    auto stop_from = tc.FindStop(from);
    if (stop_from == nullptr) {
        throw(json::ParsingError("Failed parsing Distances: Stop \""s + std::string(from) + "\" not found"s));
    }

    for (auto& [stop, distance_node] : distances_node.AsMap()) {
        auto stop_to_ptr = tc.FindStop(stop);
        if (stop_to_ptr == nullptr) {
            throw(json::ParsingError("Failed parsing Distances: Stop \""s + stop + "\" not found"s));
        }
        tc.AddDistance(stop_from, stop_to_ptr, distance_node.AsInt());
    }
}

void JsonReader::FillCatalogueBus(TransportCatalogue& tc, const json::Node& bus_node) const {
    using namespace std::literals;
    std::vector<const Stop*> stops;
    for (auto& stop_node : bus_node.AsMap().at("stops"s).AsArray()) {
        auto stop_ptr = tc.FindStop(stop_node.AsString());
        if (stop_ptr == nullptr) {
            throw(json::ParsingError("Failed parsing Bus: Stop \""s + stop_node.AsString() + "\" not found"s));
        }
        stops.push_back(stop_ptr);
    }

    const Stop* last_stop = nullptr;

    // Если маршрут некольцевой, автобус едет по нему в обратную сторону
    if (!bus_node.AsMap().at("is_roundtrip").AsBool()) {
        last_stop = stops.back();
        stops.insert(stops.end(), std::next(stops.rbegin()), stops.rend());
    }

    tc.AddBus(bus_node.AsMap().at("name"s).AsString(), std::move(stops), bus_node.AsMap().at("is_roundtrip").AsBool(), last_stop);
}

svg::Color JsonReader::JsonParseColor(const json::Node& color_node) const {
    using namespace std::literals;
    if (color_node.IsArray()) {
        if (color_node.AsArray().size() == 3) { // RGB
            return svg::Rgb{
                static_cast<uint8_t>(color_node.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color_node.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color_node.AsArray()[2].AsInt()),
            };
        } else if (color_node.AsArray().size() == 4) { //RGBA
            return svg::Rgba{
                static_cast<uint8_t>(color_node.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color_node.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color_node.AsArray()[2].AsInt()),
                color_node.AsArray()[3].AsDouble(),
            };
        } else {
            throw(json::ParsingError("Failed parsing Color"s));
        }
    } else if (color_node.IsString()) { // string color
        return color_node.AsString();
    }

    throw(json::ParsingError("Failed parsing Color"s));
}

bool JsonReader::ISJsonRenderSettings() const {
    using namespace std::literals;
    return document_.GetRoot().AsMap().count("render_settings"s) != 0;
}

}  // namespace transport_catalogue::io
