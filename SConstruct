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
    ])

common_objects = []
common_objects.append(env.SharedObject(Glob("#common/src/main/cpp/export/*.cpp")))

SConscript([
    "godotopenxrmeta/SConstruct",
    "godotopenxrpico/SConstruct",
    "godotopenxrlynx/SConstruct",
    "godotopenxrkhronos/SConstruct",
    ], 'env common_objects')
