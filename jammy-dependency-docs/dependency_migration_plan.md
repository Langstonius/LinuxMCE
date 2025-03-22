# Dependency Migration Plan for Ubuntu Jammy

This document outlines a plan for migrating from deprecated packages that are no longer available in Ubuntu Jammy.

## Missing Dependency Usage Analysis

We analyzed the codebase to identify components that depend on packages no longer available in Ubuntu Jammy. Below are our findings and recommended migration paths.

### High Impact Dependencies

#### 1. libqt4-dev (144 references)

**Affected Components:**
- DLNA
- QML_Light_Switch
- qOrbiter
- YouTube_Player
- qMediaPlayer_Plugin
- Hue_Controller

**Migration Path:**
- Upgrade to Qt5 or Qt6
- Update include paths from `QtGui/QApplication` to `QtWidgets/QApplication`
- Replace `#include <QtCore/qglobal.h>` with `#include <QtCore/QtGlobal>`
- Update version checks (`QT_VERSION` macros) to handle Qt5/Qt6
- For Qt5: Use QtWidgets module for GUI components that were previously in QtGui

**Sample Code Changes:**
```cpp
// Old Qt4 code
#include <QtGui/QApplication>

// New Qt5+ code
#include <QtWidgets/QApplication>
```

#### 2. libgnomeui-dev (16 references)

**Affected Components:**
- mce-launcher

**Migration Path:**
- Replace with GTK3 or GTK4
- libgnomeui is deprecated and most applications have migrated to plain GTK
- Update build system to use pkg-config for GTK3 instead of GNOME UI
- Remove dependencies on other GNOME 2.x components (libgnome-2.0, libglade-2.0, etc.)

### Medium Impact Dependencies

#### 3. libqjson-dev (7 references)

**Migration Path:**
- Replace with Qt's built-in JSON support (available since Qt5)
- Use `QJsonDocument`, `QJsonObject`, and `QJsonArray` instead

#### 4. libgnome2-dev (6 references)

**Migration Path:**
- Similar to libgnomeui-dev, migrate to GTK3/GTK4
- Replace GNOME-specific APIs with GTK equivalents

### Lower Impact Dependencies

#### 5. libdancer-xml0-dev, libesd0-dev, libgnomevfs2-dev, libkonq5-dev, python3-oauth, python3-urlgrabber, libcec-platform-dev, dh-systemd (3-4 references each)

**Migration Recommendations:**
- **libdancer-xml0-dev**: Replace with another XML parser like libxml2
- **libesd0-dev**: Replace with PulseAudio or ALSA
- **libgnomevfs2-dev**: Replace with GIO/GVfs from GLib
- **libkonq5-dev**: Replace with Qt5/Qt6 file dialogs and browsing widgets
- **python3-oauth**: Replace with python3-requests-oauthlib
- **python3-urlgrabber**: Replace with python3-requests
- **libcec-platform-dev**: Investigate compatibility with libcec4-dev
- **dh-systemd**: This functionality is now part of debhelper; update debian/rules

## Migration Strategy

1. **Prioritize Qt4 Migration**:
   - Most critical due to number of affected components
   - Create a separate branch for Qt5 migration
   - Test each component individually after migration

2. **GTK/GNOME Migration**:
   - Address mce-launcher dependencies
   - Test with GTK3 replacements

3. **Python Dependencies**:
   - Update Python imports to use modern equivalents

4. **Build System Updates**:
   - Update CMakeLists.txt, configure scripts, and Makefiles
   - Replace pkg-config checks for deprecated packages

## Testing Plan

1. Create test builds for each affected component
2. Verify functionality after dependency changes
3. Address any compatibility issues
4. Create comprehensive test suite for affected components

## Timeline

1. **Phase 1**: Qt4 Migration (Highest Priority)
2. **Phase 2**: GTK/GNOME Migration
3. **Phase 3**: Python and Remaining Dependencies
4. **Phase 4**: Integration Testing
5. **Phase 5**: Documentation Updates