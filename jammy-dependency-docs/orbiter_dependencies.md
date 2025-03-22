# Orbiter Dependencies Analysis

This document outlines the dependencies found in the Orbiter component and their status in Ubuntu Jammy.

## Identified Dependencies

### SDL and Graphics Libraries

1. **SDL 1.2**
   - **Status**: Not available in Ubuntu Jammy (only SDL 2.0 is available)
   - **Used in**: SDL implementation of the Orbiter renderer
   - **Migration Difficulty**: High - SDL 2.0 has significant API differences
   - **Notes**: Orbiter has specific surface manipulation code that would need rewriting

2. **X11 Libraries**
   - **Status**: Available in Ubuntu Jammy but with potential API changes
   - **Used in**: Linux implementation of the Orbiter renderer
   - **Files**: OrbiterRenderer_Linux.cpp, X3DWindow.cpp, XPromptUser.cpp
   - **Migration Difficulty**: Medium - X11 APIs have some changes in newer versions

3. **GTK 2.x**
   - **Status**: Deprecated in Ubuntu Jammy (GTK 3.x or 4.x is preferred)
   - **Used in**: GTKPromptUser.cpp, GTKProgressWnd.cpp
   - **Migration Difficulty**: High - Significant API differences between GTK 2.x and 3.x/4.x

4. **OpenGL**
   - **Status**: Available in Ubuntu Jammy but with potential compatibility issues
   - **Used in**: Optional OpenGL rendering mode
   - **Migration Difficulty**: Medium - May need updates for newer OpenGL versions

### System Libraries

1. **XTest Extension**
   - **Status**: Available in Ubuntu Jammy
   - **Used in**: XRecordExtensionHandler.cpp
   - **Migration Difficulty**: Low - Similar API

2. **libpthread**
   - **Status**: Available in Ubuntu Jammy (part of glibc)
   - **Used in**: Threading functionality
   - **Migration Difficulty**: Low - Standard library

### Internal/Custom Dependencies

1. **libDCECommon**
2. **libPlutoUtils**
3. **libSerializeClass**
4. **libSDL_Helpers** (custom helper library for SDL)

## Architecture and Component Structure

The Orbiter has multiple rendering implementations:

1. **SDL Renderer**: Uses SDL 1.2 for cross-platform rendering
2. **Linux Renderer**: Uses X11 directly for rendering on Linux
3. **OpenGL Renderer**: Optional OpenGL-based rendering

These rendering paths have different dependencies and migration challenges.

## Migration Challenges

### 1. SDL 1.2 to SDL 2.0 Migration

The Orbiter's SDL implementation relies heavily on SDL 1.2 surface manipulation that is fundamentally different in SDL 2.0:

```cpp
// SDL 1.2 code (current)
SDL_Surface *pSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, iWidth, iHeigth, 16, 31 << 0, 31 << 5, 31 << 10, 0);
// Direct pixel manipulation with putpixel()

// SDL 2.0 approach (required)
SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, iWidth, iHeight);
// Need to lock/unlock texture for pixel manipulation
```

### 2. GTK 2.x to GTK 3.x/4.x Migration

The Orbiter uses GTK 2.x for UI dialogs, which has significant API differences from GTK 3.x/4.x:

```cpp
// GTK 2.x code (current)
gtk_window_set_wmclass(GTK_WINDOW(pWindow), m_wndName.c_str(), m_wndName.c_str());

// GTK 3.x approach (required)
// gtk_window_set_wmclass is deprecated
// Need to use GdkWindow properties instead
```

### 3. X11 Compatibility Issues

The Orbiter's X11 code includes low-level X11 manipulation that may need updates:

```cpp
// Current code using X11 mask operations
pOrbiterLinux->m_pX11->Delete_Pixmap(m_screenMaskObjects);
```

### 4. Wayland Compatibility

Ubuntu Jammy can use Wayland instead of X11, which would require a complete redesign of the X11-specific code.

## Migration Recommendations

### Short-term Solutions

1. **Build SDL 1.2 from Source**:
   - Compile SDL 1.2 and dependencies from source
   - Create compatibility packages for Ubuntu Jammy
   - Ensure compatibility with modern OpenGL and X11 libraries

2. **X11 Compatibility Layer**:
   - Test existing X11 code with Jammy's X11 libraries
   - Implement compatibility functions for deprecated APIs

3. **Force X11 Session**:
   - Configure Ubuntu Jammy to use X11 instead of Wayland
   - Avoid Wayland compatibility issues in the short term

### Long-term Solutions

1. **Port to SDL 2.0**:
   - Rewrite renderer code to use SDL 2.0 API
   - Update surface manipulation to texture-based approach
   - Requires significant refactoring

2. **Update GTK Interface**:
   - Migrate GTK 2.x code to GTK 3.x or 4.x
   - Update dialog interfaces and callbacks

3. **Wayland Support**:
   - Implement Wayland-specific rendering path
   - Consider libwayland or other modern display server libraries

## Specific Code Changes Required

### SDL 2.0 Migration

1. **Initialization**:
```cpp
// Current
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE)

// SDL 2.0
SDL_Init(SDL_INIT_VIDEO)
// SDL_INIT_NOPARACHUTE is no longer supported
```

2. **Surface Handling**:
```cpp
// Current
SDL_Surface *pSurface = SDL_CreateRGBSurface(...)
putpixel(pSurface, x, y, pixel)

// SDL 2.0
SDL_Texture *texture = SDL_CreateTexture(...)
// Need to use SDL_LockTexture/SDL_UnlockTexture for pixel manipulation
```

### GTK 3.x Migration

1. **GtkBuilder API Changes**:
```cpp
// Current
gtk_builder_add_from_file(pBuilder, "/usr/pluto/share/GTKOrbiter.glade", NULL)

// GTK 3.x
// May need error handling changes and different widget creation patterns
```

2. **Window Properties**:
```cpp
// Current
gtk_window_set_wmclass(GTK_WINDOW(pWindow), m_wndName.c_str(), m_wndName.c_str())

// GTK 3.x
// Use GdkWindow properties or other GTK 3.x APIs
```

## Testing Strategy

1. **Rendering Path Testing**:
   - Test each rendering implementation separately
   - Compare output with previous versions
   - Verify UI element positioning and appearance

2. **Input Handling**:
   - Test mouse and keyboard input through various rendering paths
   - Verify touch input on compatible devices

3. **Dialog Testing**:
   - Test GTK dialogs and prompts
   - Verify compatibility with both X11 and Wayland sessions

## Additional Notes

The Orbiter component has a complex architecture with multiple rendering paths, which provides flexibility but increases migration complexity. The SDL path is likely the easiest to maintain long-term, but requires significant updates for SDL 2.0.

The X11-specific code may become increasingly challenging to maintain as Linux distributions move toward Wayland. A long-term strategy should include Wayland compatibility or reliance on cross-platform libraries that abstract the display server.