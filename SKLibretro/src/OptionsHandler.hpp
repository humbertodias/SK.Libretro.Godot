#pragma once

#include <string>
#include <unordered_map>

#include <libretro.h>

namespace SK
{
class OptionsHandler
{
public:
    bool GetCoreOptionsVersion(uint32_t* version);
    bool GetVariable(retro_variable* variable);
    bool GetVariableUpdate(bool* variable_update);
    bool SetVariable(const retro_variable* variable);
    bool SetVariables(const retro_variable* variables);
    bool SetVariableUpdate(bool update);
    bool SetCoreOptions(const retro_core_option_definition* definitions);
    bool SetCoreOptions(const retro_core_options_intl* definitions);
    bool SetCoreOptionsV2(const retro_core_options_v2* options);
    bool SetCoreOptionsV2Intl(const retro_core_options_v2_intl* options);
    bool SetCoreOptionsUpdateDisplayCallback(const retro_core_options_update_display_callback* callback);

private:
    static const uint32_t SUPPORTED_CORE_OPTIONS_VERSION = 2;

    std::unordered_map<std::string, std::string> m_variables = {};
    bool m_variable_update = false;
    retro_core_options_update_display_callback_t m_core_options_update_display_callback = nullptr;

    void SerializeToFile();
    void DeserializeFromFile();
};
}
