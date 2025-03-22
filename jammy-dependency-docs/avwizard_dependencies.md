# AVWizard Dependencies Analysis

This document outlines the dependencies found in the AVWizard component and their status in Ubuntu Jammy.

## Component Overview

AVWizard is a GUI application for configuring audio and video settings in LinuxMCE. It provides:

- A wizard-based interface for configuring display resolution and audio settings
- Audio testing capabilities (including Dolby and DTS tests)
- Video resolution and display size adjustment
- UI switcher configuration

The application uses SDL and OpenGL for rendering the user interface, and Xine for audio/video playback.

## Identified Dependencies

### System Libraries

1. **SDL Libraries**
   - **Status**: SDL 1.2 is not available in Ubuntu Jammy (SDL 2.0 is available)
   - **Used for**: Core graphical rendering and input handling
   - **Files**: SDLFrontEnd.h/cpp, main.cpp
   - **Migration Difficulty**: High - SDL 2.0 has significant API differences
   - **Notes**: Uses SDL_ttf, SDL_image, and SDL_gfx extensions

2. **OpenGL**
   - **Status**: Available in Ubuntu Jammy
   - **Used for**: Alternative rendering backend
   - **Files**: OpenGLFrontEnd.h/cpp
   - **Migration Difficulty**: Medium - May need updates for newer OpenGL versions

3. **Xine**
   - **Status**: Available in Ubuntu Jammy but deprecated
   - **Used for**: Audio/video playback for testing
   - **Files**: XinePlayer.h/cpp
   - **Migration Difficulty**: High - Xine is deprecated, may need to migrate to GStreamer/VLC

4. **X11/XRandR**
   - **Status**: Available in Ubuntu Jammy but with evolving API
   - **Used for**: Display resolution detection and configuration
   - **Files**: XRandrParser.cpp, XUtilities.cpp, ScreenManager.cpp
   - **Migration Difficulty**: Medium - XRandR API has evolved, may need updates

5. **XML Libraries**
   - **Status**: Available in Ubuntu Jammy
   - **Used for**: Configuration file parsing
   - **Files**: AVWizardConfParser.cpp, WizardConfigParser.cpp
   - **Migration Difficulty**: Low - libxml2 is still standard and available

6. **POSIX Threads**
   - **Status**: Available in Ubuntu Jammy
   - **Used for**: Thread management
   - **Files**: ThreadSleeper.h/cpp, SafetyLock.h/cpp
   - **Migration Difficulty**: Low - Standard library

### External Dependencies

1. **libresolution**
   - **Status**: Custom library, needs verification
   - **Used for**: Resolution handling
   - **Migration Difficulty**: Medium - Depends on compatibility with Jammy

### Internal Dependencies

1. **AMixerParser**
   - **Used for**: ALSA mixer parsing and configuration
   - **Files**: AMixerParser.h/cpp, AMixerOptions.h/cpp
   - **Migration Difficulty**: Medium - Depends on ALSA compatibility with Jammy

## Architecture and Component Structure

The AVWizard application uses a modular architecture with several key components:

1. **Frontend Rendering Engines**
   - SDLFrontEnd: SDL-based rendering
   - OpenGLFrontEnd: OpenGL-based rendering

2. **Backend Components**
   - XinePlayer: Audio/video playback
   - XRandrParser: Display resolution detection
   - ScreenManager: Display configuration 

3. **Wizard Framework**
   - WizardPage classes for different configuration steps
   - Widget system for UI elements

## Migration Challenges

### 1. SDL 1.2 to SDL 2.0 Migration

The application relies heavily on SDL 1.2 APIs which are significantly different in SDL 2.0:

```cpp
// In SDLFrontEnd.h
SDL_Surface* Screen;
SDL_Surface* Display;
// ...
void Blit(SDL_Surface* Surface, SDL_Rect SrcRect, SDL_Rect DestRect);
```

SDL 2.0 moves from a surface-based model to a texture-based renderer:

```cpp
// SDL 2.0 approach
SDL_Renderer* renderer;
SDL_Texture* texture;
// ...
SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
```

### 2. Xine Library Deprecation

Xine is increasingly deprecated in modern distributions:

```cpp
// In XinePlayer.h
xine_t *xine;
xine_stream_t *stream;
xine_audio_port_t *ao_port;
// ...
void InitPlayerEngine(std::string ConfigName, std::string FileName);
```

This would need to be replaced with a more modern media library like GStreamer or libVLC.

### 3. XRandR API Evolution

The XRandR API has evolved significantly, particularly with multi-monitor support:

```cpp
// In XRandrParser.cpp
system("xrandr -d :0 -q > /tmp/pluto_xrandr.txt");
```

This command-line parsing approach is fragile and needs replacing with direct XRandR API calls.

### 4. Wayland Compatibility

Ubuntu Jammy defaults to Wayland rather than X11:

```cpp
// In ScreenManager.cpp
// X11-specific code that will not work under Wayland
```

A Wayland-compatible approach would be needed for full support.

## Migration Recommendations

### Short-term Solutions

1. **Build SDL 1.2 From Source**:
   - Compile SDL 1.2 and dependencies from source
   - Create compatibility packages for Ubuntu Jammy
   - This is a temporary solution to get the software running

2. **Force X11 Session**:
   - Configure Ubuntu Jammy to use X11 instead of Wayland
   - Update XRandR parsing to handle newer XRandR output formats

3. **Replace Xine Command-Line Calls**:
   - Use direct API calls instead of parsing command-line output
   - Create abstraction layers to facilitate later migration

### Long-term Solutions

1. **Port to SDL 2.0**:
   - Rewrite SDLFrontEnd using SDL 2.0 renderer API
   - Update event handling for SDL 2.0
   - Adapt texture handling for UI elements

2. **Replace Xine with GStreamer**:
   - Create a new media player class using GStreamer
   - Implement the same interface for backward compatibility
   - Update audio testing functionality

3. **Implement Wayland Support**:
   - Create a Wayland-specific display manager
   - Use wlroots or similar library for display configuration
   - Make display backend pluggable to support both X11 and Wayland

## Testing Strategy

1. **Resolution Testing**:
   - Test resolution detection on various displays
   - Verify resolution switching works correctly
   - Test multi-monitor configurations

2. **Audio Testing**:
   - Test audio device detection
   - Verify audio format testing (Dolby, DTS)
   - Test volume adjustment

3. **UI Testing**:
   - Verify wizard flow works correctly
   - Test UI rendering on different resolutions
   - Verify input handling (keyboard, mouse)

## Additional Notes

The AVWizard is tightly coupled with X11 and SDL 1.2, both of which are increasingly outdated in modern Linux distributions. A significant rewrite may be required for full compatibility with Ubuntu Jammy, particularly if Wayland support is needed.

The heavy use of system command execution and parsing (like "xrandr -q") is particularly problematic, as command output formats can change between versions, making the software fragile. Moving to direct API calls would improve reliability.

For interim compatibility, running in X11 mode rather than Wayland would provide the path of least resistance, but a long-term solution should embrace Wayland and modern media libraries.