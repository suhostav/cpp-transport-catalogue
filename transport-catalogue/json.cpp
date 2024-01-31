#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

using Number = std::variant<int, double>;

void SkipWhiteSymbols(istream& input){
    char c;
    while(input >> c){
        if(isspace(c)){
            continue;
        } else {
            input.putback(c);
            break;
        }
    }
}

std::string LoadWord(istream& input){
    SkipWhiteSymbols(input);
    std::string res{};
    char c;
    while(input >> c){
        if(isalpha(c)){
            res += c;
        } else {
            input.putback(c);
            break;
        }
    }
    return res;
}

Number LoadNumber(std::istream& input) {
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
    } else {
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
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadArray(istream& input) {
    Array result;
    char c = '\0';
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if(c != ']'){
        throw ParsingError("LoadArray: Parsing error. No close bracket "s + "]"s);
    }

    return Node(move(result));
}

std::string LoadString(std::istream& input) {
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
        } else if (ch == '\\') {
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
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadDict(istream& input) {
    Dict result;
    SkipWhiteSymbols(input);
    char c = '\0';
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        SkipWhiteSymbols(input);
        input >> c;
        if(c != ':'){
            throw ParsingError("LoadDict. No \':\' sign after key.");
        }
        result.insert({move(key), LoadNode(input)});
    }
    if(c != '}'){
        throw ParsingError("LoadDict: parsing error. No close bracket }");
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {

    SkipWhiteSymbols(input);
    if(input.eof()){
        return Node();
    }
    
    char c;
    input >> c;
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node(LoadString(input));
    } else if (c == '+' || c == '-' || isdigit(c)) {
        input.putback(c);
        Number num = LoadNumber(input);
        if(num.index() == 0){
            return Node(std::get<0>(num));
        }
        return Node(std::get<1>(num));
    } else if( isalpha(c)) {   
        input.putback(c);
        auto s = LoadWord(input);
        if(s == "null"s){
            return Node();
        } else if(s == "true"s){
            return Node(true);
        } else if(s == "false"s){
            return Node(false);
        }
        throw json::ParsingError("LoadNode: Parsing error. Unknown word "s + s);
    } else {
        throw json::ParsingError("LoadNode: Parsing error");
    }
}

}  // namespace


// --------- Node -------------

Node::Node(int value)
    : value_(value) {
}

Node::Node(double value)
    : value_(value) {
}

Node::Node(bool value)
    : value_(value) {
}

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(string value)
    : value_(move(value)) {
}

bool Node::IsInt() const{
    return holds_alternative<int>(value_);
}

bool Node::IsDouble() const{
    return (holds_alternative<int>(value_) || holds_alternative<double>(value_));
}

bool Node::IsPureDouble() const{
    return holds_alternative<double>(value_);
}

bool Node::IsBool() const{
    return holds_alternative<bool>(value_);
}

bool Node::IsString() const{
    return holds_alternative<std::string>(value_);
}

bool Node::IsNull() const{
    return holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const{
    return holds_alternative<Array>(value_);
}

bool Node::IsMap() const{
    return holds_alternative<Dict>(value_);
}

int Node::AsInt() const {
    if(IsInt()){
        return std::get<int>(value_);
    }
    throw std::logic_error("Invalid Node value type"s);
}

double Node::AsDouble() const {
    if(IsDouble()){
        if(IsPureDouble()){
            return std::get<double>(value_);
        }
        return static_cast<double>(std::get<int>(value_));
    }
    throw std::logic_error("Invalid Node value type"s);
}

bool Node::AsBool() const {
    if(IsBool()){
        return std::get<bool>(value_);
    }
    throw std::logic_error("Invalid Node value type"s);
}

const string& Node::AsString() const {
    if(IsString()){
        return std::get<std::string>(value_);
    }
    throw std::logic_error("Invalid Node value type"s);
}

const Array& Node::AsArray() const {
    if(IsArray()){
        return std::get<Array>(value_);
    }
    throw std::logic_error("Invalid Node value type"s);
}

const Dict& Node::AsMap() const {
    if(IsMap()){
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Invalid Node value type"s);
}

bool Node::operator ==(const Node& other) const{
    if(IsNull() && other.IsNull()){
        return true;
    }
    if(IsInt() && other.IsInt()){
        return AsInt() == other.AsInt();
    } else if(IsPureDouble() && other.IsPureDouble()){
        return AsDouble() == other.AsDouble();
    } else if(IsBool() && other.IsBool()){
        return AsBool() == other.AsBool();
    } else if(IsString() && other.IsString()){
        return AsString() == other.AsString();
    } else if(IsArray() && other.IsArray()){
        return AsArray() == other.AsArray();
    } else if(IsMap() && other.IsMap()){
        return AsMap() == other.AsMap();
    }
    return false;
}

bool Node::operator !=(const Node& other) const{
    return !(operator ==(other));
}

// ----------- Document ----------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, const PrintContext& context);

void Print(const Document& doc, std::ostream& output) {
    output << std::boolalpha;
    PrintNode(doc.GetRoot(), {output});
}

bool Document::operator ==(const Document& other) const{
    return GetRoot() == other.GetRoot();
}

bool Document::operator !=(const Document& other) const{
    return GetRoot() != other.GetRoot();
}
// ---------- end Document ---------------------------

void PrintValue(int value, const PrintContext& context) {
    context.out << value;
}

void PrintValue(double value, const PrintContext& context) {
    context.out << value;
}

void PrintValue(std::nullptr_t, const PrintContext& context) {
    context.out << "null"sv;
}

void PrintValue(bool value, const PrintContext& context) {
    if(value){
        context.out << "true"sv;
    } else {
        context.out << "false"sv;
    }
}

void PrintValue(const std::string& value, const PrintContext& context){
    context.out << "\""sv;
    for(auto c : value){
        switch (c) {
        case '\\':
            context.out << "\\\\"sv;
            break;
        case '"':
            context.out << "\\\""sv;
            break;
        case '\r':
            context.out << "\\r"sv;
            break;
        case '\n':
            context.out << "\\n"sv;
            break;
        case '\t':
            context.out << "\\t"sv;
            break;
        default:
            context.out << c;
        }
    }
    context.out << "\""sv;
}

void PrintValue(const Array& value, const PrintContext& context){
    bool hasData = value.size() > 0;
    if(!hasData){
        context.out << "[]";
        return;
    }
    context.PrintIndent();
    context.out << "[\n";
    auto el_ctx = context.Indented();
    bool first = true;
    for(auto elem : value){
        if(first){
            first = false;
            el_ctx.PrintIndent();
        } else {
            context.out << ", "sv;
            el_ctx.out << '\n';
            el_ctx.PrintIndent();
        }
        PrintNode(elem, el_ctx);
    }
    context.out << '\n';
    context.PrintIndent();
    context.out << ']';
}

void PrintValue(const Dict& value, const PrintContext& context){
    bool hasData = value.size() > 0;
    context.PrintIndent();
    if(!hasData){
        context.out << "{}";
        return;
    }
    context.out << "{\n"sv;
    auto el_ctx = context.Indented();
    bool first = true;
    for(auto [k, v] : value){
        if(first){
            first = false;
            el_ctx.PrintIndent();
        } else {
            el_ctx.out << ", "sv;
            el_ctx.out << '\n';
            el_ctx.PrintIndent();
        }
        el_ctx.out << '"' << k << "\": ";
        PrintNode(v, el_ctx);
    }
    context.out << '\n';
    context.PrintIndent();
    context.out << '}';
}

void PrintNode(const Node& node, const PrintContext& context) {
    std::visit(
        [&context](const auto& value){ PrintValue(value, context); },
        node.GetValue());
} 

}  // namespace json