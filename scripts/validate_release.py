#!/usr/bin/env python3

import sys, os, re


FILES_AND_PATTERNS = {
    'config.gradle': r'defaultVersion = "([^"]*)"',
    'plugin/src/main/cpp/include/export/export_plugin.h': r'PLUGIN_VERSION = "([^"]*)"',
}


def get_version(path, pattern):
    with open(path, 'rt') as fd:
        data = fd.read()

    m = re.search(pattern, data, re.MULTILINE | re.DOTALL)
    if not m:
        raise Exception(f"Unable to find pattern {pattern} in {path}")

    return m[1]


def main():
    if len(sys.argv) < 3:
        print(f"USAGE: {sys.argv[0]} SOURCE_DIR RELEASE_VERSION", file=sys.stderr)
        sys.exit(1)

    source_dir = sys.argv[1]
    release_version = sys.argv[2]

    valid = True
    for file, pattern in FILES_AND_PATTERNS.items():
        file_parts = file.split('/')
        file_version = get_version(os.path.join(*[source_dir]+file_parts), pattern)
        if file_version != release_version:
            valid = False
            print(f"ERROR: Version from file '{file}' ({file_version}) doesn't match release version ({release_version})", file=sys.stderr)

    if not valid:
        sys.exit(1)


if __name__ == '__main__': main()
