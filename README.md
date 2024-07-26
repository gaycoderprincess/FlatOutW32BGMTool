# FlatOut W32 Tool

A tool to handle .w32 files in FlatOut games, highly WIP

This tool is currently capable of:
- Parsing and recreating the format from memory
- Converting FlatOut 2 tracks to FlatOut 1
- Exporting tracks to .fbx
- Editing track meshes
- Deleting track meshes
- Moving track props
- Deleting track props
- Creating a dummy track_bvh.gen

Thanks to Gulbroz for their existing work on this format, it was a great jumping-off point.

## Usage

- Enter a commandline prompt, run `FlatOut2W32Tool_gcp.exe (filename)` with the arguments you'd like to use
- The file will now be analyzed and processed by the tool
- Enjoy, nya~ :3

## Arguments

- `-export_fbx` - Exports the map into a viewable .fbx file
- `-export_w32` - Exports the map into a .w32 file (this should yield an identical file if there are no additional arguments, if it doesn't then file a bug report!)
- `-export_text` - Exports the map into a human-readable text file
- `-text_streams` - Exports all vertex and index buffers into text, huge filesize and time cost!
- `-text_materials` - Exports all material data into text, including shaders and some unknown metadata
- `-text_streams_fouc_offseted` - Exports vertex buffers into text with their surface offsets already applied
- `-remove_object_dummies` - Creates a new .w32 file of the map with all objects and object dummies removed (e.g. menu cameras in FOUC)
- `-remove_props` - Creates a new .w32 file of the map with all props removed
- `-enable_all_props` - Creates a new .w32 file of the map with all props visible (BugBear left a lot of props hidden in each track)
- `-convert_to_fo1` - Creates a new .w32 file of the map that can be loaded by FlatOut 1
- `-empty_bvh_gen` - Takes a track_bvh.gen file and generates a new one without any culling, required for custom maps to not have issues!
- `-import_moved_props` - Imports moved prop positions from an .fbx file, usage: `FlatOutW32Tool_gcp.exe (w32 filename) (fbx filename) -import_moved_props`
- `-ungroup_moved_props` - Ungroups moved props if `-import_moved_props` is enabled, can prevent unwanted physics behavior
- `-import_surfaces` - Imports surface meshes from an .fbx file if they have the suffix `_export` in their name, usage: `FlatOutW32Tool_gcp.exe (w32 filename) (fbx filename) -import_surfaces`
- `-import_materials` - Imports material changes and additions if `-import_surfaces` is enabled, might introduce issues
- `-import_deletions` - Deletes surfaces and props that have been deleted from an .fbx file, usage: `FlatOutW32Tool_gcp.exe (w32 filename) (fbx filename) -import_deletions`

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
