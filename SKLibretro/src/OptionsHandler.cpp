#include "OptionsHandler.hpp"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

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

    variable->value = m_variables[variable->key].c_str();
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
    if (!variable || !variable->key)
        return true;

    m_variables[variable->key] = variable->value;

    return true;
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
}
