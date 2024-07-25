cmake_minimum_required(VERSION 3.27)
project(FlatOut2W32Tool)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

link_directories(${CMAKE_SOURCE_DIR}/../nya-common/lib32)

add_executable(FlatOut2W32Tool main.cpp)
target_include_directories(FlatOut2W32Tool PUBLIC ../nya-common)
target_include_directories(FlatOut2W32Tool PUBLIC ../nya-common/3rdparty)
set_target_properties(FlatOut2W32Tool PROPERTIES SUFFIX "_gcp.exe")

target_link_libraries(FlatOut2W32Tool assimp)
target_link_libraries(FlatOut2W32Tool zlib)
target_link_libraries(FlatOut2W32Tool pugixml)
target_link_libraries(FlatOut2W32Tool polyclipping)
target_link_libraries(FlatOut2W32Tool minizip)
target_link_libraries(FlatOut2W32Tool kubazip)
target_link_libraries(FlatOut2W32Tool poly2tri)
target_link_libraries(FlatOut2W32Tool draco)