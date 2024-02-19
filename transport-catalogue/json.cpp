#include "json.h"

#include <stdexcept>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
        };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
        };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
                return Node(std::stod(parsed_num));
            }
        }
        return Node(std::stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            std::cout << "sdf"sv << std::endl;
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input)
    {
        throw ParsingError("Array parsing error"s);
    }

    return Node(result);
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input).AsString();
        if (input >> c && c == ':') {
            result.insert({ key, LoadNode(input) });
        }
        else {
            throw ParsingError("Dist parsing error"s);
        }
    }
    if (!input)
    {
        throw ParsingError("Dist parsing error"s);
    }

    return Node(result);
}

Node LoadNull(istream& input) {
    std::string result;
    char c;
    while (true) {
        if (!(input >> c)) {
            break;
        }
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            input.putback(c);
            break;
        }
        result.push_back(c);

    }
    if (result == "null") {
        return Node{ nullptr };
    }
    throw ParsingError("Null parsing error");
}

Node LoadBool(istream& input) {
    std::string result;
    char c;
    while (true) {
        if (!(input >> c)) {
            break;
        }
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            input.putback(c);
            break;
        }
        result.push_back(c);

    }

    if (result == "true") {
        return Node{ true };
    }
    if (result == "false") {
        return Node{ false };
    }
    throw ParsingError("Bool parsing error");
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

//-----------Node-----------------

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

//Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    return std::holds_alternative<double>(*this)
        || std::holds_alternative<int>(*this);
}

//Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

nullptr_t Node::AsNull() const {
    if (!IsNull()) {
        throw std::logic_error("Type is not null");
    }
    return nullptr;
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Type is not int");
    }
    return std::get<int>(*this);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Type is not double"s);
    }
    if (IsPureDouble()) {
        return std::get<double>(*this);
    }
    return static_cast<double>(AsInt());
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Type is not bool");
    }
    return std::get<bool>(*this);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Type is not array");
    }
    return std::get<Array>(*this);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Type is not map");
    }
    return std::get<Dict>(*this);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Type is not string");
    }
    return std::get<string>(*this);
}

const Node::Value& Node::GetValue() const {
    return *this;
}

bool Node::operator==(const Node& rhs) const {
    return GetValue() == rhs.GetValue();
}

bool Node::operator!=(const Node& rhs) const {
    return !(*this == rhs);
}

//-----------Document-------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

//-----------PrintValue___________

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

void PrintString(const std::string& str, std::ostream& out) {
    out << '"';
    for (const auto& c : str) {
        switch (c) {
        case '\"':
            out << "\\\""sv;
            break;
        case '\\':
            out << "\\\\"sv;
            break;
        case '\n':
            out << "\\n"sv;
            break;
        case '\t':
            out << "\\t"sv;
            break;
        case '\r':
            out << "\\r"sv;
            break;
        default:
            out << c;
        }
    }
    out << '"';
}

void PrintValue(const std::nullptr_t /* value */, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(const int value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(const double value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(const bool value, const PrintContext& ctx) {
    ctx.out << std::boolalpha << value << std::noboolalpha;
}

void PrintValue(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    auto inner_ctx = ctx.Indented();

    bool is_first = true;
    for (const auto& item : array) {
        if (!is_first) {
            out << ",\n"sv;
        }
        is_first = false;
        inner_ctx.PrintIndent();
        PrintNode(item, inner_ctx);
    }

    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

void PrintValue(const Dict& dist, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    auto inner_ctx = ctx.Indented();

    bool is_first = true;
    for (const auto& [key, value] : dist) {
        if (!is_first) {
            out << ",\n"sv;
        }
        is_first = false;
        inner_ctx.PrintIndent();
        out << '"' << key << "\":"sv;
        PrintNode(value, inner_ctx);
    }

    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](auto value) {
            PrintValue(value, ctx);
        }, node.GetValue());
}

void Print(const Document& doc, std::ostream& out) {
    PrintNode(doc.GetRoot(), PrintContext{ out });
}

}  // namespace json