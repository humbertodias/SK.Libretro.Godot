#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include <SDL3/SDL_video.h>

#include <libretro.h>
#include <readerwriterqueue.h>

#include "Core.hpp"
#include "Environment.hpp"
#include "Video.hpp"
#include "Audio.hpp"
#include "Input.hpp"

class SDL_Window;

namespace SK
{
class Libretro : public godot::Node
{
    GDCLASS(Libretro, Node);

public:
    Libretro() = default;
    ~Libretro() override = default;

    void _ready() override;
    void _exit_tree() override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
    void _process(double delta) override;

    void StartContent(godot::String root_directory, godot::String core_name, godot::String game_path);
    void StopContent();

    static Libretro* GetInstance();

    std::string GetTempDirectory() const { return m_temp_directory; }

    void StopEmulationThread(bool log);

    std::unique_ptr<Core> m_core = nullptr;
    std::unique_ptr<Environment> m_environment = nullptr;
    std::unique_ptr<Video> m_video = nullptr;
    std::unique_ptr<Audio> m_audio = nullptr;
    std::unique_ptr<Input> m_input = nullptr;

    class ThreadCommand
    {
    public:
        ThreadCommand(Libretro* instance);
        virtual ~ThreadCommand() = default;

        virtual void Execute() = 0;

    protected:
        Libretro* m_instance;
    };

    class ThreadCommandInitAudio : public ThreadCommand
    {
    public:
        ThreadCommandInitAudio(Libretro* instance, float bufferCapacitySec, double sampleRate);
        ~ThreadCommandInitAudio() override = default;

        void Execute() override;

    private:
        float m_bufferCapacitySec;
        double m_sampleRate;
    };

    class ThreadCommandCreateTexture : public ThreadCommand
    {
    public:
        ThreadCommandCreateTexture(Libretro* instance, godot::Image::Format image_format, godot::PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y);
        ~ThreadCommandCreateTexture() override = default;

        void Execute() override;

    private:
        godot::Image::Format m_imageFormat;
        godot::PackedByteArray m_pixelData;
        int32_t m_width;
        int32_t m_height;
        bool m_flipY;
    };

    class ThreadCommandUpdateTexture : public ThreadCommand
    {
    public:
        ThreadCommandUpdateTexture(Libretro* instance, godot::PackedByteArray pixelData, bool flipY);
        ~ThreadCommandUpdateTexture() override = default;

        void Execute() override;

    private:
        godot::PackedByteArray m_pixelData;
        bool m_flipY;
    };

    static Libretro* s_instance;
    static std::mutex s_instance_mutex;

    static bool RegisterInstance(Libretro* instance);
    static void UnregisterInstance();

    std::thread m_thread;
    moodycamel::ReaderWriterQueue<std::unique_ptr<ThreadCommand>> m_main_thread_commands_queue;
    std::mutex m_mutex;
    bool m_mutex_done = false;
    std::condition_variable m_condition_variable;
    bool m_running = false;

    std::string m_root_directory;
    std::string m_temp_directory;
    std::string m_username = "DefaultUser";
    retro_log_level m_log_level = RETRO_LOG_WARN;

    std::string m_game_path;

    std::vector<unsigned char> m_game_buffer;

    godot::Ref<godot::StandardMaterial3D> m_original_surface_material_override = nullptr;
    godot::Ref<godot::StandardMaterial3D> m_new_material = nullptr;

    void DeInitGraphics();
    void InitAudio(float bufferLengthSec, double sampleRate);
    void DeinitAudio();

    void StartEmulationThread(godot::String root_directory, godot::String core_name, godot::String game_path);
    void EmulationThreadLoop();
    void CreateTexture(godot::Image::Format image_format, godot::PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y);
    void UpdateTexture(godot::PackedByteArray pixel_data, bool flip_y);

    static void LogInterfaceLog(retro_log_level level, const char* fmt, ...);
    static void LedInterfaceSetLedState(int32_t led, int32_t state);

    static void _bind_methods();
};
}
