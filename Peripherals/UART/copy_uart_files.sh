#!/bin/bash

# Script to copy all UART folder file contents to paste.txt
# Usage: ./copy_uart_files.sh

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UART_DIR="$SCRIPT_DIR"
OUTPUT_FILE="$UART_DIR/paste.txt"

# Clear the output file
> "$OUTPUT_FILE"

echo "Copying all UART files to paste.txt..."
echo "UART Directory: $UART_DIR"
echo "Output File: $OUTPUT_FILE"
echo ""

# Add header to the output file
cat >> "$OUTPUT_FILE" << EOF
================================================================================
UART Folder Contents - Generated on $(date)
================================================================================

EOF

# Function to add file content with header
add_file_content() {
    local file="$1"
    local filename=$(basename "$file")

    echo "Processing: $filename"

    cat >> "$OUTPUT_FILE" << EOF
--------------------------------------------------------------------------------
FILE: $filename
--------------------------------------------------------------------------------
EOF

    # Add the file content
    cat "$file" >> "$OUTPUT_FILE"

    cat >> "$OUTPUT_FILE" << EOF


EOF
}

# Find all files in the UART directory (excluding paste.txt and this script)
find "$UART_DIR" -maxdepth 1 -type f \
    -not -name "paste.txt" \
    -not -name "copy_uart_files.sh" \
    -not -name ".*" \
    | sort | while read -r file; do

    # Check if file is readable
    if [[ -r "$file" ]]; then
        add_file_content "$file"
    else
        echo "Warning: Cannot read file $file"
    fi
done

# Add footer
cat >> "$OUTPUT_FILE" << EOF
================================================================================
End of UART Folder Contents - $(date)
Total files processed: $(find "$UART_DIR" -maxdepth 1 -type f -not -name "paste.txt" -not -name "copy_uart_files.sh" -not -name ".*" | wc -l)
================================================================================
EOF

echo ""
echo "Done! All UART files have been copied to paste.txt"
echo "Output file size: $(du -h "$OUTPUT_FILE" | cut -f1)"
echo "Total lines: $(wc -l < "$OUTPUT_FILE")"
