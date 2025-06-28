#include "Environment.hpp"

#include <godot_cpp/variant/string.hpp>

#include "Libretro.hpp"
#include "Debug.hpp"
#include "Input.hpp"

#include <filesystem>

#define VFS_FRONTEND
#include <vfs/vfs_implementation.h>
#include <features/features_cpu.h>

using namespace godot;

namespace SK
{
#define RETRO_ENVIRONMENT_RETROARCH_START_BLOCK 0x800000
#define RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND (2 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
#define RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB (3 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)
#define RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE (4 | RETRO_ENVIRONMENT_RETROARCH_START_BLOCK)

static retro_time_t retro_perf_get_time_usec()
{
    return cpu_features_get_time_usec();
}

static uint64_t retro_get_cpu_features()
{
    return cpu_features_get();
}

static retro_perf_tick_t retro_perf_get_counter()
{
    return cpu_features_get_perf_counter();
}

static void retro_perf_register(retro_perf_counter* counter)
{
}

static void retro_perf_start(retro_perf_counter* counter)
{
}

static void retro_perf_stop(retro_perf_counter* counter)
{
}

static void retro_perf_log()
{
}

static bool runloop_clear_all_thread_waits(uint32_t clear_threads, void* data)
{
    if (clear_threads <= 0)
        return true;

    return true;
}

Environment::Environment()
{
    m_vfs_interface =
    {
        retro_vfs_file_get_path_impl,
        retro_vfs_file_open_impl,
        retro_vfs_file_close_impl,
        retro_vfs_file_size_impl,
        retro_vfs_file_tell_impl,
        retro_vfs_file_seek_impl,
        retro_vfs_file_read_impl,
        retro_vfs_file_write_impl,
        retro_vfs_file_flush_impl,
        retro_vfs_file_remove_impl,
        retro_vfs_file_rename_impl,
        retro_vfs_file_truncate_impl,
        retro_vfs_stat_impl,
        retro_vfs_mkdir_impl,
        retro_vfs_opendir_impl,
        retro_vfs_readdir_impl,
        retro_vfs_dirent_get_name_impl,
        retro_vfs_dirent_is_dir_impl,
        retro_vfs_closedir_impl
    };    
}

bool Environment::Callback(uint32_t cmd, void* data)
{
    auto instance = Libretro::GetInstance();
    if (!instance)
    {
        LogError("Callback: Null Instance.");
        return false;
    }

    switch (cmd)
    {
    case RETRO_ENVIRONMENT_SET_ROTATION:
        instance->m_environment->m_rotation = *static_cast<uint32_t*>(data);
        return false;
    case RETRO_ENVIRONMENT_GET_OVERSCAN:
        *static_cast<int32_t*>(data) = false;
        break;
    case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        *static_cast<bool*>(data) = true;
        break;
    case RETRO_ENVIRONMENT_SET_MESSAGE:
    {
        if (!data)
            return false;

        auto message = static_cast<const retro_message*>(data);
        Log("[RETRO_ENVIRONMENT_SET_MESSAGE] message: " + std::string(message->msg));
        Log("[RETRO_ENVIRONMENT_SET_MESSAGE] frames: " + std::to_string(message->frames));
    }
    break;
    case RETRO_ENVIRONMENT_SHUTDOWN:
        Log("[RETRO_ENVIRONMENT_SHUTDOWN] Shutting down from core...");
        instance->StopEmulationThread(true);
        break;
    case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
        instance->m_environment->m_performance_level = *static_cast<uint32_t*>(data);
        break;
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    {
        if (!std::filesystem::is_directory(instance->m_environment->m_system_directory))
        {
            if (!std::filesystem::create_directories(instance->m_environment->m_system_directory))
            {
                LogError("[RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY] Failed to create system directory: " + instance->m_environment->m_system_directory);
                return false;
            }
        }
        *static_cast<const char**>(data) = instance->m_environment->m_system_directory.c_str();
    }
    break;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    {
        if (!data)
            return false;

        instance->m_video->m_pixel_format = *static_cast<retro_pixel_format*>(data);
        switch (instance->m_video->m_pixel_format)
        {
        case RETRO_PIXEL_FORMAT_0RGB1555:
            Log("[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] Using RETRO_PIXEL_FORMAT_0RGB1555");
            break;
        case RETRO_PIXEL_FORMAT_XRGB8888:
            Log("[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] Using RETRO_PIXEL_FORMAT_XRGB8888");
            break;
        case RETRO_PIXEL_FORMAT_RGB565:
            Log("[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] Using RETRO_PIXEL_FORMAT_RGB565");
            break;
        case RETRO_PIXEL_FORMAT_UNKNOWN:
            LogError("[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] Unhandled pixel format: " + std::to_string(instance->m_video->m_pixel_format));
            break;
        }
    }
    break;
    case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    {
        if (!data)
            return false;

        auto descriptors = static_cast<const retro_input_descriptor*>(data);

        instance->m_input->m_devices.clear();
        for (int i = 0; descriptors[i].description; ++i)
        {
            Input::RetroDevice device;
            device.port        = descriptors[i].port;
            device.device      = descriptors[i].device;
            device.index       = descriptors[i].index;
            device.id          = descriptors[i].id;
            device.description = descriptors[i].description;
            instance->m_input->m_devices.emplace_back(std::move(device));
        }
    }
    break;
    case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK:
    {
        auto keyboardCallback = static_cast<retro_keyboard_callback*>(data);
        if (!keyboardCallback->callback)
        {
            LogError("[RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK] Invalid callback provided.");
            return false;
        }

        instance->m_input->m_keyboard_event = keyboardCallback->callback;
    }
    break;
    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
    {
        if (!data)
        {
            instance->m_environment->m_disk_control_callback = {};
            return true;
        }

        auto diskControlCallback = static_cast<const retro_disk_control_callback*>(data);
        instance->m_environment->m_disk_control_callback = *diskControlCallback;
    }
    break;
    case RETRO_ENVIRONMENT_SET_HW_RENDER:
    {
        if (!data)
            return false;

        Log("[RETRO_ENVIRONMENT_SET_HW_RENDER] Setting hardware render callback...");

        auto hwRenderCallback = static_cast<retro_hw_render_callback*>(data);
        Log("[RETRO_ENVIRONMENT_SET_HW_RENDER] Context type: " + std::to_string(hwRenderCallback->context_type));
        if (hwRenderCallback->context_type != RETRO_HW_CONTEXT_OPENGL
         && hwRenderCallback->context_type != RETRO_HW_CONTEXT_OPENGL_CORE)
        {
            LogError("[RETRO_ENVIRONMENT_SET_HW_RENDER] Unsupported context type: " + std::to_string(hwRenderCallback->context_type));
            return false;
        }

        instance->m_video->m_context_reset = hwRenderCallback->context_reset;
        instance->m_video->m_context_destroy = hwRenderCallback->context_destroy;

        hwRenderCallback->get_current_framebuffer = Video::HwRenderGetCurrentFramebuffer;
        hwRenderCallback->get_proc_address = Video::HwRenderGetProcAddress;
    }
    break;
    case RETRO_ENVIRONMENT_GET_VARIABLE:
    {
        auto variable = static_cast<retro_variable*>(data);
        // Log("Getting variable: " + std::string(variable->key) + " = " + std::string(instance->m_environment->m_variables[variable->key]));
        variable->value = instance->m_environment->m_variables[variable->key].c_str();
    }
    break;
    case RETRO_ENVIRONMENT_SET_VARIABLES:
    {
        auto vars = static_cast<const retro_variable*>(data);
        for (int i = 0; vars[i].key; ++i)
        {
            auto values = String(vars[i].value).split(";")[1].split("|");
            auto value  = values[0].trim_prefix(" ");
            instance->m_environment->m_variables.emplace(vars[i].key, value.utf8().get_data());
        }
    }
    break;
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
    {
        *static_cast<bool*>(data) = instance->m_environment->m_variable_update;
        instance->m_environment->m_variable_update = false;
    }
    break;
    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
        instance->m_core->SetSupportsNoGame(*static_cast<bool*>(data));
        break;
    case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
        *static_cast<const char**>(data) = instance->m_core->GetPath().c_str();
        break;
    case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK:
        LogWarning("[RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK:
        LogWarning("[RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
    {
        if (!data)
            return false;

        auto rumbleInterface = static_cast<retro_rumble_interface*>(data);
        rumbleInterface->set_rumble_state = instance->m_input->RumbleInterfaceSetRumbleState;
    }
    break;
    case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
        *static_cast<uint32_t*>(data) = (1 << RETRO_DEVICE_JOYPAD)
                                      | (1 << RETRO_DEVICE_MOUSE)
                                      | (1 << RETRO_DEVICE_KEYBOARD)
                                      | (1 << RETRO_DEVICE_LIGHTGUN)
                                      | (1 << RETRO_DEVICE_ANALOG)
                                      | (1 << RETRO_DEVICE_POINTER);
        break;
    case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        static_cast<retro_log_callback*>(data)->log = Libretro::LogInterfaceLog;
        break;
    case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
    {
        if (!data)
            return false;

        auto perfcallback = static_cast<retro_perf_callback*>(data);
        perfcallback->get_time_usec = retro_perf_get_time_usec;
        perfcallback->get_cpu_features = retro_get_cpu_features;
        perfcallback->get_perf_counter = retro_perf_get_counter;
        perfcallback->perf_register = retro_perf_register;
        perfcallback->perf_start = retro_perf_start;
        perfcallback->perf_stop = retro_perf_stop;
        perfcallback->perf_log = retro_perf_log;
    }
    break;
    case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE] Environment Not Implemented");
        return false;
    // case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
    //     LogWarning("[RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY] Environment Not Implemented");
    //     return false;
    case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY:
    {
        if (!std::filesystem::is_directory(instance->m_environment->m_core_assets_directory))
        {
            if (!std::filesystem::create_directories(instance->m_environment->m_core_assets_directory))
            {
                LogError("[RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY] Failed to create core assets directory: " + instance->m_environment->m_core_assets_directory);
                return false;
            }
        }
        *static_cast<const char**>(data) = instance->m_environment->m_core_assets_directory.c_str();
    }
    break;
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    {
        if (!std::filesystem::is_directory(instance->m_environment->m_save_directory))
        {
            if (!std::filesystem::create_directories(instance->m_environment->m_save_directory))
            {
                LogError("[RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY] Failed to create save directory: " + instance->m_environment->m_save_directory);
                return false;
            }
        }
        *static_cast<const char**>(data) = instance->m_environment->m_save_directory.c_str();
    }
    break;
    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
    {
        if (!data)
            return false;

        auto systemAvInfo = static_cast<const retro_system_av_info*>(data);

        Log("[RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO] System AV Info: " + std::to_string(systemAvInfo->geometry.base_width) + "x" + std::to_string(systemAvInfo->geometry.base_height) +
                   " @ " + std::to_string(systemAvInfo->geometry.max_width) + "x" + std::to_string(systemAvInfo->geometry.max_height) +
                   " (aspect ratio: " + std::to_string(systemAvInfo->geometry.aspect_ratio) + ")" +
                   "FPS: " + std::to_string(systemAvInfo->timing.fps) + " Sample Rate: " + std::to_string(systemAvInfo->timing.sample_rate));
    }
    break;
    case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
        LogWarning("[RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
    {
        if (!data)
            return false;

        // auto subsystemInfo = static_cast<const retro_subsystem_info*>(data);
        // for (int i = 0; subsystemInfo[i].desc; ++i)
        // {
        //     Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] desc: " + String(subsystemInfo[i].desc));
        //     Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] ident: " + String(subsystemInfo[i].ident));
        //     for (int j = 0; j < subsystemInfo[i].num_roms; ++j)
        //     {
        //         Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom desc: " + String(subsystemInfo[i].roms[j].desc));
        //         Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom valid_extensions: " + String(subsystemInfo[i].roms[j].valid_extensions));
        //         Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom need_fullpath: " + String::num_int64(subsystemInfo[i].roms[j].need_fullpath));
        //         Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom block_extract: " + String::num_int64(subsystemInfo[i].roms[j].block_extract));
        //         Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom required: " + String::num_int64(subsystemInfo[i].roms[j].required));
        //         for (size_t k = 0; k < subsystemInfo[i].roms[j].num_memory; ++k)
        //         {
        //             Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom memory extension: " + String(subsystemInfo[i].roms[j].memory[k].extension));
        //             Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] rom memory type: " + String::num_int64(subsystemInfo[i].roms[j].memory[k].type));
        //         }
        //     }
        //     Log("[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] id: " + String::num_int64(subsystemInfo[i].id));
        // }
    }
    break;
    case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    {
        auto controllerInfo = static_cast<const retro_controller_info*>(data);
        if (!controllerInfo)
            return false;

        for (int i = 0; controllerInfo[i].types; ++i)
        {
            std::vector<Input::RetroController> controllers;
            for (int j = 0; j < controllerInfo[i].num_types; ++j)
            {
                // Looks like some cores don't set num_types to the correct value...
                if (!controllerInfo[i].types[j].desc) 
                    continue;
                
                Input::RetroController controller;
                controller.name = controllerInfo[i].types[j].desc;
                controller.id   = controllerInfo[i].types[j].id;
                controllers.emplace_back(std::move(controller));
            }
            instance->m_input->m_controllers.emplace_back(std::move(controllers));
        }
    }
    break;
    case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
    {
        if (!data)
            return false;

        auto memoryMaps = static_cast<const retro_memory_map*>(data);
        for (size_t i = 0; i < memoryMaps->num_descriptors; ++i)
        {
            const retro_memory_descriptor& descriptor = memoryMaps->descriptors[i];
            //Log("Memory Descriptor: flags = " + std::to_string(descriptor.flags));
        }
    }
    break;
    case RETRO_ENVIRONMENT_SET_GEOMETRY:
    {
        // TODO: Handle geometry changes when not applied to a 3D model (ie. Sprite2D)

        auto geometry = static_cast<const retro_game_geometry*>(data);
        if (!geometry)
            return false;

        Log("[RETRO_ENVIRONMENT_SET_GEOMETRY] " + std::to_string(geometry->base_width) + "x" + std::to_string(geometry->base_height) +
            " @ " + std::to_string(geometry->max_width) + "x" + std::to_string(geometry->max_height) +
            " (aspect ratio: " + std::to_string(geometry->aspect_ratio) + ")");
    }
    break;
    case RETRO_ENVIRONMENT_GET_USERNAME:
        *static_cast<const char**>(data) = instance->m_username.c_str();
        break;
    case RETRO_ENVIRONMENT_GET_LANGUAGE:
    {
        if (!data)
            return false;

        *static_cast<retro_language*>(data) = RETRO_LANGUAGE_ENGLISH;
    }
    break;
    case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
    {
        return false;

        // if (!data)
        //     return false;

        // auto framebuffer = static_cast<retro_framebuffer*>(data);
        // Log("[RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER]");
        // Log("framebuffer->data:" + std::to_string(reinterpret_cast<uintptr_t>(framebuffer->data)));
        // Log("framebuffer->width: " + std::to_string(framebuffer->width));
        // Log("framebuffer->height: " + std::to_string(framebuffer->height));
        // Log("framebuffer->pitch: " + std::to_string(framebuffer->pitch));
        // Log("framebuffer->format: " + std::to_string(framebuffer->format));
        // Log("framebuffer->access_flags: " + std::to_string(framebuffer->access_flags));
        // Log("framebuffer->memory_flags: " + std::to_string(framebuffer->memory_flags));

        // framebuffer->data         = nullptr;
        // framebuffer->pitch        = framebuffer->width * 2;
        // framebuffer->format       = RETRO_PIXEL_FORMAT_RGB565;
        // framebuffer->memory_flags = RETRO_MEMORY_TYPE_CACHED;
    }
    break;
    case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS:    
    {
        if (!data)
            return false;

        auto supportAchievements = *static_cast<bool*>(data);
        // Log("[RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS] Support: " + (supportAchievements ? "enabled" : "disabled"));
    }
    break;
    case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
        LogWarning("[RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
        LogWarning("[RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
    {
        auto vfsInterfaceInfo = static_cast<retro_vfs_interface_info*>(data);
        if (vfsInterfaceInfo->required_interface_version > s_supported_vfs_version)
        {
            LogError("[RETRO_ENVIRONMENT_GET_VFS_INTERFACE] Requested VFS interface not implemented: " + std::to_string(vfsInterfaceInfo->required_interface_version));
            return false;
        }

        vfsInterfaceInfo->required_interface_version = s_supported_vfs_version;
        vfsInterfaceInfo->iface                      = &instance->m_environment->m_vfs_interface;
    }
    break;
    case RETRO_ENVIRONMENT_GET_LED_INTERFACE:
    {
        auto led_interface = static_cast<retro_led_interface*>(data);
        if (!led_interface)
            return false;

        led_interface->set_led_state = instance->LedInterfaceSetLedState;
    }
    break;
    case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
    {
        auto audioVideoEnable = static_cast<retro_av_enable_flags*>(data);
        if (!audioVideoEnable)
            return false;

        *audioVideoEnable = (retro_av_enable_flags)(RETRO_AV_ENABLE_AUDIO | RETRO_AV_ENABLE_VIDEO);
    }
    break;
    case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_MIDI_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_FASTFORWARDING:
    {
        if (!data)
            return false;

        *static_cast<bool*>(data) = false;
    }
    break;
    case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
        LogWarning("[RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
    {
        // data should be ignored but just in case a core misuses this environment
        if (data)
            *static_cast<bool*>(data) = true;
    }
    break;
    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
    {
        if (!data)
            return false;

        *static_cast<uint32_t*>(data) = 2;
    }
    break;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS:
        LogWarning("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL:
        LogWarning("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY:
        // LogWarning("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
    {
        if (!data)
            return false;

        *static_cast<retro_hw_context_type*>(data) = RETRO_HW_CONTEXT_OPENGL_CORE;
    }
    break;
    case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
        *static_cast<uint32_t*>(data) = 1;
        break;
    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
    {
        if (!data)
        {
            instance->m_environment->m_disk_control_ext_callback = {};
            return true;
        }

        auto diskControlExtCallback = static_cast<const retro_disk_control_ext_callback*>(data);
        instance->m_environment->m_disk_control_ext_callback = *diskControlExtCallback;
    }
    break;
    case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
        *static_cast<uint32_t*>(data) = 1;
        break;
    case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
    {
        if (!data)
            return false;

        auto messageExt = static_cast<const retro_message_ext*>(data);
        Log("[RETRO_ENVIRONMENT_SET_MESSAGE_EXT] message: " + std::string(messageExt->msg) +
            " duration: " + std::to_string(messageExt->duration) +
            " priority: " + std::to_string(messageExt->priority) +
            " level: " + std::to_string(messageExt->level) +
            " target: " + std::to_string(messageExt->target) +
            " type: " + std::to_string(messageExt->type) +
            " progress: " + std::to_string(messageExt->progress));
    }
    break;
    case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
        LogWarning("[RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
    {
        if (!data)
        {
            Log("[RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK] callback unregistered");
            instance->m_audio->m_audio_buffer_status_callback = nullptr;
            return true;
        }

        auto bufferStatusCallback = static_cast<retro_audio_buffer_status_callback*>(data);
        if (!bufferStatusCallback->callback)
        {
            LogError("[RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK] provided callback is null");
            return false;
        }

        instance->m_audio->m_audio_buffer_status_callback = bufferStatusCallback->callback;
        Log("[RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK] callback registered");
    }
    case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
    {
        if (!data)
            return false;

        auto minimumAudioLatency = *static_cast<uint32_t*>(data);
        Log("[RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY] Minimum audio latency hinted: " + std::to_string(minimumAudioLatency));
    }
    break;
    case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
        LogWarning("[RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
        LogWarning("[RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
        LogWarning("[RETRO_ENVIRONMENT_GET_GAME_INFO_EXT] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
    {
        if (!data)
        {
            Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] Clearing options");
            instance->m_environment->m_variables.clear();
            return true;
        }

        auto coreOptionsV2 = static_cast<const retro_core_options_v2*>(data);
        instance->m_environment->ProcessCoreOptionsV2(instance, coreOptionsV2);
    }
    break;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
    {
        if (!data)
        {
            Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL] Clearing options");
            instance->m_environment->m_variables.clear();
            return true;
        }

        auto coreOptionsV2Intl = static_cast<const retro_core_options_v2_intl*>(data);
        retro_core_options_v2* coreOptionsV2 = nullptr;
        if (coreOptionsV2Intl->local)
            coreOptionsV2 = coreOptionsV2Intl->local;
        else
            coreOptionsV2 = coreOptionsV2Intl->us;
        ProcessCoreOptionsV2(instance, coreOptionsV2);
    }
    break;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK:
    {
        if (!data)
        {
            instance->m_environment->m_core_options_update_display_callback = nullptr;
            return true;
        }

        auto coreOptionsUpdateDisplayCallback = static_cast<retro_core_options_update_display_callback*>(data);
        instance->m_environment->m_core_options_update_display_callback = coreOptionsUpdateDisplayCallback->callback;
    }
    break;
    case RETRO_ENVIRONMENT_SET_VARIABLE:
    {
        if (!data)
            return true;

        auto variable = static_cast<retro_variable*>(data);
        if (instance->m_environment->m_variables.find(variable->key) != instance->m_environment->m_variables.end())
        {
            Log("[RETRO_ENVIRONMENT_SET_VARIABLE] Setting variable: " + std::string(variable->key) + " = " + std::string(variable->value));
            instance->m_environment->m_variables[variable->key] = variable->value;
            instance->m_environment->m_variable_update = true;
        }
        else
        {
            LogError("[RETRO_ENVIRONMENT_SET_VARIABLE] Variable not found: " + std::string(variable->key));
            variable->value = nullptr;
        }
    }
    break;
    case RETRO_ENVIRONMENT_GET_THROTTLE_STATE:
    {
        if (!data)
            return false;

        auto throttleState = static_cast<retro_throttle_state*>(data);
        throttleState->mode = RETRO_THROTTLE_NONE;
        throttleState->rate = 0.0f;
    }
    break;
    case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT:
        LogWarning("[RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT:
        LogWarning("[RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_JIT_CAPABLE:
        LogWarning("[RETRO_ENVIRONMENT_GET_JIT_CAPABLE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_DEVICE_POWER:
        LogWarning("[RETRO_ENVIRONMENT_GET_DEVICE_POWER_STATE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE:
        LogWarning("[RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY:
        LogWarning("[RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY:
        LogWarning("[RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY] Environment Not Implemented");
        return false;
    // custom environment commands
    case RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND:
        LogWarning("[RETRO_ENVIRONMENT_SET_SAVE_STATE_IN_BACKGROUND] Environment Not Implemented");
        return false;
    case RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB:
        *static_cast<retro_environment_t*>(data) = runloop_clear_all_thread_waits;
        break;
    case RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE:
        LogWarning("[RETRO_ENVIRONMENT_POLL_TYPE_OVERRIDE] Environment Not Implemented");
        return false;
    default:
        LogError("[RETRO_ENVIRONMENT_UNKNOWN] " + std::to_string(cmd));
        return false;
    }

    return true;
}

void Environment::ProcessCoreOptionsV2(Libretro* instance, const retro_core_options_v2* coreOptionsV2)
{
    // if (coreOptionsV2->categories)
    // {
    //     for (retro_core_option_v2_category* cat = coreOptionsV2->categories; cat->key; ++cat)
    //     {
    //         Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] Category - Key: " + std::string(cat->key) +
    //             " Desc:" + std::string(cat->desc) +
    //             " Info: " + std::string(cat->info ? cat->info : "explicit null"));
    //     }
    // }

    for (retro_core_option_v2_definition* def = coreOptionsV2->definitions; def->key; ++def)
    {
        // Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] Definition - Key: " + std::string(def->key) +
        //     " Desc:" + std::string(def->desc) +
        //     " Desc Categorized: " + std::string(def->desc_categorized ? def->desc_categorized : "explicit null") +
        //     " Info: " + std::string(def->info ? def->info : "explicit null") +
        //     " Info Categorized: " + std::string(def->info_categorized ? def->info_categorized : "explicit null") +
        //     " Category Key: " + std::string(def->category_key ? def->category_key : "explicit null"));
        
        // for (retro_core_option_value* value = def->values; value->value; ++value)
        // {
        //     Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] Value - Value: " + std::string(value->value) +
        //         " Label: " + std::string(value->label ? value->label : "explicit null"));
        // }

        // Log("[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] Default Value: " + std::string(def->default_value));

        if (def->key && def->default_value)
            instance->m_environment->m_variables[def->key] = def->default_value;
    }
}
}
