# FlatOut 2 W32 Tool

A tool to handle .w32 files in FlatOut 2, highly WIP

This tool is currently only capable of parsing and recreating the format from memory, and converting FlatOut 2 tracks to FlatOut 1.

Thanks to Gulbroz for their existing work on this format, it was a great jumping-off point.

## Usage

- Enter a commandline prompt, run `FlatOut2W32Extractor_gcp.exe (filename)` with the arguments you'd like to use
- The file will now be analyzed and processed by the tool
- Enjoy, nya~ :3

## Arguments

- `-export_fbx` - Exports the map into a viewable .fbx file
- `-export_w32` - Exports the map into a .w32 file (this should yield an identical file with no other arguments, if it doesn't then file a bug report!)
- `-export_text` - Exports the map into a human-readable text file
- `-export_streams_into_text` - If `-export_text` is enabled, also exports all vertex and index buffers into text, huge filesize and time cost!
- `-remove_props` - Creates a new .w32 file of the map with all props removed
- `-convert_to_fo1` - Creates a new .w32 file of the map that can be loaded by FlatOut 1

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process.

Required packages: `mingw-w64-gcc vcpkg`

To install all dependencies, use:
```console
vcpkg install assimp:x86-mingw-static
```

Once installed, copy files from `~/.vcpkg/vcpkg/installed/x86-mingw-static/`:

- `include` dir to `nya-common/3rdparty`
- `lib` dir to `nya-common/lib32`

You should be able to build the project now in CLion.
