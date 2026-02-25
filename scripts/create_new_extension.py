#!/usr/bin/env python3
"""Generate boilerplate header and source files for a new OpenXR extension wrapper.

Takes an extension name in the format XR_VENDOR_feature_name, where:
  - XR is the literal prefix
  - VENDOR is the vendor name (uppercase)
  - feature_name is the feature in snake_case (or UPPER_CASE, will be lowered)

The vendor, feature, class name, file names, and OpenXR constant are all derived
from this single input.

Usage:
    # Interactive mode:
    python scripts/create_new_extension.py

    # CLI mode:
    python scripts/create_new_extension.py XR_ANDROID_light_estimation
    python scripts/create_new_extension.py XR_FB_color_space
"""

import argparse
import os
import sys
from string import Template

# Repo layout constants.
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
TEMPLATE_DIR = os.path.join(SCRIPT_DIR, "templates")
HEADER_DIR = os.path.join(REPO_ROOT, "plugin", "src", "main", "cpp", "include", "extensions")
SOURCE_DIR = os.path.join(REPO_ROOT, "plugin", "src", "main", "cpp", "extensions")


def snake_to_pascal(snake: str) -> str:
    """Convert a snake_case string to PascalCase."""
    return "".join(word.capitalize() for word in snake.split("_"))


def pad_filename_comment(filename: str) -> str:
    """Pad a filename to fit the copyright header comment field."""
    # The comment format is: /*  <filename><padding>*/
    # Inner width between the two spaces after /* and before */ is 68 chars.
    inner_width = 68
    if len(filename) >= inner_width:
        return filename
    return filename + " " * (inner_width - len(filename))


def parse_extension_name(name: str) -> tuple[str, str]:
    """Parse 'XR_VENDOR_feature_name' into (vendor, feature).

    The vendor is always the second component (lowered).
    Everything after the vendor is the feature name (lowered).
    """
    parts = name.split("_")
    if len(parts) < 3 or parts[0].upper() != "XR":
        print(f"Error: Extension name must be in the format XR_VENDOR_feature_name, got: {name}", file=sys.stderr)
        sys.exit(1)

    vendor = parts[1].lower()
    feature = "_".join(parts[2:]).lower()

    if not feature:
        print(f"Error: No feature name found in: {name}", file=sys.stderr)
        sys.exit(1)

    return vendor, feature


def build_template_vars(vendor: str, feature: str) -> dict:
    """Compute all template substitution variables."""
    file_base = f"openxr_{vendor}_{feature}_extension"
    class_name = f"OpenXR{vendor.capitalize()}{snake_to_pascal(feature)}Extension"
    ext_name_constant = f"XR_{vendor.upper()}_{feature.upper()}_EXTENSION_NAME"

    return {
        "vendor": vendor,
        "feature": feature,
        "file_base": file_base,
        "header_file_name": f"{file_base}.h",
        "source_file_name": f"{file_base}.cpp",
        "class_name": class_name,
        "ext_name_constant": ext_name_constant,
        "ext_bool_name": f"{vendor}_{feature}_ext",
        "init_function_name": f"initialize_{vendor}_{feature}_extension",
        "ext_display_name": f"XR_{vendor.upper()}_{feature}",
    }


def render_template(template_path: str, vars: dict) -> str:
    """Read a template file and substitute variables."""
    with open(template_path, "r") as f:
        tmpl = Template(f.read())

    return tmpl.substitute(vars)


def generate(params: dict, dry_run: bool = False) -> None:
    """Generate the header and source files."""
    header_template = os.path.join(TEMPLATE_DIR, "extension.h.template")
    source_template = os.path.join(TEMPLATE_DIR, "extension.cpp.template")

    header_out = os.path.join(HEADER_DIR, params["header_file_name"])
    source_out = os.path.join(SOURCE_DIR, params["source_file_name"])

    # Render header.
    h_vars = dict(params, file_name=pad_filename_comment(params["header_file_name"]))
    header_content = render_template(header_template, h_vars)

    # Render source.
    s_vars = dict(params, file_name=pad_filename_comment(params["source_file_name"]))
    source_content = render_template(source_template, s_vars)

    if dry_run:
        print(f"\n--- {header_out} ---")
        print(header_content)
        print(f"\n--- {source_out} ---")
        print(source_content)
        return

    # Check for existing files.
    for path in (header_out, source_out):
        if os.path.exists(path):
            print(f"Error: {path} already exists. Aborting.", file=sys.stderr)
            sys.exit(1)

    with open(header_out, "w") as f:
        f.write(header_content)
    print(f"Created: {header_out}")

    with open(source_out, "w") as f:
        f.write(source_content)
    print(f"Created: {source_out}")

    print()
    print("Next steps:")
    print(f"  1. Add #include \"extensions/{params['header_file_name']}\" to register_types.cpp")
    print(f"  2. Register the class in initialize_plugin() and uninitialize_plugin()")
    print(f"  3. Add EXT_PROTO_XRRESULT_FUNCn() declarations to the header")
    print(f"  4. Add GDEXTENSION_INIT_XR_FUNC_V() calls in {params['init_function_name']}()")


def main():
    parser = argparse.ArgumentParser(
        description="Generate boilerplate for a new OpenXR extension wrapper.",
    )
    parser.add_argument("extension_name", nargs="?",
        help="Extension name in XR_VENDOR_feature_name format (e.g. XR_ANDROID_light_estimation)")
    parser.add_argument("--dry-run", action="store_true",
        help="Print generated files to stdout instead of writing")

    args = parser.parse_args()

    if args.extension_name:
        vendor, feature = parse_extension_name(args.extension_name)
    else:
        # Interactive mode.
        print("Generate a new OpenXR extension wrapper")
        print("=" * 42)
        print()
        name = ""
        while not name:
            name = input("Extension name (e.g. XR_ANDROID_light_estimation): ").strip()
        vendor, feature = parse_extension_name(name)

    params = build_template_vars(vendor, feature)

    print(f"  Vendor:    {vendor}")
    print(f"  Feature:   {feature}")
    print(f"  Class:     {params['class_name']}")
    print(f"  Header:    {params['header_file_name']}")
    print(f"  Source:    {params['source_file_name']}")
    print(f"  Constant:  {params['ext_name_constant']}")
    print()

    generate(params, dry_run=args.dry_run)


if __name__ == "__main__":
    main()
