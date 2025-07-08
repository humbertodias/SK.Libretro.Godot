#include "OptionsHandler.hpp"

#include <fstream>
#include <filesystem>

#include "Wrapper.hpp"

using namespace godot;

namespace SK
{
bool OptionsHandler::GetCoreOptionsVersion(uint32_t* version)
{
    if (!version)
        return true;

    *version = OptionsHandler::SUPPORTED_CORE_OPTIONS_VERSION;
    return true;
}

bool OptionsHandler::GetVariable(retro_variable* variable)
{
    if (!variable || !variable->key)
        return true;

    std::string key(variable->key);
    if (!m_variables.contains(key))
        return true;

    std::string value = variable->value ? variable->value : "";

    variable->value = m_variables[key].c_str();
    return true;
}

bool OptionsHandler::GetVariableUpdate(bool* variable_update)
{
    if (!variable_update)
        return true;

    *variable_update = m_variable_update;
    return SetVariableUpdate(false);
}

bool OptionsHandler::SetVariable(const retro_variable* variable)
{
    if (!variable)
        return true;

    if (!variable->key
     || strlen(variable->key) == 0
     || !variable->value
     || strlen(variable->value) == 0
     || !m_variables.contains(variable->key))
        return false;

    m_variables[variable->key] = variable->value;

    return SetVariableUpdate(true);
}

bool OptionsHandler::SetVariables(const retro_variable* variables)
{
    m_variables.clear();

    if (!variables)
        return true;

    for (int i = 0; variables[i].key; ++i)
    {
        auto values = String(variables[i].value).split(";")[1].split("|");
        if (values.size() < 1)
            continue;

        auto value = values[0].trim_prefix(" ");
        m_variables.emplace(variables[i].key, value.utf8().get_data());
    }

    return true;
}

bool OptionsHandler::SetVariableUpdate(bool update)
{
    m_variable_update = update;
    return true;
}

bool OptionsHandler::SetCoreOptions(const retro_core_option_definition* definitions)
{
    m_variables.clear();

    if (!definitions)
        return true;

    for (auto definition = definitions; definition->key; ++definition)
        if (definition->key && definition->default_value)
            m_variables[definition->key] = definition->default_value;

    DeserializeFromFile();

    return true;
}

bool OptionsHandler::SetCoreOptions(const retro_core_options_intl* options_intl)
{
    return SetCoreOptions(options_intl->local ? options_intl->local : options_intl->us);
}

bool OptionsHandler::SetCoreOptionsV2(const retro_core_options_v2* options)
{
    m_variables.clear();

    if (!options)
        return true;

    for (auto definition = options->definitions; definition->key; ++definition)
        if (definition->key && definition->default_value)
            m_variables[definition->key] = definition->default_value;

    DeserializeFromFile();

    return true;
}

bool OptionsHandler::SetCoreOptionsV2Intl(const retro_core_options_v2_intl* options)
{
    return SetCoreOptionsV2(options->local ? options->local : options->us);
}

bool OptionsHandler::SetCoreOptionsUpdateDisplayCallback(const retro_core_options_update_display_callback* update_display_callback)
{
    m_core_options_update_display_callback = nullptr;

    if (update_display_callback)
        m_core_options_update_display_callback = update_display_callback->callback;

    return true;
}

void OptionsHandler::SerializeToFile()
{
    const auto& root_directory = Wrapper::GetInstance()->GetRootDirectory();
    const auto& core_name = Wrapper::GetInstance()->m_core->GetName();
    std::filesystem::path file_path = std::filesystem::path(root_directory) / "core_options" / (core_name + ".opt");
    
    if (!std::filesystem::exists(file_path))
    {
        std::error_code ec;
        if (!std::filesystem::create_directories(file_path.parent_path(), ec))
        {
            LogError(ec.message());
            return;
        }
    }

    std::ofstream file(file_path, std::ofstream::trunc);
    if (!file.is_open())
    {
        LogError("Failed to open options file for writing: " + file_path.string());
        return;
    }

    for (const auto& [key, value] : m_variables)
        file << key << " = \"" << value << "\"\n";
}

void OptionsHandler::DeserializeFromFile()
{
    const auto& root_directory = Wrapper::GetInstance()->GetRootDirectory();
    const auto& core_name = Wrapper::GetInstance()->m_core->GetName();
    std::filesystem::path file_path = std::filesystem::path(root_directory) / "core_options" / (core_name + ".opt");

    if (!std::filesystem::exists(file_path))
    {
        SerializeToFile();
        return;
    }

    std::ifstream file(file_path);
    if (!file.is_open())
    {
        LogError("Failed to open options file: " + file_path.string());
        return;
    }

    std::string line_str;
    while (std::getline(file, line_str))
    {
        std::string_view line(line_str);

        auto eq_pos = line.find('=');
        if (eq_pos == std::string_view::npos)
            continue;

        auto key = line.substr(0, eq_pos);
        auto key_begin = key.find_first_not_of(" \t");
        auto key_end = key.find_last_not_of(" \t");
        if (key_begin == std::string_view::npos || key_end == std::string_view::npos)
            continue;
        key = key.substr(key_begin, key_end - key_begin + 1);
        if (key.empty())
            continue;

        auto value = line.substr(eq_pos + 1);
        auto value_begin = value.find_first_not_of(" \t");
        auto value_end = value.find_last_not_of(" \t");
        if (value_begin == std::string_view::npos || value_end == std::string_view::npos)
            continue;
        value = value.substr(value_begin, value_end - value_begin + 1);
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
            value = value.substr(1, value.size() - 2);

        if (m_variables.contains(std::string(key)))
            m_variables[std::string(key)] = std::string(value);
    }
}
}
