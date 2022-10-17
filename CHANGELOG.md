# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.3] - 2022-10-17
### Added
- Horizontal tab cycles focus among textfields.
- When a new window is added to screen set focus on it if the window is a
textfield.
- When setting focus on a window store the focused window and draw the
previously focused window. 
- Added api to ncui::Window to mark it dirty.

### Removed
- Remove redundant code.

## [0.1.2] - 2022-10-06
### Added
- Use exit_curses to exit the program which will free memory allocated by
ncurses if the library is configured to support memory-leak checking.

### Fixed
- Fix double free while destroying windows in end_screen.

## [0.1.1] - 2022-09-30
### Changed
- Use smart pointers for pimpl.
- Enforce pimpl idiom.
- Add format strings to mvprintw and mvwprintw.

### Removed
- Remove source files from prerequisits of objects.

### Fixed
- Fix behavior when cursor reaches end of textfield.

## [0.1.0] - 2022-08-23
### Added
- Screen class to manage ncurses screen.
- Window class to manage windows.
- Windows can be bordered.
- Windows can be text fields.
- Event handlers for mouse and keyboard input can be attached.
