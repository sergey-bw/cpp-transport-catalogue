#include "map_renderer.h"

namespace transport_catalogue::renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(RenderSettings settings)
    : render_settings_(std::move(settings))
{
}

void MapRenderer::SetRenderSettings(RenderSettings settings) {
    render_settings_ = std::move(settings);
}

svg::Document MapRenderer::GetSvgDocument(std::map<std::string_view, const Bus*> busname_to_bus) const {
    std::vector<geo::Coordinates> coordinates;
    std::map<std::string_view, const Stop*> stopname_to_stop;
    svg::Document result;

    // Собираем данные
    for (const auto& [bus_name, bus_ptr] : busname_to_bus) {
        if (bus_ptr->stops.size() == 0) continue;
        for (const auto& stop : bus_ptr->stops) {
            coordinates.push_back(stop->coordinates);
            stopname_to_stop[stop->id] = stop;
        }
    }

    // Рисуем слои
    SphereProjector sp(coordinates.begin(), coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetBusLines(busname_to_bus, sp)) {
        result.Add(line);
    }
    for (const auto& text : GetBusLabels(busname_to_bus, sp)) {
        result.Add(text);
    }
    for (const auto& circle : GetStopCircle(stopname_to_stop, sp)) {
        result.Add(circle);
    }
    for (const auto& text : GetStopLabels(stopname_to_stop, sp)) {
        result.Add(text);
    }

    return result;
}

std::vector<svg::Polyline> MapRenderer::GetBusLines(std::map<std::string_view, const Bus*> busname_to_bus, const SphereProjector& sp) const {
    using namespace std::literals;
    std::vector<svg::Polyline> result;
    uint32_t color_num = 0;

    for (auto& [bus_name, bus_ptr] : busname_to_bus) {
        if (bus_ptr->stops.size() == 0) {
            continue;
        }

        svg::Polyline bus_line;
        for (auto stop : bus_ptr->stops) {
            bus_line.AddPoint(sp(stop->coordinates));
        }
        bus_line.SetFillColor("none"s)
            .SetStrokeColor(render_settings_.color_palette[color_num])
            .SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        result.push_back(std::move(bus_line));

        ++color_num;
        if (color_num >= render_settings_.color_palette.size()) {
            color_num = 0;
        }
    }
    return result;
}

std::vector<svg::Text> MapRenderer::GetBusLabels(std::map<std::string_view, const Bus*> busname_to_bus, const SphereProjector& sp) const {
    using namespace std::literals;
    std::vector<svg::Text> result;
    uint32_t color_num = 0;

    for (auto& [bus_name, bus_ptr] : busname_to_bus) {
        if (bus_ptr->stops.size() == 0) {
            continue;
        }

        // Подложка
        result.push_back(
            svg::Text()
                .SetPosition(sp(bus_ptr->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(bus_ptr->id)
            );     

        // Верхний слой (Название маршрута)
        result.push_back(
            svg::Text()
                .SetPosition(sp(bus_ptr->stops[0]->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFillColor(render_settings_.color_palette[color_num])
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus_ptr->id)
            );

        // Eсли маршрут некольцевой
        if (!bus_ptr->is_roundtrip && bus_ptr->stops[0] != bus_ptr->last_stop) {
            // Подложка
            result.push_back(
                svg::Text()
                .SetPosition(sp(bus_ptr->last_stop->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(bus_ptr->id)
            );

            // Верхний слой (Название маршрута)
            result.push_back(
                svg::Text()
                .SetPosition(sp(bus_ptr->last_stop->coordinates))
                .SetOffset(render_settings_.bus_label_offset)
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFillColor(render_settings_.color_palette[color_num])
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus_ptr->id)
            );
        }

        ++color_num;
        if (color_num >= render_settings_.color_palette.size()) {
            color_num = 0;
        }
    }

    return result;
}

std::vector<svg::Circle> MapRenderer::GetStopCircle(std::map<std::string_view, const Stop*> stopname_to_stop, const SphereProjector& sp) const {
    using namespace std::literals;
    std::vector<svg::Circle> result;
    for (auto& [stop_name, stop_ptr] : stopname_to_stop) {
        result.push_back(
            svg::Circle()
                .SetCenter(sp(stop_ptr->coordinates))
                .SetRadius(render_settings_.stop_radius)
                .SetFillColor("white"s)
        );
    }
    return result;
}

std::vector<svg::Text> MapRenderer::GetStopLabels(std::map<std::string_view, const Stop*> stopname_to_stop, const SphereProjector& sp) const {
    using namespace std::literals;
    std::vector<svg::Text> result;

    for (auto& [stop_name, stop_ptr] : stopname_to_stop) {
        // Подложка
        result.push_back(
            svg::Text()
                .SetPosition(sp(stop_ptr->coordinates))
                .SetOffset(render_settings_.stop_label_offset)
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(stop_ptr->id)
        );

        // Верхний слой (Название Остановки)
        result.push_back(
            svg::Text()
                .SetPosition(sp(stop_ptr->coordinates))
                .SetOffset(render_settings_.stop_label_offset)
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFillColor("black"s)
                .SetData(stop_ptr->id)
        );
    }

    return result;
}

} // namespace transport_catalogue::renderer