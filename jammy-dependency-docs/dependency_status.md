# LinuxMCE Dependency Status for Ubuntu Jammy

## Summary

We've analyzed the build dependencies listed in `../src/Ubuntu_Helpers_NoHardcode/conf-files/jammy-amd64/build-packages` for compatibility with Ubuntu Jammy (22.04 LTS). 

**Results:**
- Total packages checked: 229
- Available in Jammy: 214 (93.4%)
- Not available/requiring replacement: 15 (6.6%)

## Migration Plan

1. **Direct replacements** - Some packages have clear modern alternatives:
   - Qt4 → Qt5 migration: `libqt4-dev` → `qtbase5-dev`
   - GNOME 2 → GTK3 migration: `libgnome2-dev`, `libgnomeui-dev` → `libgtk-3-dev`
   - KDE4 → KF5 migration: `libkonq5-dev` → `libkf5konq-dev`
   - Sound system: `libesd0-dev` → `libpulse-dev`
   - Python modules: `python3-oauth` → `python3-oauthlib`, `python3-urlgrabber` → `python3-requests`
   - Build tools: `dh-systemd` functionality moved into `debhelper`

2. **Packages needing further investigation**:
   - `libdancer-xml0-dev` - XML processing library, needs alternative
   - `libdvb-dev` - DVB development library, check current DVB libraries
   - `libcrystalhd-dev` - Broadcom Crystal HD hardware decoder, may no longer be supported
   - `libcec-platform-dev` - CEC platform development, may need to build from source
   - `libqjson-dev` - JSON for Qt, functionality now in Qt5 core
   - `gnome-applets` - GNOME 2 applets, likely obsolete

## Next Steps

1. Create a PR to update the `build-packages` file with the replacements identified in `updated_build_packages`

2. For packages without direct replacements, analyze code that depends on them:
   ```
   # Example commands to trace dependencies
   grep -r "dancer-xml" --include="*.h" --include="*.cpp" ../src/
   grep -r "dvb/" --include="*.h" --include="*.cpp" ../src/
   ```

3. Update application code where needed to use modern libraries instead of deprecated ones

4. Test builds with the updated dependency list

## Files Created

1. `missing_packages.txt` - List of packages missing in Jammy
2. `replacement_suggestions.txt` - Detailed search for potential replacements
3. `package_migration.md` - Table of missing packages and suggested replacements
4. `updated_build_packages` - Updated build-packages file with replacements and comments
5. `dependency_status.md` - This summary report