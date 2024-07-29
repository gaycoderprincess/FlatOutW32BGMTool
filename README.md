# FlatOut W32 & BGM Tool

A tool to handle .w32 and .bgm files in FlatOut games

This tool is currently capable of:
- Parsing and recreating the formats from memory
- Importing vehicle models into FlatOut 1, 2 and Ultimate Carnage
- Converting FlatOut 2 tracks to FlatOut 1
- Converting FlatOut: Ultimate Carnage cars to FlatOut 1 and 2
- Exporting cars and tracks to .fbx
- Editing track meshes
- Deleting track meshes
- Moving track props
- Deleting track props
- Creating a dummy track_bvh.gen

Thanks to Gulbroz for their existing work on this format, it was a great jumping-off point.

![custom car mesh preview](https://i.imgur.com/Liwqm6v.png)
![custom track mesh preview](https://i.imgur.com/lpQsDOl.png)

## Usage

- Enter a commandline prompt, run `FlatOutW32BGMTool_gcp.exe (filename)` with the arguments you'd like to use
- The file will now be analyzed and processed by the tool
- Enjoy, nya~ :3

## W32 Arguments

- `-export_fbx` - Exports the map into a viewable .fbx file
- `-export_w32` - Exports the map into a .w32 file (this should yield an identical file if there are no additional arguments, if it doesn't then file a bug report!)
- `-export_text` - Exports the map into a human-readable text file
- `-text_streams` - Exports all vertex and index buffers into text, huge filesize and time cost!
- `-text_materials` - Exports all material data into text, including shaders and some unknown metadata
- `-text_streams_fouc_offseted` - Exports vertex buffers into text with their surface offsets already applied
- `-text_streams_fouc_normalized` - Exports vertex buffers into text as normalized floating points
- `-text_streams_fouc_int8` - Exports vertex buffers into text as int8 arrays
- `-remove_object_dummies` - Creates a new .w32 file of the map with all objects and object dummies removed (e.g. menu cameras in FOUC)
- `-remove_props` - Creates a new .w32 file of the map with all props removed
- `-enable_all_props` - Creates a new .w32 file of the map with all props visible (BugBear left a lot of props hidden in each track)
- `-convert_to_fo1` - Creates a new .w32 file of the map that can be loaded by FlatOut 1
- `-empty_bvh_gen` - Takes a track_bvh.gen file and generates a new one without any culling, required for custom maps to not have issues!
- `-import_moved_props` - Imports moved prop positions from an .fbx file, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_moved_props`
- `-ungroup_moved_props` - Ungroups moved props if `-import_moved_props` is enabled, can prevent unwanted physics behavior
- `-import_cloned_props` - Imports new cloned props from an .fbx file, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_cloned_props`
- `-import_surfaces` - Imports surface meshes from an .fbx file if they have the suffix `_export` in their name, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_surfaces`
- `-import_all_surfaces` - Imports all surface meshes from an .fbx file, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_all_surfaces`
- `-import_and_match_all_surfaces` - Imports all surface meshes from an .fbx file and matches them up to any valid w32 surface, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_and_match_all_surfaces`
- `-clear_old_materials` - Removes all original materials from the w32 before importing the ones from the .fbx, has no effect unless `-import_and_match_all_surfaces` is enabled
- `-import_deletions` - Deletes surfaces and props that have been deleted from an .fbx file, usage: `FlatOutW32BGMTool_gcp.exe (w32 filename) (fbx filename) -import_deletions`

## BGM Arguments
- `-create_fo1_bgm` - Exports an .fbx into a FlatOut 1 car bgm
- `-create_fo2_bgm` - Exports an .fbx into a FlatOut 2 car bgm
- `-create_fouc_bgm` - Exports an .fbx into a FlatOut: Ultimate Carnage car bgm
- `-export_fbx` - Exports the car into a viewable .fbx file
- `-export_bgm` - Exports car into a .bgm file (this should yield an identical file if there are no additional arguments, if it doesn't then file a bug report!)
- `-export_text` - Exports the car into a human-readable text file
- `-text_streams` - Exports all vertex and index buffers into text, huge filesize and time cost!
- `-text_materials` - Exports all material data into text, including shaders and some unknown metadata
- `-text_streams_fouc_offseted` - Exports vertex buffers into text with their surface offsets already applied
- `-text_streams_fouc_normalized` - Exports vertex buffers into text as normalized floating points
- `-text_streams_fouc_int8` - Exports vertex buffers into text as int8 arrays
- `-convert_to_fo1` - Converts the car from the FlatOut 2 or Ultimate Carnage format to the FlatOut 1 format
- `-convert_to_fo2` - Converts the car from the Ultimate Carnage format to the FlatOut 2 format

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
