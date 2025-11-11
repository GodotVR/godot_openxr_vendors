import os

def build_raw_header(source_filename: str, constant_name: str) -> None:
    # Read the source file content.
    with open(source_filename, "r") as source_file:
        source_content = source_file.read()
        constant_name = constant_name.replace(".", "_")
        # Build header content using a C raw string literal.
        header_content = (
            "/* THIS FILE IS GENERATED. EDITS WILL BE LOST. */\n\n"
            "#pragma once\n\n"
            f"inline constexpr const char *{constant_name}"
            " = "
            f'R"<!>({source_content})<!>"'
            ";\n"
        )
        # Write the header to the provided file name with a ".gen.h" suffix.
        header_filename = f"{source_filename}.gen.h"
        with open(header_filename, "w") as header_file:
            header_file.write(header_content)

def build_raw_headers_action(target, source, env):
    env.NoCache(target)
    for src in source:
        source_filename = str(src)
        constant_name = os.path.basename(source_filename)
        build_raw_header(source_filename, constant_name)
