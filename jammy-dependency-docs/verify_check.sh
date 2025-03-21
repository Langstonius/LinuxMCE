#!/bin/bash

echo "=== Checking for package coverage ==="

# Count the original packages properly
ORIG_COUNT=$(cat /home/dad/LinuxMCE/src/Ubuntu_Helpers_NoHardcode/conf-files/jammy-amd64/build-packages | wc -l)
echo "Original file line count: $ORIG_COUNT"

# Count lines in updated packages
UPDATED_COUNT=$(cat /home/dad/LinuxMCE/updated_build_packages | grep -v "^$" | grep -v "^# Updated" | grep -v "^# Original" | grep -v "^# Modified" | wc -l)
echo "Updated file active line count: $UPDATED_COUNT"

# Count commented out packages (replacements)
COMMENTED_COUNT=$(cat /home/dad/LinuxMCE/updated_build_packages | grep "^# .*-" | wc -l)
echo "Commented packages (with replacements): $COMMENTED_COUNT"

# Count packages in missing list
MISSING_COUNT=$(cat /home/dad/LinuxMCE/missing_packages.txt | grep -v "^Packages" | grep -v "^$" | wc -l)
echo "Packages in missing list: $MISSING_COUNT"

echo ""
echo "=== Validating if all packages are accounted for ==="
echo "Total accounted packages (active + commented): $((UPDATED_COUNT - COMMENTED_COUNT + MISSING_COUNT))"
echo ""

# Manual check for a few key missing packages to verify they're in our output files
echo "=== Sampling verification ==="
for pkg in "libkonq5-dev" "libqt4-dev" "dh-systemd" "python3-oauth"; do
  if grep -q "$pkg" /home/dad/LinuxMCE/missing_packages.txt; then
    echo "✓ $pkg properly identified as missing"
  else
    echo "✗ $pkg not found in missing_packages.txt"
  fi
  
  if grep -q "$pkg" /home/dad/LinuxMCE/replacement_suggestions.txt; then
    echo "✓ $pkg has replacement suggestions"
  else
    echo "✗ $pkg missing from replacement_suggestions.txt"
  fi
  
  if grep -q "$pkg" /home/dad/LinuxMCE/package_migration.md; then
    echo "✓ $pkg included in migration document"
  else
    echo "✗ $pkg missing from package_migration.md"
  fi
  echo ""
done