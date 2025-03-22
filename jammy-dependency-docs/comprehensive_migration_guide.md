# Comprehensive Migration Guide for Ubuntu Jammy

This guide details the steps required to migrate LinuxMCE components that depend on packages no longer available in Ubuntu Jammy (22.04).

## Source Code Dependencies Overview

Our analysis of the codebase has identified several components that rely on missing packages:

| Missing Package | Affected Components | Impact Level | References |
|----------------|---------------------|--------------|------------|
| libqt4-dev | DLNA, QML_Light_Switch, qOrbiter, YouTube_Player, qMediaPlayer_Plugin, Hue_Controller | High | 144 |
| libgnomeui-dev | mce-launcher | Medium | 16 |
| libqjson-dev | Various Qt components | Medium | 7 |
| libgnome2-dev | mce-launcher | Medium | 6 |
| libgnomevfs2-dev | mce-launcher | Low | 4 |
| libkonq5-dev | KDE-related components | Low | 4 |
| Other missing packages | Various components | Low | 3-4 each |

## Migration Strategies by Package

### 1. libqt4-dev (High Priority)

**Replacement:** qtbase5-dev + related Qt5 modules

**Required Code Changes:**
- Update include paths:
  ```cpp
  // Old
  #include <QtGui/QApplication>
  #include <QtCore/qglobal.h>
  
  // New
  #include <QtWidgets/QApplication>
  #include <QtCore/QtGlobal>
  ```

- Update build system:
  ```cmake
  # Old
  find_package(Qt4 REQUIRED)
  
  # New
  find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
  ```

- Update Qt version checks:
  ```cpp
  // Old
  #if QT_VERSION < 0x040800
  
  // New
  #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  ```

- Replace deprecated classes:
  - QWidget-based classes moved from QtGui to QtWidgets
  - Some Qt4 classes have been renamed or split in Qt5

**Affected Files:**
- DLNA/*.cpp, DLNA/*.h
- QML_Light_Switch/**/*.cpp, QML_Light_Switch/**/*.h
- qOrbiter/**/*.cpp, qOrbiter/**/*.h
- And others (see dependency_usage_results/libqt4-dev_usage.txt for complete list)

### 2. GNOME Libraries (libgnome2-dev, libgnomeui-dev, libgnomevfs2-dev)

**Replacement:** libgtk-3-dev, libglib2.0-dev, libglib2.0-dev-bin

**Required Code Changes:**
- Replace GNOME-specific APIs with GTK equivalents
- Update build systems:
  ```
  # Old
  pkg-config --cflags "libgnome-2.0 libgnomeui-2.0 libglade-2.0 vte gthread-2.0"
  
  # New
  pkg-config --cflags "gtk+-3.0 glib-2.0 vte-2.91 gthread-2.0"
  ```

- Replace GNOME VFS file access with GIO/GVfs
- Update UI components to use GTK3 widgets

**Affected Files:**
- mce-launcher/configure
- mce-launcher/Makefile
- mce-launcher/src/Makefile

### 3. libqjson-dev

**Replacement:** Use Qt5's native JSON support

**Required Code Changes:**
- Replace QJson classes with Qt5's JSON classes:
  ```cpp
  // Old (QJson)
  #include <qjson/parser.h>
  QJson::Parser parser;
  
  // New (Qt5)
  #include <QJsonDocument>
  #include <QJsonObject>
  QJsonDocument doc = QJsonDocument::fromJson(data);
  QJsonObject obj = doc.object();
  ```

### 4. libkonq5-dev

**Replacement:** libkf5konq-dev

**Required Code Changes:**
- Update include paths to KF5 equivalents
- Update class usage for KF5 compatibility
- Update build system to use KF5 components

### 5. Python Packages (python3-oauth, python3-urlgrabber)

**Replacement:** python3-requests-oauthlib, python3-requests

**Required Code Changes:**
- Update import statements:
  ```python
  # Old
  import urlgrabber
  
  # New
  import requests
  
  # Old
  import oauth
  
  # New
  from requests_oauthlib import OAuth1
  ```

- Update API calls to match the new libraries

### 6. Other Dependencies

- **libdancer-xml0-dev**: Replace with libxml2-dev or another XML processing library
- **libesd0-dev**: Replace with libpulse-dev for audio functionality
- **libcec-platform-dev**: Investigate if functionality is available in libcec-dev
- **dh-systemd**: Update debian/rules to use debhelper >= 9.20160709 instead

## Migration Process

### Phase 1: Qt4 to Qt5 Migration (Highest Priority)

1. Create a dedicated branch for Qt migration
2. For each component:
   - Update include paths
   - Update build system (CMake, qmake, or Makefiles)
   - Fix compilation errors related to API changes
   - Test functionality
3. Complete checklist for each component:
   - [ ] Build succeeds
   - [ ] Basic functionality works
   - [ ] Integration with other components works

### Phase 2: GNOME/GTK Migration

1. Focus on mce-launcher component
2. Update build system to use GTK3 instead of GNOME libraries
3. Replace GNOME-specific API calls with GTK/GIO equivalents
4. Test functionality

### Phase 3: Other Dependencies

1. Address each remaining dependency based on its usage
2. Prioritize by number of references and importance of the component

## Testing Strategy

1. Create a test build environment with Ubuntu Jammy
2. For each migrated component:
   - Unit tests where available
   - Manual functionality tests
   - Integration tests with other components

## Reference Resources

- [Qt4 to Qt5 Migration Guide](https://doc.qt.io/qt-5/sourcebreaks.html)
- [GTK3 Migration Guide](https://developer.gnome.org/gtk3/stable/gtk-migrating-2-to-3.html)
- [Python Requests Documentation](https://docs.python-requests.org/)
- [Requests-OAuthlib Documentation](https://requests-oauthlib.readthedocs.io/)

## Appendix: Detailed Replacement Candidates

See `replacement_candidates.md` for a comprehensive list of replacement packages available in Ubuntu Jammy.