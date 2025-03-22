#!/bin/bash

# Set the source directory
SRC_DIR="../src"

# Create output directory if it doesn't exist
RESULTS_DIR="dependency_usage_results"
mkdir -p "$RESULTS_DIR"

# Read the missing packages
MISSING_PACKAGES=($(grep -v "^Packages missing in Ubuntu Jammy:" missing_packages.txt))

echo "Searching for usage of missing packages in source code..."

# Search patterns for different types of package usage
for package in "${MISSING_PACKAGES[@]}"; do
  echo "Analyzing package: $package"
  
  # Remove -dev suffix for library searches
  lib_name=${package%-dev}
  
  # Output file for this package
  output_file="$RESULTS_DIR/${package}_usage.txt"
  
  echo "Results for $package" > "$output_file"
  echo "=================================================" >> "$output_file"
  
  # Search in CMakeLists.txt files (find_package, pkg_check_modules)
  echo "CMake dependencies:" >> "$output_file"
  grep -r --include="CMakeLists.txt" -E "(find_package|pkg_check_modules).*${lib_name}" "$SRC_DIR" >> "$output_file" 2>/dev/null
  
  # Search in Makefiles
  echo -e "\nMakefile dependencies:" >> "$output_file"
  grep -r --include="Makefile*" "${lib_name}" "$SRC_DIR" >> "$output_file" 2>/dev/null
  
  # Search in debian/control files
  echo -e "\nDebian package dependencies:" >> "$output_file"
  grep -r --include="control" "${package}" "$SRC_DIR/*/debian/" >> "$output_file" 2>/dev/null
  
  # Search in configure scripts and autoconf files
  echo -e "\nConfigure/Autoconf dependencies:" >> "$output_file"
  grep -r --include="configure*" --include="*.ac" --include="*.in" "${lib_name}" "$SRC_DIR" >> "$output_file" 2>/dev/null
  
  # Search for header includes (for dev packages)
  if [[ "$package" == *-dev ]]; then
    # Try to determine possible header names (simple heuristic)
    header_pattern=${lib_name#lib}
    header_pattern=${header_pattern%%[0-9]*}
    
    echo -e "\nPossible header includes:" >> "$output_file"
    grep -r --include="*.h" --include="*.hpp" --include="*.c" --include="*.cpp" --include="*.cc" \
      -E "#include.*${header_pattern}" "$SRC_DIR" >> "$output_file" 2>/dev/null
  fi
  
  # For Qt4 specifically
  if [[ "$package" == "libqt4-dev" ]]; then
    echo -e "\nQt4 usage:" >> "$output_file"
    grep -r --include="*.cpp" --include="*.h" --include="*.cc" -E "(Qt4|QT_VERSION|QtCore|QtGui)" "$SRC_DIR" >> "$output_file" 2>/dev/null
    grep -r --include="CMakeLists.txt" -E "(Qt4|qt4|QT4)" "$SRC_DIR" >> "$output_file" 2>/dev/null
  fi
  
  # For Python packages
  if [[ "$package" == python3-* ]]; then
    module_name=${package#python3-}
    echo -e "\nPython module imports:" >> "$output_file"
    grep -r --include="*.py" -E "import\s+${module_name}|from\s+${module_name}\s+import" "$SRC_DIR" >> "$output_file" 2>/dev/null
  fi
  
  # Count the number of results
  result_count=$(grep -v "^=" "$output_file" | grep -v "^$" | grep -v "^[A-Za-z]* dependencies:" | wc -l)
  echo -e "\nTotal references found: $result_count" >> "$output_file"
  
  # Create a summary for quick review
  echo "$package: $result_count references" >> "$RESULTS_DIR/summary.txt"
done

echo "Search complete. Results saved in $RESULTS_DIR/"
echo "See $RESULTS_DIR/summary.txt for a quick overview."