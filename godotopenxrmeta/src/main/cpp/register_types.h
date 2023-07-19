#pragma once

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_plugin_module(ModuleInitializationLevel p_level);
void terminate_plugin_module(ModuleInitializationLevel p_level);
