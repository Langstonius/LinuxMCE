# LinuxMCE Build System Modernization Proposal

This document outlines a proposed approach to modernize the LinuxMCE build system while maintaining compatibility with the SQL database requirements and existing components.

## Current Build System Analysis

The current build system, particularly `build-maindebs.sh`, has several challenges:

1. **Hard-coded version identifiers**: Uses hard-coded version strings and distribution IDs
2. **Ad-hoc environment preparation**: Contains multiple "hackozaurus" comments indicating temporary hacks
3. **Monolithic structure**: All logic is in one large function with limited modularity
4. **Legacy build system**: Uses a custom MakeRelease tool rather than standard build tools
5. **Manual exclude lists**: Package IDs are managed through long, comma-separated lists
6. **Inconsistent case statements**: The distribution-specific logic has different patterns
7. **Multiple commented sections**: Indicates unclear purpose or obsolete code
8. **Limited documentation**: Minimal comments explaining functionality

## Proposed Modernization Approach

### 1. Maintain SQL Database Integration

Given that the SQL database contains essential device trees and other information needed during the build process, it must be preserved while improving how it's used:

```bash
# Example: Improved database integration
setup_database_connection() {
    # Load credentials from config or environment
    local db_host="${DB_HOST:-localhost}"
    local db_user="${DB_USER:-root}"
    local db_pass="${DB_PASS:-}"
    local db_port="${DB_PORT:-3306}"
    local db_name="${DB_NAME:-pluto_main_build}"
    
    # Export formatted credentials for tools that need them
    export DB_DSN="mysql:host=${db_host};port=${db_port};dbname=${db_name}"
    
    # For backward compatibility with MakeRelease
    export PLUTO_BUILD_CRED="-h ${db_host} -P ${db_port} -u ${db_user}"
    [[ -n "$db_pass" ]] && export PLUTO_BUILD_CRED="${PLUTO_BUILD_CRED} -p ${db_pass}"
    
    # Test connection and provide helpful error if failed
    if ! mysql ${PLUTO_BUILD_CRED} -e "SELECT 1" >/dev/null 2>&1; then
        log_error "Failed to connect to database. Please check credentials."
        return 1
    fi
}
```

### 2. Structured Configuration Management

Move from hard-coded values to configuration files:

```yaml
# configs/build-jammy-amd64.yaml
---
build:
  arch: amd64
  distro: ubuntu
  distro_id: 24  # For Jammy
  repo_source: 25
  main_version: 2.0.0.49.
  
database:
  host: localhost
  user: root
  pass: ""
  name: pluto_main_build
  
exclude_packages:
  # Commenting each exclusion improves maintainability
  - id: "426,427"  # videolan_plugin_common - not used
  - id: "429,430"  # videolan_plugin_server - not used  
  - id: "431,432"  # videolan_plugin_client - broken
  # ...additional exclusions...
```

### 3. Modularized Script Structure

Break down the monolithic script into modular components:

```
build/
├── config/
│   ├── jammy-amd64.yaml    # Platform-specific settings
│   └── build.conf          # Global build settings
├── lib/
│   ├── common.sh           # Common functions
│   ├── database.sh         # Database interaction
│   ├── logging.sh          # Logging functions
│   ├── makerelease.sh      # MakeRelease wrapper
│   └── workarounds.sh      # Documented workarounds
├── scripts/
│   ├── prepare.sh          # Set up build environment
│   ├── compile.sh          # Component compilation
│   ├── package.sh          # Package creation
│   └── version.sh          # Version management
└── build.sh                # Main entry point
```

### 4. Wrapper for MakeRelease

Create a maintainable wrapper for the legacy MakeRelease tool:

```bash
# In makerelease.sh
run_makerelease() {
    local options=()
    local make_jobs="${MAKE_JOBS:-}"
    
    # Common options
    options+=(-a)  # All
    options+=(-R "${SVN_REVISION}")
    options+=(-O "${OUTPUT_DIR}")
    options+=(-D "${DB_NAME}")
    options+=(-o "${DISTRO_ID}")
    options+=(-r "${REPO_SOURCE}")
    options+=(-m "${MODULE_RANGE:-1,1176}")
    options+=(-s "${SOURCE_DIR}")
    options+=(-n "/")
    options+=(-d)  # Debug info
    
    # Add exclude list if provided
    [[ -n "${EXCLUDE_LIST:-}" ]] && options+=(-K "${EXCLUDE_LIST}")
    
    # Log the command for debugging
    log_command "MakeRelease" "${make_jobs}" "${options[@]}"
    
    # Execute MakeRelease with arch variable set
    env arch="${BUILD_ARCH}" "${MAKERELEASE_BIN}" ${make_jobs} "${options[@]}"
}
```

