#!/bin/bash

# Set variables
JAMMY_PACKAGES_FILE="../src/Ubuntu_Helpers_NoHardcode/conf-files/jammy-amd64/build-packages"
MISSING_PACKAGES_FILE="missing_packages.txt"
OUTPUT_FILE="replacement_candidates.md"

# Check if required files exist
if [ ! -f "$MISSING_PACKAGES_FILE" ]; then
    echo "Error: Missing packages file not found at $MISSING_PACKAGES_FILE"
    exit 1
fi

if [ ! -f "$JAMMY_PACKAGES_FILE" ]; then
    echo "Error: Jammy packages file not found at $JAMMY_PACKAGES_FILE"
    exit 1
fi

# Create output file with header
cat > "$OUTPUT_FILE" << EOF
# Replacement Package Candidates

This document lists potential replacement packages for those that are missing in Ubuntu Jammy.

## Methodology
Packages are identified by:
1. Searching for similar package names in Ubuntu Jammy repositories
2. Identifying successor packages based on library version numbers
3. Finding functional equivalents where direct replacements don't exist

## Replacement Candidates

EOF

# Read the missing packages file and process each package
grep -v "^Packages missing in Ubuntu Jammy:" "$MISSING_PACKAGES_FILE" | while read -r package; do
    if [ -z "$package" ]; then
        continue
    fi
    
    echo "Processing $package..."
    
    # Write package header to output
    echo "### $package" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # Basic pattern matching - look for similarly named packages
    base_name=$(echo "$package" | sed -E 's/-dev$//' | sed -E 's/[0-9]+$//' | sed -E 's/lib//')
    
    echo "#### Similar Packages in Jammy" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    echo '```' >> "$OUTPUT_FILE"
    apt-cache search "$base_name" | grep -E "lib.*$base_name|$base_name.*lib" >> "$OUTPUT_FILE"
    echo '```' >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # For dev packages, suggest newer versions
    if [[ "$package" == *-dev ]]; then
        lib_base=$(echo "$package" | sed -E 's/-dev$//')
        echo "#### Newer Development Packages" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
        echo '```' >> "$OUTPUT_FILE"
        apt-cache search "$lib_base[0-9]+-dev" >> "$OUTPUT_FILE"
        echo '```' >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
    fi
    
    # Special cases with known replacements
    case "$package" in
        "libqt4-dev")
            echo "#### Recommended Replacement" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "- **qtbase5-dev**: Main Qt5 development package" >> "$OUTPUT_FILE"
            echo "- **qt5-qmake**: qmake for Qt5" >> "$OUTPUT_FILE"
            echo "- **qtdeclarative5-dev**: For QML development" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            ;;
        "libgnome2-dev" | "libgnomeui-dev" | "libgnomevfs2-dev")
            echo "#### Recommended Replacement" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "- **libgtk-3-dev**: GTK3 development files" >> "$OUTPUT_FILE"
            echo "- **libglib2.0-dev**: GLib development files" >> "$OUTPUT_FILE"
            if [ "$package" = "libgnomevfs2-dev" ]; then
                echo "- **libglib2.0-dev-bin**: GIO/GVfs utilities for file access" >> "$OUTPUT_FILE"
            fi
            echo "" >> "$OUTPUT_FILE"
            ;;
        "dh-systemd")
            echo "#### Recommended Replacement" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "- **debhelper**: dh-systemd functionality is now part of debhelper" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            ;;
        "python3-oauth")
            echo "#### Recommended Replacement" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "- **python3-requests-oauthlib**: Modern OAuth library for Python" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            ;;
        "python3-urlgrabber")
            echo "#### Recommended Replacement" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            echo "- **python3-requests**: Modern HTTP library for Python" >> "$OUTPUT_FILE"
            echo "" >> "$OUTPUT_FILE"
            ;;
    esac
    
    echo "---" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
done

echo "Replacement candidates list created in $OUTPUT_FILE"