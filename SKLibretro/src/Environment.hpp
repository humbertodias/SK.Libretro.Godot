#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <libretro.h>

namespace SK
{
class Libretro;

class Environment
{
public:
    Environment();
    ~Environment() = default;

    static bool Callback(uint32_t cmd, void* data);
    
    static const uint32_t s_supported_vfs_version = 3;

    uint32_t m_rotation = 0;
    uint32_t m_performance_level = 0;
    std::string m_system_directory;
    std::string m_save_directory;
    std::string m_core_assets_directory;
    retro_vfs_interface m_vfs_interface;
    retro_core_options_update_display_callback_t m_core_options_update_display_callback = nullptr;
    retro_disk_control_callback m_disk_control_callback = {};
    retro_disk_control_ext_callback m_disk_control_ext_callback = {};
    std::unordered_map<std::string, std::string> m_variables = {};
    bool m_variable_update = false;
};
}
