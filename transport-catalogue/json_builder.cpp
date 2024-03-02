#include "json_builder.h"

namespace json {

/**
* Class Builder
*/

Builder::~Builder() {
    while (!nodes_stack_.empty()) {
        delete nodes_stack_.back();
        nodes_stack_.pop_back();
    }
}

Builder::DictItemContext Builder::StartDict() {
    StartContainer(Dict{});
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    using namespace std::literals;
    // Если нет активного словаря
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Dict is not open"s);
    }
    Node::Value value = std::move(nodes_stack_.back()->GetValue());
    nodes_stack_.pop_back();
    AddValue(std::move(value));
    return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
    StartContainer(Array{});
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    using namespace std::literals;
    // Если нет активного Array
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Array is not open"s);
    }
    Node::Value value = std::move(nodes_stack_.back()->GetValue());
    nodes_stack_.pop_back();
    AddValue(std::move(value));
    return *this;
}


Builder::DictKeyItemContext Builder::Key(std::string key) {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    }

    // Может быть только ключем элемента словаря
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key can only be used as a key Dict"s);
    }

    nodes_stack_.emplace_back(new Node{ std::move(key) });
    
    return DictKeyItemContext(*this);
}

Builder::BaseItemContext Builder::Value(Node::Value value) {
    AddValue(std::move(value));
    return BaseItemContext(*this);
}

Node Builder::Build() {
    using namespace std::literals;
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Object has not been built yet"s);
    }
    else if (root_.IsNull()) {
        throw std::logic_error("Object is empty "s);
    }
    return root_;
}

void Builder::StartContainer(Node::Value value) {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    }

    // Если вызывается вместо ключа словаря
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Object can't be a key dict"s);
    }

    // Может быть первым элементом, элеменом массива и значением словоря
    nodes_stack_.emplace_back(new Node{ std::move(value) });
}

void Builder::AddValue(Node::Value value) {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    }

    // Может быть единственным элементом, элеменом массива и значением словоря

    // Value единственный элемент JSON
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
        return;
    }

    // Value не может быть ключем Dict
    if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Value can't be a key dict"s);
    }

    if (nodes_stack_.back()->IsString()) { // Значение элемента словоря
        Node::Value key = std::move(nodes_stack_.back()->GetValue());
        nodes_stack_.pop_back();
        // Ключ не должен повторяться
        if (nodes_stack_.back()->AsDict().count(std::get<std::string>(key)) > 0) {
            throw std::logic_error("Key already exist"s);
        }
        std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(std::get<std::string>(key))].GetValue() = std::move(value);
        return;
    }

    // Value элемент Array
    std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
}

/**
* Class BaseItemContext
*/

Builder::BaseItemContext::BaseItemContext(Builder& builder)
    : builder_(builder)
{
}

Builder::DictItemContext Builder::BaseItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::BaseItemContext Builder::BaseItemContext::EndDict() {
    return builder_.EndDict();
}

Builder::ArrayItemContext Builder::BaseItemContext::StartArray() {
    return builder_.StartArray();
}

Builder::BaseItemContext Builder::BaseItemContext::EndArray() {
    return builder_.EndArray();
}

Builder::DictKeyItemContext Builder::BaseItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder::BaseItemContext Builder::BaseItemContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

json::Node Builder::BaseItemContext::Build() {
    return builder_.Build();
}

/**
* Class DictItemContext : public BaseItemContext
*/

Builder::DictItemContext::DictItemContext(Builder& builder)
    : Builder::BaseItemContext(builder) // Родительский конструктор
{
}

Builder::DictKeyItemContext Builder::DictItemContext::Key(std::string key) {
    return Builder::BaseItemContext::Key(std::move(key));
}

/**
* Class ArrayItemContext : public BaseItemContext
*/

Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
    : Builder::BaseItemContext(builder) // Родительский конструктор
{
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return Builder::BaseItemContext::StartDict();
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return Builder::BaseItemContext::StartArray();
}

Builder::ArrayValueItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return Builder::BaseItemContext::Value(std::move(value));
}

/**
* Class DictKeyItemContext : public BaseItemContext
*/

Builder::DictKeyItemContext::DictKeyItemContext(Builder& builder)
    : Builder::BaseItemContext(builder) // Родительский конструктор
{
}

Builder::DictItemContext Builder::DictKeyItemContext::StartDict() {
    return Builder::BaseItemContext::StartDict();
}

Builder::ArrayItemContext Builder::DictKeyItemContext::StartArray() {
    return Builder::BaseItemContext::StartArray();
}

Builder::DictValueItemContext Builder::DictKeyItemContext::Value(Node::Value value) {
    return Builder::BaseItemContext::Value(value); // Родительский конструктор копирования
    // у родительскогго класса нет конструктора перемещения, std::move бессмысленный

    /* 
        * Можно уточнить?
        * Я расчитывал на дефолтный конструктор перемещения родительского класса, по идее если его явно не удалили,
        * он сгенерируется компилятором? Что подтверждает отсутствие ошибки компиляции. 
        * Правильно понимаю?
        * 
        * Другой момент про std::move(). Тут согласен не имеет смысла, в виду того, что в объекте хранится только ссылка на объект Builder.
        * Заменил Родительский конструктор перемещения на конструктор копирования.
        * 
        * Спасибо :)
        */
}

/**
* Class DictValueItemContext : public BaseItemContext
*/

Builder::DictValueItemContext::DictValueItemContext(Builder& builder)
    : Builder::BaseItemContext(builder) // Родительский конструктор
{
}

Builder::DictValueItemContext::DictValueItemContext(BaseItemContext base_context)
    : Builder::BaseItemContext(base_context) // Родительский конструктор копирования
{
}

Builder::DictKeyItemContext Builder::DictValueItemContext::Key(std::string key) {
    return Builder::BaseItemContext::Key(std::move(key));
}

/**
* Class ArrayValueItemContext : public BaseItemContext
*/

Builder::ArrayValueItemContext::ArrayValueItemContext(Builder& builder)
    : Builder::BaseItemContext(builder) // Родительский конструктор
{
}

Builder::ArrayValueItemContext::ArrayValueItemContext(BaseItemContext base_context)
    : Builder::BaseItemContext(std::move(base_context)) // Родительский конструктор копирования
{
}

Builder::DictItemContext Builder::ArrayValueItemContext::StartDict() {
    return Builder::BaseItemContext::StartDict();
}

Builder::ArrayItemContext Builder::ArrayValueItemContext::StartArray() {
    return Builder::BaseItemContext::StartArray();
}

Builder::ArrayValueItemContext Builder::ArrayValueItemContext::Value(Node::Value value) {
    return Builder::BaseItemContext::Value(std::move(value));
}

} // namespace json