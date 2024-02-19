#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Ёта ошибка должна выбрасыватьс¤ при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    // Делаем доступными все конструкторы родительского класса variant
    using variant::variant;

    Node() = default;

    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;

    std::nullptr_t AsNull() const;
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const std::string& AsString() const;

    const Value& GetValue() const;

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);

bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json