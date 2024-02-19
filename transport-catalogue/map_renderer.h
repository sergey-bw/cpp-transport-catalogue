#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdint>
#include <map>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport_catalogue::renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) 
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    // Ширина и высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
    double width = {};
    double height = {};
	
    // Отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
    double padding = {};

    // Толщина линий, которыми рисуются автобусные маршруты.Вещественное число в диапазоне от 0 до 100000.
    double line_width = {};
    // Радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
    double stop_radius = {};

    // Размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
    uint16_t bus_label_font_size = {};
    // Смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double.
    // Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
    svg::Point bus_label_offset = {};

    // Размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
    uint16_t stop_label_font_size = {};
    // Смещение названия остановки относительно её координат на карте. Массив из двух элементов типа double.
    // Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
    svg::Point stop_label_offset = {};

    // Цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
    svg::Color underlayer_color = {};
    // Толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>.
    // Вещественное число в диапазоне от 0 до 100000.
    double underlayer_width = {};

    // Цветовая палитра. Непустой массив
    std::vector<svg::Color> color_palette = {};
};

class MapRenderer {
public:

    MapRenderer() = default;

    MapRenderer(RenderSettings settings);

    void SetRenderSettings(RenderSettings settings);

    svg::Document GetSvgDocument(std::map<std::string_view, const Bus*> busname_to_bus) const;

private:
    RenderSettings render_settings_;

    std::vector<svg::Polyline> GetBusLines(std::map<std::string_view, const Bus*> busname_to_bus, const SphereProjector& sp) const;

    std::vector<svg::Text> GetBusLabels(std::map<std::string_view, const Bus*> busname_to_bus, const SphereProjector& sp) const;

    std::vector<svg::Circle> GetStopCircle(std::map<std::string_view, const Stop*> stopname_to_stop, const SphereProjector& sp) const;

    std::vector<svg::Text> GetStopLabels(std::map<std::string_view, const Stop*> stopname_to_stop, const SphereProjector& sp) const;
};

} // namespace transport_catalogue::renderer