#!/bin/bash

echo "=== Comprehensive Validation ==="
echo "Checking that all identified missing packages are documented"
echo ""

# Get the list of missing packages
missing_packages=$(grep -v "^Packages" /home/dad/LinuxMCE/missing_packages.txt | grep -v "^$")

# Check each missing package
while read -r pkg; do
  pkg=$(echo "$pkg" | xargs) # Trim whitespace
  
  # Skip empty lines
  if [ -z "$pkg" ]; then
    continue
  fi
  
  echo "Checking documentation for: $pkg"
  
  # Check if it's in the package_migration.md file
  if grep -q "$pkg" /home/dad/LinuxMCE/package_migration.md; then
    echo "  ✓ Included in migration document"
  else
    echo "  ✗ MISSING from package_migration.md"
  fi
  
  # Check if it's commented out in the updated_build_packages file
  if grep -q "^# $pkg" /home/dad/LinuxMCE/updated_build_packages; then
    echo "  ✓ Commented in updated build packages"
  else
    # Check if it appears elsewhere in the file (might be a replacement)
    if grep -q "$pkg" /home/dad/LinuxMCE/updated_build_packages; then
      echo "  ✓ Referenced in updated build packages"
    else
      echo "  ✗ MISSING from updated_build_packages"
    fi
  fi
  
  echo ""
done <<< "$missing_packages"

echo "=== Summary ==="
echo "Total missing packages: $(echo "$missing_packages" | wc -l)"
echo "If all checks passed, all missing packages are properly documented and addressed in the migration plan."