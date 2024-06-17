#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("thirdparty/godot-cpp/SConstruct")

opts = Variables('custom.py')
opts.Update(env)

# Add common includes
env.Append(CPPPATH=[
    "#common/src/main/cpp/include/",
    "#thirdparty/openxr/include/",
    ])

sources = []
sources += Glob("#common/src/main/cpp/*.cpp")
sources += Glob("#common/src/main/cpp/export/*.cpp")
sources += Glob("#common/src/main/cpp/extensions/*.cpp")
sources += Glob("#common/src/main/cpp/classes/*.cpp")

if env["target"] in ["editor", "template_debug"]:
  doc_data = env.GodotCPPDocData("#common/src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
  sources.append(doc_data)

binary_path = '#demo/addons/godotopenxrvendors/.bin'
project_name = 'godotopenxrvendors'

# Statically link with libgcc and libstdc++ for more Linux compatibility.
if env["platform"] == "linux":
    env.Append(
        LINKFLAGS=[
            "-Wl,--no-undefined",
            "-static-libgcc",
            "-static-libstdc++",
        ]
    )

# Create the library target
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/{1}/{2}/lib{3}.{1}.framework/{3}.{1}".format(
            binary_path,
            env["platform"],
            env["target"],
            project_name,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/{}/{}/{}/lib{}{}".format(
            binary_path,
            env["platform"],
            env["target"],
            env["arch"],
            project_name,
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)

