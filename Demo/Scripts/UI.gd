extends Control

@export var game_node : Node3D

func _unhandled_input(event: InputEvent) -> void:
	var libretro = game_node.get_node("denirosmagicmon").get_node("Libretro") as Libretro

	if event.is_action_pressed("retro_start_emulation"):
		libretro.start_content("D:/Libretro", "mame2003_plus", "E:/Roms/mame2003-plus/ssf2.zip")
		#libretro.start_content("D:/Libretro", "mupen64plus_next", "E:/Roms/GoldenEye 007 (USA).z64")
		#libretro.start_content("D:/Libretro", "ppsspp", "E:/Roms/0723 - Killzone - Liberation (Europe) (En,Fr,De,Es,It,Nl,Pl,Ru).iso")

	if event.is_action_pressed("retro_stop_emulation"):
		libretro.stop_content()
