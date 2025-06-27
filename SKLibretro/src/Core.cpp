#include "Core.hpp"

#include <SDL3/SDL_loadso.h>

#include "Libretro.hpp"

#include <filesystem>
#include <sstream>
#include <random>

#include "Debug.hpp"
#include "Environment.hpp"
#include "Video.hpp"
#include "Audio.hpp"
#include "Input.hpp"

namespace SK
{
static uint32_t RandomChar()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

static std::string GenerateHex(const uint32_t len)
{
    std::stringstream ss;
    for (auto i = 0; i < len; i++)
    {
        const auto rc = RandomChar();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

#define LoadFunction(funcPtr) \
    if (!LoadFunction_(funcPtr, #funcPtr)) \
        return false; \

Core::Core(const std::string& path)
: m_path(path)
{
}

bool Core::Load()
{
    if (!std::filesystem::is_regular_file(m_path))
    {
        LogError("Core not found: " + m_path);
        return false;
    }

    std::string name = std::filesystem::path(m_path).filename().replace_extension("").string();
    const std::string suffix = "_libretro";
    size_t pos = name.rfind(suffix);
    if (pos != std::string::npos)
        name.erase(pos, suffix.length());
    m_name = name;

    std::string extension = std::filesystem::path(m_path).extension().string();
    std::filesystem::path temp_path = std::filesystem::path(Libretro::GetInstance()->GetTempDirectory()) / (name + GenerateHex(10) + extension);
    try
    {
        std::filesystem::copy_file(m_path, temp_path, std::filesystem::copy_options::overwrite_existing);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LogError("Failed to copy core file: " + m_path + " to " + temp_path.string() + " - " + e.what());
        return false;
    }
    m_path = temp_path.string();
    std::replace(m_path.begin(), m_path.end(), '\\', '/');

    if (!LoadHandle())
        return false;

    LoadFunction(retro_set_environment_func);
    LoadFunction(retro_set_video_refresh_func);
    LoadFunction(retro_set_audio_sample_func);
    LoadFunction(retro_set_audio_sample_batch_func);
    LoadFunction(retro_set_input_poll_func);
    LoadFunction(retro_set_input_state_func);
    LoadFunction(retro_init_func);
    LoadFunction(retro_deinit_func);
    LoadFunction(retro_api_version_func);
    LoadFunction(retro_get_system_info_func);
    LoadFunction(retro_get_system_av_info_func);
    LoadFunction(retro_set_controller_port_device_func);
    LoadFunction(retro_reset_func);
    LoadFunction(retro_run_func);
    LoadFunction(retro_serialize_size_func);
    LoadFunction(retro_serialize_func);
    LoadFunction(retro_unserialize_func);
    LoadFunction(retro_cheat_reset_func);
    LoadFunction(retro_cheat_set_func);
    LoadFunction(retro_load_game_func);
    LoadFunction(retro_load_game_special_func);
    LoadFunction(retro_unload_game_func);
    LoadFunction(retro_get_region_func);
    LoadFunction(retro_get_memory_data_func);
    LoadFunction(retro_get_memory_size_func);

    retro_set_environment_func(Environment::Callback);
    retro_set_video_refresh_func(Video::RefreshCallback);
    retro_set_audio_sample_func(Audio::SampleCallback);
    retro_set_audio_sample_batch_func(Audio::SampleBatchCallback);
    retro_set_input_poll_func(Input::PollCallback);
    retro_set_input_state_func(Input::StateCallback);

    retro_init_func();

    return true;
}

void Core::Unload()
{
    if (m_handle)
    {
        UnloadHandleNative();
        m_handle = nullptr;
    }

    if (std::filesystem::is_regular_file(m_path))
    {
        try
        {
            std::filesystem::remove(m_path);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            LogError("Failed to remove core file: " + m_path + " - " + e.what());
        }
    }
}

bool Core::LoadHandle()
{
    m_handle = LoadHandleNative();
    if (!m_handle)
    {
        LogError("Failed to load core handle: " + m_path);
        return false;
    }
    return true;
}

void* Core::LoadHandleNative()
{
    return SDL_LoadObject(m_path.c_str());
}

void Core::UnloadHandleNative()
{
    SDL_UnloadObject(static_cast<SDL_SharedObject*>(m_handle));
}

void* Core::LoadFunctionNative(const char* functionName)
{
    return SDL_LoadFunction(static_cast<SDL_SharedObject*>(m_handle), functionName);
}
}
