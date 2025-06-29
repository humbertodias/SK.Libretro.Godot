SCONS_TARGET=template_debug

godot-cpp/pull:
	git submodule update --init --recursive

# godot-cpp/build:	godot-cpp/pull
# 	cd godot-cpp && scons target=${SCONS_TARGET}

# godot-extension/build:
# 	scons target=${SCONS_TARGET}

clean:
	rm -rf godot-cpp/bin