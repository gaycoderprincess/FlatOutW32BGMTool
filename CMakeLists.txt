cmake_minimum_required(VERSION 3.27)
project(FlatOut2W32BGMTool)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

link_directories(${CMAKE_SOURCE_DIR}/../nya-common/lib32)

add_executable(FlatOut2W32BGMTool main.cpp)
target_include_directories(FlatOut2W32BGMTool PUBLIC ../nya-common)
target_include_directories(FlatOut2W32BGMTool PUBLIC ../nya-common/3rdparty)
set_target_properties(FlatOut2W32BGMTool PROPERTIES SUFFIX "_gcp.exe")

target_link_libraries(FlatOut2W32BGMTool assimp)
target_link_libraries(FlatOut2W32BGMTool zlib)
target_link_libraries(FlatOut2W32BGMTool pugixml)
target_link_libraries(FlatOut2W32BGMTool polyclipping)
target_link_libraries(FlatOut2W32BGMTool minizip)
target_link_libraries(FlatOut2W32BGMTool kubazip)
target_link_libraries(FlatOut2W32BGMTool poly2tri)
target_link_libraries(FlatOut2W32BGMTool draco)