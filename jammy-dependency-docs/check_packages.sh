#!/bin/bash

# Create a file to store missing packages
echo "Packages missing in Ubuntu Jammy:" > ../missing_packages.txt

# Check each package from the file
while read pkg; do
  # Skip empty lines and comments
  if [[ -z "$pkg" || "$pkg" =~ ^# ]]; then
    continue
  fi
  
  # Trim leading/trailing whitespace
  pkg=$(echo "$pkg" | xargs)
  
  # Use apt-cache policy to check if the package exists
  if ! apt-cache policy "$pkg" 2>/dev/null | grep -q "Candidate:"; then
    echo "$pkg" >> ../missing_packages.txt
  fi
done < ../src/Ubuntu_Helpers_NoHardcode/conf-files/jammy-amd64/build-packages

echo "Finished checking packages. Results saved to ../missing_packages.txt"