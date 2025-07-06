#include "Libretro.hpp"

#include <godot_cpp/classes/audio_stream_player.hpp>

#include "Wrapper.hpp"

using namespace godot;

namespace SK
{
void Libretro::StartContent(MeshInstance3D* node, String root_directory, String core_name, String game_path)
{
    Wrapper::GetInstance()->StartContent(node, root_directory, core_name, game_path);
}

void Libretro::StopContent()
{
    Wrapper::GetInstance()->StopContent();
}

void Libretro::_exit_tree()
{
    StopContent();
}

void Libretro::_input(const Ref<InputEvent>& event)
{
    Wrapper::GetInstance()->_input(event);
}

void Libretro::_process(double delta)
{
    Wrapper::GetInstance()->_process(delta);
}

void Libretro::_bind_methods()
{
    ClassDB::bind_static_method("Libretro", D_METHOD("StartContent", "node", "root_directory", "core_name", "game_path"), &Libretro::StartContent);
    ClassDB::bind_static_method("Libretro", D_METHOD("StopContent"), &Libretro::StopContent);
}
}
