#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("thirdparty/godot-cpp/SConstruct")

opts = Variables('custom.py')
opts.Update(env)

# Add common includes
env.Append(CPPPATH=[
    "#plugin/src/main/cpp/include/",
    "#thirdparty/openxr/include/",
    ])

sources = []
sources += Glob("#plugin/src/main/cpp/*.cpp")
sources += Glob("#plugin/src/main/cpp/export/*.cpp")
sources += Glob("#plugin/src/main/cpp/extensions/*.cpp")
sources += Glob("#plugin/src/main/cpp/classes/*.cpp")

if env["target"] in ["editor", "template_debug"]:
  doc_data = env.GodotCPPDocData("#plugin/src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
  sources.append(doc_data)

binary_path = '#demo/addons/godotopenxrvendors/.bin'
android_src_path = '#plugin/src'
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
elif env["platform"] == "android":
    android_target = "release" if env["target"] == "template_release" else "debug"
    android_arch = ""
    if env["arch"] == "arm32":
        android_arch = "armeabi-v7a"
    elif env["arch"] == "arm64":
        android_arch = "arm64-v8a"
    elif env["arch"] == "x86_32":
        android_arch = "x86"
    elif env["arch"] == "x86_64":
        android_arch = "x86_64"
    else:
        raise Exception("Unable to map %s to Android architecture name" % env["arch"])

    library = env.SharedLibrary(
        "{}/main/libs/{}/{}/{}/lib{}{}".format(
            android_src_path,
            android_target,
            android_arch,
            android_arch,
            project_name,
            env["SHLIBSUFFIX"],
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

