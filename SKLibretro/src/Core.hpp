#pragma once

#include <libretro.h>

#include <string>

#include "Debug.hpp"

namespace SK
{
class Libretro;

class Core
{
public:
    Core(const std::string& path);
    ~Core() = default;

    bool Load();
    void Unload();

    std::string GetName() const { return m_name; }
    std::string GetPath() const { return m_path; }
    bool GetSupportsNoGame() const { return m_supports_no_game; }
    void SetSupportsNoGame(bool supports) { m_supports_no_game = supports; }

    decltype(&retro_set_environment) retro_set_environment_func                       = nullptr;
    decltype(&retro_set_video_refresh) retro_set_video_refresh_func                   = nullptr;
    decltype(&retro_set_audio_sample) retro_set_audio_sample_func                     = nullptr;
    decltype(&retro_set_audio_sample_batch) retro_set_audio_sample_batch_func         = nullptr;
    decltype(&retro_set_input_poll) retro_set_input_poll_func                         = nullptr;
    decltype(&retro_set_input_state) retro_set_input_state_func                       = nullptr;
    decltype(&retro_init) retro_init_func                                             = nullptr;
    decltype(&retro_deinit) retro_deinit_func                                         = nullptr;
    decltype(&retro_api_version) retro_api_version_func                               = nullptr;
    decltype(&retro_get_system_info) retro_get_system_info_func                       = nullptr;
    decltype(&retro_get_system_av_info) retro_get_system_av_info_func                 = nullptr;
    decltype(&retro_set_controller_port_device) retro_set_controller_port_device_func = nullptr;
    decltype(&retro_reset) retro_reset_func                                           = nullptr;
    decltype(&retro_run) retro_run_func                                               = nullptr;
    decltype(&retro_serialize_size) retro_serialize_size_func                         = nullptr;
    decltype(&retro_serialize) retro_serialize_func                                   = nullptr;
    decltype(&retro_unserialize) retro_unserialize_func                               = nullptr;
    decltype(&retro_cheat_reset) retro_cheat_reset_func                               = nullptr;
    decltype(&retro_cheat_set) retro_cheat_set_func                                   = nullptr;
    decltype(&retro_load_game) retro_load_game_func                                   = nullptr;
    decltype(&retro_load_game_special) retro_load_game_special_func                   = nullptr;
    decltype(&retro_unload_game) retro_unload_game_func                               = nullptr;
    decltype(&retro_get_region) retro_get_region_func                                 = nullptr;
    decltype(&retro_get_memory_data) retro_get_memory_data_func                       = nullptr;
    decltype(&retro_get_memory_size) retro_get_memory_size_func                       = nullptr;

private:
    std::string m_name;
    std::string m_path;
    void* m_handle = nullptr;
    bool m_supports_no_game = false;

    bool LoadHandle();

    template<typename T>
    bool LoadFunction_(T& functionPtr, const std::string& functionName)
    {
        const std::string suffix = "_func";    
        std::string funcName = functionName.substr(0, functionName.size() - suffix.size());
        functionPtr = reinterpret_cast<T>(LoadFunctionNative(funcName.c_str()));
        if (!functionPtr)
        {
            LogError("Failed to load function: " + funcName);
            return false;
        }
        return true;
    }

    void* LoadHandleNative();
    void UnloadHandleNative();
    void* LoadFunctionNative(const char* functionName);
};
}
