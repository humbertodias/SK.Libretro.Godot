extends Node

@export var root_directory : String
@export var core_name : String
@export var game_path : String

var libretro : Libretro

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("retro_start_emulation"):
		libretro.start_content(root_directory, core_name, game_path)
	if event.is_action_pressed("retro_stop_emulation"):
		libretro.stop_content()
