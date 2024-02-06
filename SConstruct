#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("thirdparty/godot-cpp/SConstruct")

opts = Variables('custom.py')
opts.Update(env)

# Add common includes
env.Append(CPPPATH=[
    "#common/src/main/cpp",
    "#thirdparty/openxr/include/",
    "#thirdparty/openxr-meta-ext/include/",
    ])

sources = []
sources += Glob("#common/src/main/cpp/*.cpp")
sources += Glob("#common/src/main/cpp/export/*.cpp")
sources += Glob("#common/src/main/cpp/extensions/*.cpp")

binary_path = '#demo/addons/godotopenxrvendors/.bin'
project_name = 'godotopenxrvendors'

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

