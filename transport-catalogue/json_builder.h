#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder();
    Node Build();
    Builder& Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    
    void AssertNewObjectContext() const;
    void AddObject(Node::Value value, bool one_shot);
};

class Context {
public:
    Context(Builder& builder): builder_(builder){  }

protected:
    Builder& builder_;
};

class KeyContext;

class DictItemContext : public Context{
public:
    DictItemContext(Builder& builder);
    KeyContext Key(std::string key);
    Builder& EndDict();

};

class ArrayValueContext;

class ArrayItemContext : public Context{
public:
    ArrayItemContext(Builder& builder);
    ArrayValueContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext& StartArray();
    Builder& EndArray();

};

class KeyValueContext;

class KeyContext: public Context{
public:
    KeyContext(Builder& builder): Context(builder){}
    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};

class KeyValueContext : public Context{
public:
    KeyValueContext(Builder& builder);
    KeyContext Key(std::string key);
    Builder& EndDict();
};

class ArrayValueContext : public Context{
public:
    ArrayValueContext(Builder& builder);
    ArrayValueContext& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};
}  // namespace json