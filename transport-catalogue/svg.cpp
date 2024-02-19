#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap)
    {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}


std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

// Задаёт координаты центра окружности (атрибуты cx и cy)
Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}


// Задаёт радиус окружности (атрибут r)
Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;

    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);

    out << "/>"sv;
}

// ---------- Polyline ----------------

// Добавляет очередную вершину к ломаной линии
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const auto& point : points_) {
        if (!is_first) {
            out << ' ';
        }
        out << point.x << ',' << point.y;
        is_first = false;
    }
    out << "\"";

    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text --------------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

std::string Text::EscapingSpecialCharacters(std::string text) const {
    std::string result = ""s;
    for (const char& symbol : text) {
        if (special_characters_.count(symbol) == 1) {
            result += special_characters_.at(symbol);
        }
        else {
            result += symbol;
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\""sv;

    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }

    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);

    out << ">"sv;
    out << EscapingSpecialCharacters(data_);
    out << "</text>"sv;
}


// ---------- Document ----------------

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    RenderContext context_ = RenderContext(out);

    context_.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    context_.out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    context_.indent_step = 2;
    ++context_.indent;
    for (size_t i = 0; i < objects_.size(); ++i) {
        objects_[i]->Render(context_);
    }
    --context_.indent;

    context_.out << "</svg>"sv;
}

}  // namespace svg