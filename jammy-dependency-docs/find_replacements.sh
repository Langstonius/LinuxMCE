#!/bin/bash

# File containing missing packages
MISSING_FILE="/home/dad/LinuxMCE/missing_packages.txt"
# Output file for replacement suggestions
REPLACEMENTS_FILE="/home/dad/LinuxMCE/replacement_suggestions.txt"

# Initialize the replacements file
echo "Suggested replacements for missing packages in Ubuntu Jammy:" > $REPLACEMENTS_FILE
echo "===========================================================" >> $REPLACEMENTS_FILE
echo "" >> $REPLACEMENTS_FILE

# Process each missing package
while read -r line; do
  # Skip header line and empty lines
  if [[ "$line" == "Packages missing in Ubuntu Jammy:" || -z "$line" ]]; then
    continue
  fi
  
  # Extract the package name without version
  pkg=$(echo "$line" | awk '{print $1}')
  
  echo "Looking for replacements for: $pkg" >> $REPLACEMENTS_FILE
  
  # Extract the base package name (remove lib prefix, -dev suffix, and version numbers)
  base_name=$(echo "$pkg" | sed -E 's/^lib//; s/-dev$//; s/[0-9.]+$//')
  
  echo "  Base name search: $base_name" >> $REPLACEMENTS_FILE
  
  # Search for similar packages
  echo "  Similar packages available:" >> $REPLACEMENTS_FILE
  apt-cache search "$base_name" | grep -i dev | head -n 10 >> $REPLACEMENTS_FILE
  
  # If it's a Qt package, look for Qt5/Qt6 equivalents
  if [[ "$pkg" == *"qt4"* ]]; then
    echo "  Potential Qt5/Qt6 replacements:" >> $REPLACEMENTS_FILE
    apt-cache search "qt5-" | grep -i dev | head -n 5 >> $REPLACEMENTS_FILE
    apt-cache search "qt6-" | grep -i dev | head -n 5 >> $REPLACEMENTS_FILE
  fi
  
  # For Python packages, look for newer versions
  if [[ "$pkg" == "python3-"* ]]; then
    py_pkg=${pkg#python3-}
    echo "  Potential Python replacement packages:" >> $REPLACEMENTS_FILE
    apt-cache search "python3-$py_pkg" >> $REPLACEMENTS_FILE
    apt-cache search "python3-.*$py_pkg" >> $REPLACEMENTS_FILE
  fi
  
  # For libgnome related packages, look for GTK3/GTK4 equivalents
  if [[ "$pkg" == *"gnome"* ]]; then
    echo "  Potential GNOME/GTK replacements:" >> $REPLACEMENTS_FILE
    apt-cache search "libgtk-3" | grep -i dev | head -n 5 >> $REPLACEMENTS_FILE
    apt-cache search "libgtk-4" | grep -i dev | head -n 5 >> $REPLACEMENTS_FILE
  fi
  
  # Special case for dh-systemd which was merged into debhelper
  if [[ "$pkg" == "dh-systemd" ]]; then
    echo "  Note: dh-systemd functionality is now part of debhelper (>= 9.20160709)" >> $REPLACEMENTS_FILE
    apt-cache show debhelper | grep Version >> $REPLACEMENTS_FILE
  fi
  
  echo "" >> $REPLACEMENTS_FILE
  echo "----------------------------------------" >> $REPLACEMENTS_FILE
  echo "" >> $REPLACEMENTS_FILE
  
done < $MISSING_FILE

echo "Replacement suggestions saved to $REPLACEMENTS_FILE"