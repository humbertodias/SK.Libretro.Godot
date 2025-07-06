extends Node

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("retro_start_emulation"):
		var monitor_node = get_node("denirosmagicmon")
		if !monitor_node:
			print("monitor node not found")
			return
			
		#Libretro.StartContent(monitor_node, "D:/Libretro", "snes9x", "E:/Roms/Super Mario World (USA).sfc")
		Libretro.StartContent(monitor_node, "D:/Libretro", "mame2003_plus", "E:/Roms/mame2003-plus/ssf2.zip")
		#Libretro.StartContent(monitor_node, "D:/Libretro", "mupen64plus_next", "E:/Roms/GoldenEye 007 (USA).z64")
		#Libretro.StartContent(monitor_node, "D:/Libretro", "ppsspp", "E:/Roms/0723 - Killzone - Liberation (Europe) (En,Fr,De,Es,It,Nl,Pl,Ru).iso")

	if event.is_action_pressed("retro_stop_emulation"):
		Libretro.StopContent()
