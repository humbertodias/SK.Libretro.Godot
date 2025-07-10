extends Node

func _ready() -> void:
	Libretro.ConnectOptionsReady(Callable(self, "_on_options_ready"))
	
	
func _on_options_ready():
	var categories = Libretro.GetOptionCategories()
	var definitions = Libretro.GetOptionDefinitions()
	
	var container = get_tree().current_scene.get_node("OptionsContainer")
	for child in container.get_children():
		child.queue_free()

	for key in categories.keys():
		var category : LibretroOptionCategory = categories[key]
		var category_desc = category.GetDescription()
		var category_info = category.GetInfo()

		var category_label = Label.new()
		category_label.text = "%s: %s" % [category_desc, category_info]
		container.add_child(category_label)

		for def_key in definitions.keys():
			var definition : LibretroOptionDefinition = definitions[def_key]
			if definition.GetCategoryKey() == key:
				var row = HBoxContainer.new()
				
				var option_label = Label.new()
				option_label.text = definition.GetDescription()
				row.add_child(option_label)

				var option_dropdown = OptionButton.new()
				var values = definition.GetValues()
				for value in values:
					# value is a LibretroOptionValue
					option_dropdown.add_item(value.GetValue())
				# Optionally, set the default value as selected
				for i in range(option_dropdown.item_count):
					if option_dropdown.get_item_metadata(i) == definition.GetDefaultValue():
						option_dropdown.select(i)
						break
				row.add_child(option_dropdown)
				container.add_child(row)

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
