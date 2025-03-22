# LMCE_Launch_Manager Dependencies Analysis

This document outlines the dependencies found in the LMCE_Launch_Manager code and their status in Ubuntu Jammy.

## Identified Dependencies

### System Libraries

1. **X11/Xrandr**
   - **Status**: Available in Ubuntu Jammy (`libxrandr-dev`)
   - **Used in**: LM.h, LM.cpp for screen resolution management
   - **Migration Difficulty**: Low - standard library with stable API

2. **MySQL Client Library**
   - **Status**: Available in Ubuntu Jammy (`libmysqlclient-dev`)
   - **Used in**: LM.h, DB.cpp, Makefile
   - **Migration Difficulty**: Low - API compatible

3. **X11 Libraries**
   - **Status**: Available in Ubuntu Jammy (`libx11-dev`, `libxext-dev`)
   - **Used in**: Makefile, LM.cpp
   - **Migration Difficulty**: Low - standard libraries

4. **Python with xorgconfig module**
   - **Status**: Python is available, but `xorgconfig` is not a standard module
   - **Used in**: GetVideoDriver.py
   - **Migration Difficulty**: Medium - need to identify the source of xorgconfig module

### Internal/Custom Dependencies

1. **libSerializeClass**
2. **libDCECommon**
3. **libPlutoUtils**
4. **libpthread**

## Python Dependency Concerns

The `GetVideoDriver.py` script uses a non-standard `xorgconfig` module that may not be available in Ubuntu Jammy. This script is used to detect the video driver configuration from xorg.conf.

```python
import sys,xorgconfig

config = xorgconfig.readConfig("/etc/X11/xorg.conf")
```

This presents a migration challenge because:
1. Modern Ubuntu systems may use Wayland instead of X11
2. The xorgconfig module appears to be custom-made
3. The structure of xorg.conf may have changed in newer versions

## X11/Xrandr Usage Analysis

The component heavily relies on X11 and Xrandr for:
1. Detecting display configurations
2. Getting/setting screen resolutions
3. Managing the display environment

While these libraries are available in Ubuntu Jammy, the usage patterns might need updates if the system uses Wayland as the display server instead of X11.

## Migration Recommendations

### Short-term Solutions

1. **MySQL Client Update**:
   - Update Makefile to ensure it uses the correct MySQL client library version
   - Test DB connection code for compatibility

2. **Python Script Replacement**:
   - Create a replacement for GetVideoDriver.py that uses modern methods to detect video drivers
   - Consider using libdrm or other modern APIs instead of parsing xorg.conf

3. **X11/Xrandr Compatibility**:
   - Test X11/Xrandr functionality in Ubuntu Jammy
   - Ensure that display detection works with modern display servers

### Long-term Solutions

1. **Wayland Support**:
   - Add support for Wayland as an alternative to X11
   - Implement detection to use appropriate display server APIs

2. **Modernize Screen Resolution Management**:
   - Replace direct Xrandr calls with a more abstract interface
   - Consider using libdrm or other modern APIs that work across X11 and Wayland

## Testing Strategy

1. **Display Server Testing**:
   - Test on both X11 and Wayland sessions
   - Verify resolution changes work on both display servers

2. **Database Testing**:
   - Test MySQL connection and query functionality
   - Verify compatibility with MariaDB (the default in Ubuntu)

3. **Python Script Testing**:
   - Develop and test replacement for GetVideoDriver.py
   - Ensure it works on systems with and without X11

## Additional Notes

The LMCE_Launch_Manager has dependencies on X11 that may be problematic in future Ubuntu releases where Wayland becomes the only display server option. A long-term strategy should include abstraction of display server specific code to support both X11 and Wayland.