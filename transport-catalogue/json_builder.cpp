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

DictItemContext Builder::StartDict() {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };

    // Если вызывается вместо ключа словаря
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Dict can't be a key dict"s);
    };

    // Может быть первым элементом, элеменом массива и значением словоря
    nodes_stack_.emplace_back(new Node{ Dict{} });

    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };
    //При пустом стеке закрывать нечего
    if (nodes_stack_.empty()) {
        throw std::logic_error("Dict is not open"s);
    }
    // Если нет активного словаря
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Dict is not open"s);
    }

    // Может быть первым элементом, элеменом массива и значением словоря
    Node::Value dict = std::move(nodes_stack_.back()->GetValue()); // Может быть пустым контейнером
    nodes_stack_.pop_back();

    // Dict единственный элемент JSON
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(dict);
        return *this;
    }

    // Dict не может быть ключем другого Dict
    if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Dict can't be a key another dict"s);
    }

    // Dict значение элемента словаря
    if (nodes_stack_.back()->IsString()) { // Dict как значение элемента словаря
        Node::Value key = std::move(nodes_stack_.back()->GetValue());
        nodes_stack_.pop_back();
        // Ключ не должен повторяться ???
        if (nodes_stack_.back()->AsDict().count(std::get<std::string>(key)) > 0) {
            throw std::logic_error("Key already exist"s);
        }
        std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(std::get<std::string>(key))].GetValue() = std::move(dict);
        return *this;
    }

    // Dict элемент Array
    std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(dict));

    return *this;
}

ArrayItemContext Builder::StartArray() {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };

    // Если вызывается вместо ключа словаря
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Dict can't be a key another dict"s);
    };

    //Может быть первым элементом, элеменом массива и значением словоря
    nodes_stack_.emplace_back(new Node{ Array{} });

    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };
    //При пустом стеке закрывать нечего
    if (nodes_stack_.empty()) {
        throw std::logic_error("Array is not open"s);
    }
    // Если нет активного словаря
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Array is not open"s);
    }

    // Может быть первым элементом, элеменом массива и значением словоря
    Node::Value arr = std::move(nodes_stack_.back()->GetValue()); // Может быть пустым контейнером
    nodes_stack_.pop_back();

    // Array единственный элемент JSON
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(arr);
        return *this;
    }

    // Array не может быть ключем Dict
    if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Array can't be a key dict"s);
    }

    // Array значение элемента словаря
    if (nodes_stack_.back()->IsString()) {
        Node::Value key = std::move(nodes_stack_.back()->GetValue());
        nodes_stack_.pop_back();
        // Ключ не должен повторяться ???
        if (nodes_stack_.back()->AsDict().count(std::get<std::string>(key)) > 0) {
            throw std::logic_error("Key already exist"s);
        }
        std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(std::get<std::string>(key))].GetValue() = std::move(arr);
        return *this;
    }

    // Dict элемент Array
    std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(arr));

    return *this;
}


DictKeyItemContext Builder::Key(std::string key) {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };

    // Может быть только ключем элемента словаря
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key can only be used as a key Dict"s);
    }

    nodes_stack_.emplace_back(new Node{ key });

    return DictKeyItemContext(*this);
}

BaseItemContext Builder::Value(Node::Value value) {
    using namespace std::literals;
    // Если объект уже построен
    if (!root_.IsNull()) {
        throw std::logic_error("Object has already been built"s);
    };

    // Может быть единственным элементом, элеменом массива и значением словоря

    // Value единственный элемент JSON
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
        return *this;
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
        return *this;
    }

    // Value элемент Array
    std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
    return BaseItemContext(*this);
}

Node Builder::Build() {
    using namespace std::literals;
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Object has not been built yet"s);
    }
    else if (root_.IsNull()) {
        throw std::logic_error("Object is empty"s);
    }
    return root_;
}

/**
* Class BaseItemContext
*/

BaseItemContext::BaseItemContext(Builder& builder)
    : builder_(builder)
{
}

DictItemContext BaseItemContext::StartDict() {
    return builder_.StartDict();
}

BaseItemContext BaseItemContext::EndDict() {
    return builder_.EndDict();
}

ArrayItemContext BaseItemContext::StartArray() {
    return builder_.StartArray();
}

BaseItemContext BaseItemContext::EndArray() {
    return builder_.EndArray();
}

DictKeyItemContext BaseItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

BaseItemContext BaseItemContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

json::Node BaseItemContext::Build() {
    return builder_.Build();
}

/**
* Class DictItemContext : public BaseItemContext
*/

DictItemContext::DictItemContext(Builder& builder)
    : BaseItemContext(builder) // Родительский конструктор
{
}

DictKeyItemContext DictItemContext::Key(std::string key) {
    return BaseItemContext::Key(std::move(key));
}

/**
* Class ArrayItemContext : public BaseItemContext
*/

ArrayItemContext::ArrayItemContext(Builder& builder)
    : BaseItemContext(builder) // Родительский конструктор
{
}

DictItemContext ArrayItemContext::StartDict() {
    return BaseItemContext::StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return BaseItemContext::StartArray();
}

ArrayValueItemContext ArrayItemContext::Value(Node::Value value) {
    return BaseItemContext::Value(std::move(value));
}

/**
* Class DictKeyItemContext : public BaseItemContext
*/

DictKeyItemContext::DictKeyItemContext(Builder& builder)
    : BaseItemContext(builder) // Родительский конструктор
{
}

DictItemContext DictKeyItemContext::StartDict() {
    return BaseItemContext::StartDict();
}

ArrayItemContext DictKeyItemContext::StartArray() {
    return BaseItemContext::StartArray();
}

DictValueItemContext DictKeyItemContext::Value(Node::Value value) {
    return BaseItemContext::Value(std::move(value));
}

/**
* Class DictValueItemContext : public BaseItemContext
*/

DictValueItemContext::DictValueItemContext(Builder& builder)
    : BaseItemContext(builder) // Родительский конструктор
{
}

DictValueItemContext::DictValueItemContext(BaseItemContext base_context)
    : BaseItemContext(std::move(base_context)) // Родительский конструктор перемещения
{
}

// Hазрешенные методы
DictKeyItemContext DictValueItemContext::Key(std::string key) {
    return BaseItemContext::Key(std::move(key));
}

/**
* Class ArrayValueItemContext : public BaseItemContext
*/

ArrayValueItemContext::ArrayValueItemContext(Builder& builder)
    : BaseItemContext(builder) // Родительский конструктор
{
}

ArrayValueItemContext::ArrayValueItemContext(BaseItemContext base_context)
    : BaseItemContext(std::move(base_context)) // Родительский конструктор перемещения
{
}

DictItemContext ArrayValueItemContext::StartDict() {
    return BaseItemContext::StartDict();
}

ArrayItemContext ArrayValueItemContext::StartArray() {
    return BaseItemContext::StartArray();
}

ArrayValueItemContext ArrayValueItemContext::Value(Node::Value value) {
    return BaseItemContext::Value(std::move(value));
}

} // namespace json