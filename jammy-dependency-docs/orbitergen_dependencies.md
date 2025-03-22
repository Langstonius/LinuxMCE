# OrbiterGen Dependencies Analysis

This document outlines the dependencies found in the OrbiterGen component and their status in Ubuntu Jammy.

## Identified Dependencies

### SDL and Graphics Libraries

1. **SDL 1.2**
   - **Status**: Not available in Ubuntu Jammy (only SDL 2.0 is available)
   - **Used in**: Makefile, Renderer.cpp, SDLRendererOCGHelper.cpp
   - **Migration Difficulty**: High - SDL 2.0 has API differences from SDL 1.2
   - **Notes**: OrbiterGen heavily relies on SDL 1.2 APIs that have changed in SDL 2.0

2. **SDL_image 1.2**
   - **Status**: Not available in Ubuntu Jammy (only SDL_image 2.0 is available)
   - **Used in**: Makefile, Renderer.cpp
   - **Migration Difficulty**: High - API changes between versions

3. **SDL_ttf 1.2**
   - **Status**: Not available in Ubuntu Jammy (only SDL_ttf 2.0 is available)
   - **Used in**: Makefile, Renderer.cpp
   - **Migration Difficulty**: High - API changes between versions

4. **SDL_gfx**
   - **Status**: Available in Ubuntu Jammy, but built against SDL 2.0
   - **Used in**: Makefile, Renderer.cpp (SDL_rotozoom.h)
   - **Migration Difficulty**: High - API changes and compatibility issues

5. **SGE (SDL Graphics Extension)**
   - **Status**: Not available in Ubuntu Jammy repositories
   - **Used in**: Makefile, Renderer.cpp
   - **Migration Difficulty**: Very High - likely needs to be replaced or rebuilt

6. **libpng**
   - **Status**: Available in Ubuntu Jammy
   - **Used in**: Renderer.cpp
   - **Migration Difficulty**: Low - API compatible

### System Libraries

1. **MySQL Client Library**
   - **Status**: Available in Ubuntu Jammy (`libmysqlclient-dev`)
   - **Used in**: Makefile, database connectivity
   - **Migration Difficulty**: Low - API mostly compatible

2. **libGL**
   - **Status**: Available in Ubuntu Jammy
   - **Used in**: Makefile for OpenGL support
   - **Migration Difficulty**: Low - standard library

3. **libXrender**
   - **Status**: Available in Ubuntu Jammy
   - **Used in**: Makefile for X11 rendering
   - **Migration Difficulty**: Low - standard library

4. **libz (zlib)**
   - **Status**: Available in Ubuntu Jammy
   - **Used in**: Makefile for compression
   - **Migration Difficulty**: Low - standard library

5. **libpthread**
   - **Status**: Available in Ubuntu Jammy (part of glibc)
   - **Used in**: Makefile for threading
   - **Migration Difficulty**: Low - standard library

### Internal/Custom Dependencies

1. **libDCECommon**
2. **libPlutoUtils**
3. **libSerializeClass**
4. **libpluto_main** 
5. **libpluto_media**
6. **libSDL_Helpers** (custom helper library for SDL)

## SDL 1.2 to SDL 2.0 Migration Challenges

OrbiterGen heavily depends on SDL 1.2 and associated libraries that are not available in Ubuntu Jammy. The primary challenges include:

### 1. Initialization API Changes

```c++
// SDL 1.2 (current code)
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE)

// SDL 2.0 (required for Jammy)
SDL_Init(SDL_INIT_VIDEO)
// SDL_INIT_NOPARACHUTE is removed in SDL 2.0
```

### 2. Surface and Texture Handling

SDL 2.0 uses a renderer and texture-based approach, whereas SDL 1.2 uses surfaces directly:

```c++
// SDL 1.2 approach (current)
SDL_Surface *surface = SDL_CreateRGBSurface(...)
// Direct pixel manipulation

// SDL 2.0 approach (required)
SDL_Texture *texture = SDL_CreateTexture(renderer, ...)
// Manipulate via renderer
```

### 3. Custom Extensions

OrbiterGen uses SGE (SDL Graphics Extension) which has no direct equivalent in SDL 2.0. This requires:
- Finding alternative libraries
- Rewriting graphics code that depends on SGE functions

### 4. Font Rendering Changes

SDL_ttf has API changes between 1.2 and 2.0 that affect text rendering:

```c++
// SDL_ttf 1.2 (current)
TTF_RenderText_Blended(...)

// SDL_ttf 2.0 (required)
// Different handling for textures vs surfaces
```

## Migration Recommendations

### Short-term Solutions

1. **Build SDL 1.2 and Dependencies from Source**:
   - Compile SDL 1.2, SDL_image 1.2, SDL_ttf 1.2, SDL_gfx, and SGE from source
   - Create compatibility packages that can coexist with SDL 2.0
   - Ensure compatibility with modern OpenGL and X11 libraries

2. **Containerization**:
   - Run OrbiterGen in a container with Ubuntu 16.04 or earlier that has SDL 1.2
   - Use volume mounts to share data between host and container

### Long-term Solutions

1. **Port to SDL 2.0**:
   - Rewrite rendering code to use SDL 2.0 API
   - Replace SGE functionality with SDL 2.0 primitives or alternative libraries
   - Update surface manipulation to texture-based approach
   - Requires significant code refactoring

2. **Alternative Rendering Engine**:
   - Consider replacing SDL with a more modern rendering engine
   - Options include Qt, SFML, or direct OpenGL
   - Would require a major rewrite but provide better long-term maintainability

## Specific Code Modification Needs

1. **SDL Initialization**:
```c++
// Current code in Renderer.cpp
if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)

// Needed change
if (SDL_Init(SDL_INIT_VIDEO) == -1)
```

2. **Surface Handling**:
```c++
// Review and update all code using SDL_Surface* 
// and direct pixel manipulation
```

3. **Text Rendering**:
```c++
// Review and update all TTF_RenderText_* calls
```

4. **SGE Functions**:
```c++
// Find alternatives for all sge_* function calls
```

## Testing Strategy

1. **Component Testing**:
   - Test each rendering component separately
   - Create test patterns to verify rendering accuracy
   - Compare output with previous versions

2. **Integration Testing**:
   - Test OrbiterGen with full skin generation process
   - Verify all generated UI elements appear correctly

3. **Performance Testing**:
   - Measure rendering performance with SDL 2.0
   - Optimize code for modern graphics hardware

## Additional Notes

OrbiterGen's dependency on SDL 1.2 and associated libraries presents the most significant migration challenge. The code's tight coupling with these libraries will require substantial rewriting for compatibility with Ubuntu Jammy.

The use of core system libraries like MySQL, OpenGL, and zlib should not pose significant issues, as these have maintained API compatibility.