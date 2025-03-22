# LinuxMCE Build Scripts Documentation

This document provides detailed information about the build scripts in the `/home/dad/LinuxMCE/src/Ubuntu_Helpers_NoHardcode/build-scripts` directory, which are critical for bringing the LinuxMCE project to buildable status.

## Core Build System Scripts

1. **build-maindebs.sh**
   - Purpose: Main script for building LinuxMCE packages
   - Functionality:
     - Sets up environment variables and MySQL credentials
     - Configures multi-core build if available
     - Updates version information in the database
     - Executes MakeRelease to compile and package components
     - Handles distribution-specific exclusions (packages that won't build)
     - Supports different versions (Hardy, Intrepid, Lucid, Precise, Trusty, Xenial, Jammy)

2. **build-replacements.sh**
   - Purpose: Builds additional packages that replace or supplement distribution packages
   - Functionality:
     - Supports caching to avoid rebuilding unchanged packages
     - Builds replacement packages like pthsem, bcusdk, lirc, vdr, motion, etc.
     - Distribution-specific replacements for Ubuntu vs Raspbian
     - Architecture-specific builds (x86 vs ARM)
     - Handles Qt4-dependent components

3. **build-makerelease.sh**
   - Purpose: Compiles the MakeRelease tool (a custom build system for LinuxMCE)
   - Functionality:
     - Precompiles core libraries: pluto_main, PlutoUtils, SerializeClass, LibDCE, etc.
     - Creates the MakeRelease binary and supporting libraries
     - Copies them to the build environment

4. **checkout-svn.sh**
   - Purpose: Manages SVN repository checkout and updates
   - Functionality:
     - Initial checkout or update of source code from SVN
     - Handles both public and private repositories
     - Preserves the previous version for comparison
     - Tracks SVN revision numbers

## Repository Management Scripts

1. **create-repo.sh**
   - Purpose: Creates a local APT repository of built packages
   - Functionality:
     - Collects packages from different build directories
     - Removes duplicates, keeping only latest versions
     - Generates Packages and Packages.gz files for APT
     - Optional handling of private packages

2. **name-packages.sh**
   - Purpose: Defines package IDs and groupings for MakeRelease
   - Functionality:
     - Contains export definitions for all package IDs
     - Groups packages for easier reference in build scripts
     - Maps source and binary package IDs

3. **version-helper.sh**
   - Purpose: Manages version information for builds
   - Functionality:
     - Creates version.h with build date and SVN revision
     - Used to stamp all compiled code with version info

## CD/DVD Building Scripts

1. **cd1-build.sh** and **cd2-build.sh**
   - Purpose: Create ISO images for LinuxMCE installation CDs
   - Functionality:
     - Collects packages from local repository
     - Filters packages based on inclusion/exclusion lists
     - Removes duplicates
     - Generates ISO images

2. **makedvd.sh**
   - Purpose: Creates a live DVD installer from Ubuntu base
   - Functionality:
     - Extracts and modifies Ubuntu ISO
     - Installs LinuxMCE packages and dependencies
     - Adds installer scripts and desktop shortcuts
     - Prepares for final packaging

3. **finalizedvd.sh**
   - Purpose: Finalizes the DVD image creation
   - Functionality:
     - Creates filesystem manifests
     - Compresses the filesystem
     - Configures boot options
     - Adds preseed file for automated installation
     - Generates final ISO image

## Simulation Scripts

1. **build-maindebs-sim.sh**
   - Purpose: Simulates package building (test run)
   - Functionality:
     - Similar to build-maindebs.sh but with -c (compile only) and -S (simulate) flags
     - Tests compilation and packaging without creating actual packages
     - Useful for testing before actual builds

## Key Dependencies and Relationships

1. **Compilation Chain**:
   - First build-makerelease.sh compiles the MakeRelease tool
   - Then build-maindebs.sh uses MakeRelease to build core components
   - build-replacements.sh builds additional packages independently

2. **Package Flow**:
   - Source → MakeRelease → Package Building → Repository Creation → ISO Building

3. **Configuration Dependencies**:
   - All scripts source builder.conf and ubuntu.conf
   - Build targets are specific to Ubuntu version (gutsy, hardy, intrepid, lucid, precise, trusty, xenial, jammy)

4. **Cross-Platform Support**:
   - Architecture-specific builds (amd64, i386, armhf)
   - Distribution-specific builds (Ubuntu vs Raspbian)

These scripts form the backbone of the LinuxMCE build system, allowing it to compile from source code to installable ISO images. The system is highly modular and configurable, with specific adaptations for different Ubuntu versions and hardware architectures.