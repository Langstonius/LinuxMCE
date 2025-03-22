# DCERouter Dependencies Analysis

This document outlines the dependencies found in the DCERouter component and their status in Ubuntu Jammy.

## Identified Dependencies

### System Libraries

1. **libxml2**
   - **Status**: Available in Ubuntu Jammy (`libxml2-dev`)
   - **Used in**: Makefile, XML processing functionality
   - **Migration Difficulty**: Low - standard library with stable API

2. **libattr**
   - **Status**: Available in Ubuntu Jammy (`libattr1-dev`)
   - **Used in**: File attribute operations
   - **Migration Difficulty**: Low - standard library

3. **MySQL Client Library**
   - **Status**: Available in Ubuntu Jammy (`libmysqlclient-dev`)
   - **Used in**: Database connectivity
   - **Migration Difficulty**: Medium - API compatible but specific version dependencies
   - **Notes**: The debian/control file specifies dependencies on very old MySQL versions (libmysqlclient14, mysql-server-4.1)

4. **Python 2**
   - **Status**: Deprecated in Ubuntu Jammy (Python 2 is EOL)
   - **Used in**: discovery.py for network discovery
   - **Migration Difficulty**: Low - simple script that can easily be updated to Python 3

5. **libpthread**
   - **Status**: Available in Ubuntu Jammy (part of glibc)
   - **Used in**: Threading functionality
   - **Migration Difficulty**: Low - standard library

### Internal/Custom Dependencies

1. **libDCECommon**
2. **libSerializeClass**
3. **libPlutoUtils**
4. **libpluto_main**

### Debian Package Dependencies

The debian/control file lists numerous dependencies that may be problematic:

1. **Deprecated MySQL packages**: 
   - `libmysqlclient14`, `mysql-common-4.1`, `mysql-server-4.1`
   - These are very old versions not available in modern Ubuntu

2. **Outdated SSL libraries**:
   - `libssl0.9.7`, `libssl0.9.8`
   - Modern Ubuntu uses newer SSL versions

3. **Outdated C++ libraries**:
   - `libstdc++5` (along with `libstdc++6`)
   - Only `libstdc++6` is available in Jammy

4. **Various Pluto-specific packages**:
   - Many of these would need to be built from source or updated for Jammy

## Migration Challenges

### 1. MySQL Dependency Upgrades

The DCERouter has dependencies on very old MySQL client libraries and server versions. Key migration challenges include:

- **API Compatibility**: The code may use deprecated MySQL API functions
- **Configuration Changes**: MySQL configuration formats have changed
- **Authentication**: Modern MySQL uses different authentication methods

**Migration Strategy**:
1. Update code to use current MySQL C API
2. Test with MariaDB (the default in Ubuntu)
3. Update connection parameters for newer authentication methods

### 2. Python 2 to Python 3 Migration

The discovery.py script uses Python 2 syntax:

```python
print "From addr: '%s', msg: '%s'" % (addr[0], data)
```

This needs to be updated to Python 3 syntax:

```python
print("From addr: '{}', msg: '{}'".format(addr[0], data))
```

### 3. SSL Library Updates

The code may have dependencies on specific OpenSSL API functions that have been deprecated in newer versions.

**Migration Strategy**:
1. Identify SSL function calls in the codebase
2. Update to current OpenSSL API
3. Test SSL functionality

### 4. C++11 Compatibility

The Makefile specifies the `-std=c++11` flag, which is positive for migration. However, older C++ code may still use patterns or features that have changed in modern C++ versions.

## Detailed Dependency Review

### Direct Code Dependencies

The code directly includes and uses:
- libxml2 (for XML processing)
- libattr (for file attributes)
- MySQL client libraries
- pthread (for threading)
- Standard C++ libraries

### Package Dependencies

The debian/control file lists many dependencies that are problematic for modern Ubuntu:
1. Specific versions of MySQL (4.1 series)
2. Old SSL libraries
3. Multiple custom Pluto packages

## Migration Recommendations

### Short-term Solutions

1. **MySQL Client Update**:
   - Modify code to use current MySQL/MariaDB C API
   - Update connection parameters
   - Test with MariaDB server

2. **Python Script Update**:
   - Convert discovery.py to Python 3 syntax
   - Test network discovery functionality

3. **Build System Update**:
   - Update dependency checks in Makefile
   - Remove hard-coded paths to outdated libraries

### Long-term Solutions

1. **Package Dependency Cleanup**:
   - Review and update the debian/control file
   - Remove dependencies on specific versions where possible
   - Replace custom packages with standard alternatives where available

2. **Code Modernization**:
   - Take advantage of C++11 features already enabled
   - Update deprecated API calls
   - Improve error handling for newer library versions

## Testing Strategy

1. **Database Connectivity Testing**:
   - Test with both MySQL and MariaDB
   - Verify connection with different authentication methods

2. **Network Discovery Testing**:
   - Test updated Python 3 discovery script
   - Verify proper UDP socket communication

3. **Build System Testing**:
   - Verify that all dependencies can be satisfied in Jammy
   - Test build process with updated libraries

## Additional Notes

The DCERouter component has significant dependencies on older MySQL versions and custom Pluto packages. The migration effort should focus on updating database connectivity code and ensuring compatibility with modern library versions.