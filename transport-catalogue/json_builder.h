#pragma once

#include "json.h"

#include <string>
#include <optional>

namespace json {

class BaseItemContext;

class DictItemContext;
class ArrayItemContext;
class DictKeyItemContext;
class DictValueItemContext;
class ArrayValueItemContext;

class Builder {
public:

    Builder() = default;
    ~Builder();

    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;
    Builder(Builder&&) = delete;
    Builder& operator=(Builder&&) = delete;

    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    DictKeyItemContext Key(std::string key);
    BaseItemContext Value(Node::Value value);
    Node Build();

private:
    Node root_{ nullptr };

    /**
    * nodes_stack_<Node*> - Стек открытых элементов:
    *
    * Node<Dict> - Словарь
    * Node<Array> - Массив
    * Node<std::string> - Ключ словаря
    */
    std::vector<Node*> nodes_stack_;
};

class BaseItemContext {
public:
    BaseItemContext() = delete;
    BaseItemContext(Builder& builder);

    DictItemContext StartDict();
    BaseItemContext EndDict();
    ArrayItemContext StartArray();
    BaseItemContext EndArray();
    DictKeyItemContext Key(std::string key);
    BaseItemContext Value(Node::Value value);
    json::Node Build();

private:
    Builder& builder_;
};

class DictItemContext : public BaseItemContext {
public:
    DictItemContext(Builder& builder);

    // Разрешенные методы
    DictKeyItemContext Key(std::string key);
    // EndDict(); Наследуем от BaseItemContext

    // Запрещаем вызов
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseItemContext EndArray() = delete;
    BaseItemContext Value(Node::Value) = delete;
    json::Node Build() = delete;
};

class ArrayItemContext : public BaseItemContext {
public:
    ArrayItemContext(Builder& builder);

    // Разрешенные методы
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    ArrayValueItemContext Value(Node::Value value);
    // EndArray(); Наследуем от BaseItemContext

    // Запрещенные методы
    BaseItemContext EndDict() = delete;
    DictKeyItemContext Key(std::string) = delete;
    json::Node Build() = delete;
};

class DictKeyItemContext : public BaseItemContext {
public:
    DictKeyItemContext(Builder& builder);

    // Разрешенные методы
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    DictValueItemContext Value(Node::Value value);

    // Запрещенные методы
    BaseItemContext EndDict() = delete;
    BaseItemContext EndArray() = delete;
    DictKeyItemContext Key(std::string) = delete;
    json::Node Build() = delete;
};

class DictValueItemContext : public BaseItemContext {
public:
    DictValueItemContext(Builder& builder);
    DictValueItemContext(BaseItemContext base_context);

    // Разрешенные методы
    DictKeyItemContext Key(std::string key);
    //EndDict(); Наследуем от BaseItemContext

    // Запрещенные методы
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseItemContext EndArray() = delete;
    BaseItemContext Value(Node::Value) = delete;
    json::Node Build() = delete;
};

class ArrayValueItemContext : public BaseItemContext {
public:
    ArrayValueItemContext(Builder& builder);
    ArrayValueItemContext(BaseItemContext base_context);

    // Разрешенные методы
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    ArrayValueItemContext Value(Node::Value value);
    //EndArray();  Наследуем от BaseItemContext

    // Запрещенные методы
    BaseItemContext EndDict() = delete;
    DictKeyItemContext Key(std::string) = delete;
    json::Node Build() = delete;
};

} // namespace json