#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

Builder::Builder()
    : root_()
    , nodes_stack_{&root_}
{
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }
    return std::move(root_);
}

Builder& Builder::Key(std::string key) {
    Node::Value& host_value = GetCurrentValue();
    
    if (!std::holds_alternative<Dict>(host_value)) {
        throw std::logic_error("Key() outside a dict"s);
    }
    
    nodes_stack_.push_back(
        &std::get<Dict>(host_value)[std::move(key)]
    );
    return *this;
}

Builder& Builder::Value(Node::Value value) {
    AddObject(std::move(value), /* one_shot */ true);
    return *this;
}

DictItemContext Builder::StartDict() {
    AddObject(Dict{}, /* one_shot */ false);
    DictItemContext context {*this};
    return context;
}

ArrayItemContext Builder::StartArray() {
    AddObject(Array{}, /* one_shot */ false);
    ArrayItemContext context{*this};
    return context;
}

Builder& Builder::EndDict() {
    if (!std::holds_alternative<Dict>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside a dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (!std::holds_alternative<Array>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside an array"s);
    }
    nodes_stack_.pop_back();
    return *this;
}
    
Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }
    return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::GetCurrentValue() const {
    return const_cast<Builder*>(this)->GetCurrentValue();
}

void Builder::AssertNewObjectContext() const {
    if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
        throw std::logic_error("New object in wrong context"s);
    }
}

void Builder::AddObject(Node::Value value, bool one_shot) {
    Node::Value& host_value = GetCurrentValue();
    if (std::holds_alternative<Array>(host_value)) {
        Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
        if (!one_shot) {
            nodes_stack_.push_back(&node);
        }
    } else {
        AssertNewObjectContext();
        host_value = std::move(value);
        if (one_shot) {
            nodes_stack_.pop_back();
        }
    }
}

//------------ helper types -----------------------

DictItemContext::DictItemContext(Builder& builder): Context(builder){  }

    KeyContext DictItemContext::Key(std::string key){
        builder_.Key(key);
        KeyContext context{builder_};
        return context;
    }

    Builder& DictItemContext::EndDict(){
        return builder_.EndDict();
    }

    ArrayItemContext::ArrayItemContext(Builder& builder): Context(builder){  }

    ArrayValueContext ArrayItemContext::Value(Node::Value value){
        builder_.Value(value);
        return ArrayValueContext{builder_};
    }

    DictItemContext ArrayItemContext::StartDict(){
        builder_.StartDict();
        return DictItemContext{builder_};
    }
    ArrayItemContext& ArrayItemContext::StartArray(){
        builder_.StartArray();
        return *this;
    }

    Builder& ArrayItemContext::EndArray(){
        return builder_.EndArray();
    }

    DictItemContext KeyContext::Value(Node::Value value){
        builder_.Value(value);
        return DictItemContext{builder_};
    }

    DictItemContext KeyContext::StartDict(){
        return builder_.StartDict();
    }
    ArrayItemContext KeyContext::StartArray(){
        return builder_.StartArray();
    }

    KeyValueContext::KeyValueContext(Builder& builder): Context(builder){}

    KeyContext KeyValueContext::Key(std::string key){
        builder_.Key(key);
        return KeyContext{builder_};
    }
    Builder& KeyValueContext::EndDict(){
        return builder_.EndDict();
    }

    ArrayValueContext::ArrayValueContext(Builder& builder): Context(builder){}
    ArrayValueContext& ArrayValueContext::Value(Node::Value value){
        builder_.Value(value);
        return *this;
    }
    DictItemContext ArrayValueContext::StartDict(){
        builder_.StartDict();
        return DictItemContext{builder_};
    }

    ArrayItemContext ArrayValueContext::StartArray(){
        builder_.StartArray();
        return ArrayItemContext{builder_};
    }

    Builder& ArrayValueContext::EndArray(){
        return builder_.EndArray();
    }

}  // namespace json