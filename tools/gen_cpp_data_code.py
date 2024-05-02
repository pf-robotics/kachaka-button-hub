#!/usr/bin/env python3

import sys
import os


def generate_cpp_array_from_file(input_filename, output_filename):
    # Generate array name and size name from output_filename
    base_name = os.path.basename(input_filename)
    array_name_base = base_name.replace(".", "_")
    array_name = f"{array_name_base}_data"
    size_name = f"{array_name_base}_size"

    # Read file in binary mode
    with open(input_filename, "rb") as f:
        bytes_data = f.read()

    # Generate C++ code for uint8_t array and size, considering 80 characters per line
    array_elements = [f"0x{byte:02x}" for byte in bytes_data]
    line_length = 76  # Considering 4 spaces indentation
    array_content_lines = []
    current_line = ""
    for element in array_elements:
        if len(current_line) + len(element) + 2 > line_length:  # 2 for ", "
            array_content_lines.append(current_line)
            current_line = element
        else:
            if current_line:
                current_line += ", "
            current_line += element
    array_content_lines.append(current_line)  # Add any remaining content

    array_content = ",\n    ".join(array_content_lines)
    cpp_code = (
        f"#include <cstdint>\n"
        f"#include <cstddef>\n"
        f'#include "data.hpp"\n\n'
        f"namespace data {{\n\n"
        f"const uint8_t {array_name}[] = {{\n    {array_content}\n}};\n\n"
        f"const size_t {size_name} = sizeof({array_name});\n\n"
        f"}} // namespace data\n"
    )

    # Write the generated code to the output file
    with open(output_filename, "w") as f:
        f.write(cpp_code)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 script.py input_file_name output_file_name")
    else:
        input_filename = sys.argv[1]
        output_filename = sys.argv[2]
        generate_cpp_array_from_file(input_filename, output_filename)
