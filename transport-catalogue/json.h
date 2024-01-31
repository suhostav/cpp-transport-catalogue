#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
struct PrintContext;
// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
   using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() {}
    Node(std::nullptr_t): Node(){}
    Node(int value);
    Node(double value);
    Node(bool value);
    Node(Array array);
    Node(Dict map);
    Node(std::string value);

    template <typename T>
    bool IsType(){
        return (std::holds_alternative<T>(value_)) ? true : false;
    }
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool operator ==(const Node& other) const;
    bool operator !=(const Node& other) const;

    const Value& GetValue() const { return value_; }
private:
    Value value_;
};

class Document {
public:
    Document(Node root);

    const Node& GetRoot() const;
    bool operator ==(const Document& other) const;
    bool operator !=(const Document& other) const;
private:
    Node root_;
};

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 2;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);

}  // namespace json