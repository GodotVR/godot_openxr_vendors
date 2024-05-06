#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GODOT=${GODOT:-godot}
PROJECT=${PROJECT:-$SCRIPT_DIR/../demo}

die() {
    echo "$@" >/dev/stderr
    exit 1
}

# Regenerate the XML data from the API.
$GODOT --doctool "$SCRIPT_DIR/.." --path "$PROJECT" --gdextension-docs --xr-mode off \
    || die "Failed to regenerate XML using Godot's --doctool"
