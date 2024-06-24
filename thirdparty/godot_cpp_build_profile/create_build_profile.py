#!/usr/bin/env python

import os
import re
import json

SOURCE_DIRS = [
    '../../common',
    '../../godotopenxrkhronos',
    '../../godotopenxrlynx',
    '../../godotopenxrmeta',
    '../../godotopenxrpico',
]

GODOT_CPP_HEADERS = "../godot-cpp/gen/include/godot_cpp/classes"
#GODOT_CPP_SOURCES = "../godot-cpp/gen/src/classes"

INCLUDE_PATTERN = re.compile(r'#include <godot_cpp/classes/(\w+)\.hpp>')
CLASS_PATTERN = re.compile(r'class\s+(\w+)\s*:\s*public')

processed_files = {}
class_list = []

def find_includes(file_path):
    """
    Finds all the includes in a given file and returns the list of CLASS names.
    """
    includes = []
    try:
        with open(file_path, 'r') as file:
            content = file.read()
            includes = INCLUDE_PATTERN.findall(content)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    return includes

def find_primary_class(file_path):
    """
    Finds the primary class name in a given .hpp file by looking for 'class ClassName : public'.
    """
    try:
        with open(file_path, 'r') as file:
            for line in file:
                match = CLASS_PATTERN.search(line)
                if match:
                    return match.group(1)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    return None

def process_file(file_path, top_level=False):
    """
    Processes a given file for includes and processes the included files recursively.
    """
    if file_path in processed_files:
        return

    processed_files[file_path] = True

    if not top_level and file_path.endswith('.hpp'):
        primary_class = find_primary_class(file_path)
        if primary_class and primary_class not in class_list:
            class_list.append(primary_class)

    includes = find_includes(file_path)
    for include_name in includes:
        hpp_file = f"{GODOT_CPP_HEADERS}/{include_name}.hpp"
        #cpp_file = f"{GODOT_CPP_SOURCES}/{include_name}.cpp"

        if os.path.exists(hpp_file):
            process_file(hpp_file)
        #if os.path.exists(cpp_file):
        #    process_file(cpp_file)

def main():
    for dir in SOURCE_DIRS:
        for root, _, files in os.walk(dir):
            for file in files:
                if file.endswith('.cpp') or file.endswith('.h'):
                    file_path = os.path.join(root, file)
                    process_file(file_path, True)

    class_list.sort()

    build_profile = {
        "enabled_classes": class_list
    }

    with open("build_profile.json", "wt") as file:
        json.dump(build_profile, file, indent=4)

if __name__ == "__main__":
    main()