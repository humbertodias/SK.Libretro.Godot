#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace SK
{
class Libretro : public godot::Node
{
    GDCLASS(Libretro, godot::Node);
    
public:
    static void StartContent(godot::MeshInstance3D* node, godot::String root_directory, godot::String core_name, godot::String game_path);
    static void StopContent();

    void _exit_tree();
    void _input(const godot::Ref<godot::InputEvent>& event);
    void _process(double delta);

private:
    static void _bind_methods();
};
}
