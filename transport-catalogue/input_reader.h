#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

using std::vector, std::string, std::string_view;

namespace ctlg::input {

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    string command;      // Название команды
    string id;           // id маршрута или остановки
    geo::Coordinates coords;    //соординаты, если command == "Stop"
    string description;  // Параметры команды
    vector<std::string_view> fields;    //прочие параметры, разделенные запятыми
};

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(ctlg::TransportCatalogue& catalogue) const;

private:
    vector<CommandDescription> commands_;

    void SetDistances(CommandDescription& command, [[maybe_unused]] TransportCatalogue& catalogue) const;
};

}   //ctlg::input

string_view Trim(string_view string);
vector<string_view> Split(string_view string, char delim);
size_t GetMeters(string_view data);
string_view GetWord(string_view line, string_view& word);