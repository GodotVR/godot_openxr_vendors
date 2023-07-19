#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "include/openxr_fb_scene_capture_extension_wrapper.h"

using namespace godot;

void initialize_plugin_module(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE)
	{
		ClassDB::register_class<OpenXRFbSceneCaptureExtensionWrapper>();
		OpenXRFbSceneCaptureExtensionWrapper::get_singleton()->register_extension_wrapper();
	}
}

void terminate_plugin_module(ModuleInitializationLevel p_level)
{
}

extern "C"
{
	GDExtensionBool GDE_EXPORT plugin_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_plugin_module);
        init_obj.register_terminator(terminate_plugin_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

		return init_obj.init();
	}
}