### 5. Improved Logging and Error Handling

Implement structured logging and robust error handling:

```bash
# In logging.sh
log() {
    local level="$1"
    shift
    local message="$*"
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    
    # Color coding for terminal output
    local color=""
    local reset="\033[0m"
    
    case "${level}" in
        ERROR)   color="\033[31m" ;; # Red
        WARNING) color="\033[33m" ;; # Yellow
        INFO)    color="\033[36m" ;; # Cyan
        DEBUG)   color="\033[90m" ;; # Gray
        *)       color="" ;;
    esac
    
    # Log to file
    echo "[${timestamp}] [${level}] ${message}" >> "${LOG_FILE}"
    
    # Log to console with appropriate color
    echo -e "${color}[${level}] ${message}${reset}"
}

handle_error() {
    local exit_code=$?
    log "ERROR" "Build failed with exit code $exit_code at ${BASH_SOURCE[1]} line ${BASH_LINENO[0]}"
    exit $exit_code
}

trap handle_error ERR
```

### 6. Improved Main Build Script

A more maintainable, modular main script:

```bash
#!/usr/bin/env bash
set -euo pipefail

# Source common utilities and functions
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
source "${SCRIPT_DIR}/lib/common.sh"
source "${SCRIPT_DIR}/lib/database.sh"
source "${SCRIPT_DIR}/lib/makerelease.sh"

function build_main_debs() {
    local config_file="${1:-configs/build-${BUILD_DISTRO}-${BUILD_ARCH}.yaml}"
    
    # Load configuration
    log_step "Loading configuration from ${config_file}"
    load_config "${config_file}"
    
    # Setup database connection
    log_step "Setting up database connection"
    setup_database_connection
    
    # Setup environment variables
    log_step "Setting up build environment"
    setup_build_environment
    
    # Prepare build directory
    log_step "Preparing build directory"
    prepare_build_directory
    
    # Generate version information
    log_step "Generating version information"
    SVN_REVISION=$(get_svn_revision)
    update_version "${MAIN_VERSION}" "${SVN_REVISION}"
    create_version_header "${SVN_REVISION}"
    
    # Build exclusion list
    log_step "Building package exclusion list"
    build_exclusion_list
    
    # Run MakeRelease
    log_step "Running MakeRelease to build packages"
    run_makerelease
    
    log_success "Successfully built packages"
}

# Main execution
main() {
    log_header "LinuxMCE Package Build Process"
    
    # Parse command line arguments
    parse_arguments "$@"
    
    # Run main build function
    build_main_debs
    
    log_header "Build completed successfully"
}

main "$@"
```

### 7. Documentation and Helpers

Add proper documentation and help functionality:

```bash
# In help.sh
show_help() {
    cat <<EOF
LinuxMCE Build System ${VERSION}

Usage: ./build.sh [options]

Options:
  -a, --arch=ARCH        Build architecture (amd64, i386, armhf)
  -d, --distro=DISTRO    Distribution name (jammy, focal, etc)
  -c, --config=FILE      Use specific config file
  -j, --jobs=N           Number of parallel build jobs
  -h, --help             Show this help message
  -v, --verbose          Enable verbose output

Environment Variables:
  DB_HOST                Database host (default: localhost)
  DB_USER                Database user (default: root)
  DB_PASS                Database password
  BUILD_DIR              Base directory for build files
  
This build system requires a MySQL database with the LinuxMCE schema.
EOF
}
```

## Implementation Strategy

1. **Phased Approach**: Incrementally modernize while maintaining backward compatibility
   - Start with improved logging and documentation
   - Gradually introduce configuration files
   - Create wrapper functions for existing tools

2. **Testing Strategy**: Ensure each change doesn't break the build
   - Create test builds for each change
   - Compare results with the original system
   - Document any differences or improvements

3. **Documentation**: Well-documented code and processes
   - Each script should have a clear purpose
   - Each function should have clear inputs, outputs, and side effects
   - Document the overall build process

## Benefits of Modernization

1. **Increased Maintainability**: Easier to understand and modify
2. **Better Error Handling**: Clearer error messages and logging
3. **Improved Configurability**: More flexible without code changes
4. **Better Documentation**: Self-documenting code and dedicated documentation
5. **Reduced Technical Debt**: Fewer hacks and workarounds

## Compatibility Considerations

The modernized build system must maintain compatibility with:

1. The SQL database and its schema
2. The MakeRelease tool
3. Existing package naming and versioning
4. SVN version control system

## Next Steps

1. Document the current build system in detail
2. Create proof-of-concept implementations of key improvements
3. Develop a test plan to validate changes
4. Implement changes incrementally
5. Update documentation continuously