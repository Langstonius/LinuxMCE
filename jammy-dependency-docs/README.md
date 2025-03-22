# LinuxMCE Jammy Dependency Migration

This directory contains documentation and tools for migrating LinuxMCE build dependencies to Ubuntu Jammy (22.04 LTS).

## Summary

We've identified 13 packages that need replacement or removal from the original build-packages list. The transition primarily involves:

- Qt4 → Qt5 migration
- GNOME2 → GTK3 migration
- Several deprecated libraries that have modern replacements
- A few packages that need further investigation

## Files

### Documentation

- `missing_packages.txt` - Complete list of packages not available in Jammy
- `replacement_suggestions.txt` - Detailed search results for alternative packages
- `package_migration.md` - Table of missing packages with recommended replacements
- `updated_build_packages` - Updated build-packages file with replacements and comments
- `dependency_status.md` - Executive summary with statistics and next steps
- `comprehensive_migration_guide.md` - Complete guide for migration process
- `dependency_migration_plan.md` - Strategic plan for dependency transition
- `dcerouter_dependencies.md` - Analysis of DCERouter component dependencies
- `lmce_launch_manager_dependencies.md` - Analysis of Launch Manager dependencies
- `orbiter_dependencies.md` - Analysis of Orbiter component dependencies
- `orbitergen_dependencies.md` - Analysis of OrbiterGen component dependencies
- `generic_serial_device_dependencies.md` - Analysis of serial device dependencies

### Scripts

- `check_packages.sh` - Identifies which packages are missing in Jammy
- `find_replacements.sh` - Searches for potential replacement packages
- `find_dependency_usage.sh` - Analyzes how missing packages are used in the codebase
- `identify_replacements.sh` - Helps identify appropriate replacement packages
- `verify_check.sh` - Validates package coverage statistics
- `final_validation.sh` - Ensures all missing packages are properly documented

## Usage

1. Review `dependency_status.md` for the high-level overview
2. Examine `package_migration.md` for the specific package replacements
3. Use `updated_build_packages` as a template for updating the main build-packages file

## Next Steps

1. Create PR to update the `build-packages` file with the suggested replacements
2. Investigate code that depends on packages without direct replacements
3. Test builds with the updated dependency list

## License

Same as the main LinuxMCE project