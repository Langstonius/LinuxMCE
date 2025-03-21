# LinuxMCE Package Migration for Ubuntu Jammy

## Missing Packages and Suggested Replacements

Below is a list of packages from the original `build-packages` file that are no longer available in Ubuntu Jammy (22.04), along with suggested replacements.

| Missing Package | Suggested Replacement | Notes |
|----------------|------------------------|-------|
| libdancer-xml0-dev | N/A | No direct replacement found. May need to use an alternative XML processing library. |
| libesd0-dev | libpulse-dev | EsounD has been deprecated in favor of PulseAudio. |
| libgnome2-dev | libgtk-3-dev or libgtk-4-dev | GNOME 2 libraries are obsolete. Modern applications use GTK3/GTK4 directly. |
| libgnomeui-dev | libgtk-3-dev or libgtk-4-dev | GNOME 2 UI libraries are obsolete. Use GTK3/GTK4 directly. |
| libgnomevfs2-dev | libgio-dev (part of libglib2.0-dev) | GnomeVFS has been replaced by GIO/GVFS in modern GNOME. |
| libkonq5-dev | libkf5konq-dev | KDE4 libraries have been replaced with KDE Frameworks 5 (KF5). |
| libqt4-dev | qtbase5-dev or qt6-base-dev | Qt4 is obsolete. Use Qt5 or Qt6 instead. |
| python3-oauth | python3-oauthlib or python3-oauth2client | Depends on the specific OAuth functionality needed. |
| python3-mysqldb | python3-mysqldb | Available in Jammy despite initial check failure. |
| python3-urlgrabber | python3-requests | Modern Python applications use the requests library for HTTP operations. |
| libqjson-dev | N/A | Qt5 includes JSON support natively, use Qt5's JSON classes instead. |
| libcec-platform-dev | N/A | May need to check if functionality is available in another package or build from source. |
| dh-systemd | debhelper (>= 9.20160709) | dh-systemd functionality is now included in the debhelper package. |

## Action Plan

1. Update the `build-packages` file to replace obsolete packages with their modern equivalents.
2. For packages without direct replacements:
   - Investigate the code that depends on these packages
   - Determine if the functionality can be provided by alternative libraries
   - Consider updating the code to use modern alternatives

3. Test builds with the updated package list to identify any additional dependencies.

## Additional Notes

- The migration from Qt4 to Qt5/Qt6 may require code changes beyond just package replacements.
- GNOME 2 to GTK3/GTK4 migration similarly requires application code updates.
- Some functionality may need to be reimplemented using different APIs if direct replacements are not available.