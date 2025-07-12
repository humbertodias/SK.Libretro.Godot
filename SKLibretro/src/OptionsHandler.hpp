#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <libretro.h>

namespace SK
{
struct OptionCategory
{
    std::string desc;
    std::string info;
};

struct OptionValue
{
    std::string value;
    std::string label;
};

struct OptionDefinition
{
    std::string desc;
    std::string desc_categorized;
    std::string info;
    std::string info_categorized;
    std::string category_key;
    std::vector<OptionValue> values;
    std::string default_value;
};

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

    const std::unordered_map<std::string, OptionCategory>& GetCategories() const { return m_categories; }
    const std::unordered_map<std::string, OptionDefinition>& GetDefinitions() const { return m_definitions; }
    const std::unordered_map<std::string, std::string>& GetValues() const { return m_variables; }
    void SetVariable(const std::string& key, const std::string& value);

private:
    static const uint32_t SUPPORTED_CORE_OPTIONS_VERSION = 2;

    std::unordered_map<std::string, OptionCategory> m_categories = {};
    std::unordered_map<std::string, OptionDefinition> m_definitions = {};

    std::unordered_map<std::string, std::string> m_variables = {};
    bool m_variable_update = false;
    retro_core_options_update_display_callback_t m_core_options_update_display_callback = nullptr;

    void SerializeToFile();
    void DeserializeFromFile();
};
}
