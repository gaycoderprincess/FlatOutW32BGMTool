# FlatOut 2 W32 Tool

A tool to handle .w32 files in FlatOut 2, highly WIP

This tool is currently only capable of parsing and recreating the format from memory, and converting FlatOut 2 tracks to FlatOut 1.

Thanks to Gulbroz for their existing work on this format, it was a great jumping-off point.

## Usage

- Enter a commandline prompt, run `FlatOut2W32Extractor_gcp.exe (filename)`
- The file will now be analyzed, dumped into a text file and then converted to the FlatOut 1 format with the extension `.w32_out.w32`
- Enjoy, nya~ :3

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Required packages: `mingw-w64-gcc`
