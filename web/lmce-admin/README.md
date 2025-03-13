# LinuxMCE Admin Web Interface

## PHP 7+ Compatibility Improvements

This document outlines the work done and remaining tasks needed to make the LinuxMCE admin web interface compatible with PHP 7+ environments.

### Completed Updates

1. **MySQL Functions (✅ UPDATED)**
   - Replaced deprecated `mysql_*` functions with `mysqli_*` equivalents
   - Updated files:
     - `setEa2.php`
     - `operations/mediaBrowser/tvdbbatch.php` 
     - `operations/mediaBrowser/checkTMDB.php`
     - `operations/mediaBrowser/editMediaTagsRecursive.php`
     - `operations/mediaBrowser/internet_radio.php`
     - `cdrviewer/include/plugins/au_callrates.inc.php`
     - `operations/myDevices/orbiters/setupWebOrbiter.php`
     - `operations/mediaBrowser/httpImageProvider.php`
     - `include/utils.inc.php`

2. **Regular Expression Functions (✅ UPDATED)**
   - Replaced deprecated `ereg/eregi` functions with `preg_match` equivalents
   - Updated files:
     - `include/utils.inc.php`
     - `viewCamera.php`
     - `operations/webfilter_proxy/proxy.php`
     - `operations/rooms/rooms.php`
     - `operations/security/outsideAccess.php`
     - `operations/others/backup.php`
     - `operations/mediaBrowser/mainMediaFilesSync.php`
     - `operations/mediaBrowser/grabAmazonAttributes.php`
     - `operations/mediaBrowser/coverArt.php`
     - `operations/mediaBrowser/editDirectoryAttributes.php`
     - `operations/mediaBrowser/editMediaFile.php`
     - `operations/events/editTimedEvent.php`
     - `mediaDirector.php`
     - `check.wml`

3. **Each Function (✅ UPDATED)**
   - Replaced deprecated `each()` with `foreach` or key/current/next functions
   - Updated files:
     - `operations/datalog/viewDatalog.php`
     - `operations/myDevices/mythSettings.php`
     - `include/vcard.inc.php`

### Remaining PHP 7+ Compatibility Issues

#### High Priority

1. **Create_function() Usage**
   - **Issue**: Deprecated in PHP 7.2, removed in PHP 8.0
   - **Fix**: Replace with anonymous functions (closures)
   - **Main locations**: 
     - adodb drivers (e.g., `include/adodb/drivers/adodb-sqlite.inc.php`)
     - adodb main files

2. **Session Handling Functions**
   - **Issue**: `session_register()`, `session_unregister()`, and `session_is_registered()` removed in PHP 7.0
   - **Fix**: Replace with direct `$_SESSION` array manipulation
   - **Main locations**: 
     - Older session-related code
     - `include/adodb/session/` legacy files

3. **Variable Handling with Indirect References**
   - **Issue**: Behavior of variable variables (`$$var`) changed in PHP 7
   - **Fix**: Refactor code to avoid variable variables or update syntax
   - **Main locations**:
     - `weborbiter/IT.php`
     - `operations/telecom/voicemail.php`

#### Medium Priority

4. **Mcrypt Functions**
   - **Issue**: `mcrypt_*` functions deprecated in PHP 7.1, removed in PHP 7.2
   - **Fix**: Replace with OpenSSL functions
   - **Main locations**:
     - `include/adodb/session/adodb-encrypt-mcrypt.php`

5. **Deprecated String Functions**
   - **Issue**: `split()` function removed, should use `explode()` or `preg_split()`
   - **Fix**: Replace with modern alternatives
   - **Main locations**: Throughout codebase

6. **Reserved Keywords as Variable Names**
   - **Issue**: PHP 7 introduced new reserved keywords
   - **Fix**: Rename variables using reserved keywords
   - **Main locations**: Requires manual review

#### Lower Priority

7. **Pass-by-reference Behavior Changes**
   - **Issue**: PHP 7 changed how pass-by-reference works
   - **Fix**: Update code that passes temporary expressions by reference
   - **Main locations**: Requires manual review

8. **Error Handling Changes**
   - **Issue**: Error handling changed significantly in PHP 7
   - **Fix**: Update error handling code
   - **Main locations**: Throughout codebase

9. **Integer Handling**
   - **Issue**: PHP 7 changed how integer overflow is handled
   - **Fix**: Review code relying on specific overflow behavior
   - **Main locations**: Mathematical calculations

10. **JSON Handling Changes**
    - **Issue**: `json_decode()` behavior changed in PHP 7
    - **Fix**: Add proper error checking for JSON operations
    - **Main locations**: API interactions, JSON data processing

## Implementation Notes

When updating code for PHP 7+ compatibility:

1. Test thoroughly after each change
2. Be careful with library code, especially adodb
3. Check for side effects when updating function calls
4. Consider using PHP linting tools to identify issues
5. Remember that PHP 7+ is more strict about type handling

## Future Considerations

- Consider upgrading the adodb library to a more recent version
- Add automated tests to verify PHP 7+ compatibility
- Document any application-specific workarounds
- Consider using PHP 7+ features for new code to improve performance

## References

- [PHP 7.0 Migration Guide](https://www.php.net/manual/en/migration70.php)
- [PHP 7.1 Migration Guide](https://www.php.net/manual/en/migration71.php)
- [PHP 7.2 Migration Guide](https://www.php.net/manual/en/migration72.php)
- [PHP 7.3 Migration Guide](https://www.php.net/manual/en/migration73.php)
- [PHP 7.4 Migration Guide](https://www.php.net/manual/en/migration74.php)