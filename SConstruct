#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("thirdparty/godot-cpp/SConstruct")

opts = Variables('custom.py')
opts.Update(env)

# Add source files.
env.Append(CPPPATH=[
# Meta includes
    "godotopenxrmeta/src/main/cpp",
    "godotopenxrmeta/src/main/cpp/include",
    "godotopenxrmeta/libs/ovr_openxr_mobile_sdk/OpenXR/Include",
# Common includes
    "thirdparty/openxr/include/",
    ])

# Meta source files
sources = Glob("godotopenxrmeta/src/main/cpp/*.cpp")

meta_binary_path = 'demo/addons/godotopenxrvendors/meta/.bin'
meta_project_name = 'godotopenxrmeta'

# Create the library target
if env["platform"] == "android":
    print("Use gradle to generate the Android binaries")
    Exit(255)
elif env["platform"] == "macos":
    meta_library = env.SharedLibrary(
        "{0}/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            meta_binary_path,
            meta_project_name,
            env["platform"],
            env["target"],
        ),
        source=sources,
    )
else:
    meta_library = env.SharedLibrary(
        "{}/lib{}.{}.{}.{}{}".format(
            meta_binary_path,
            meta_project_name,
            env["platform"],
            env["target"],
            env["arch"],
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(meta_library)
