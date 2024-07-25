cmake_minimum_required(VERSION 3.27)
project(FlatOut2W32Extractor)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

link_directories(${CMAKE_SOURCE_DIR}/../nya-common/lib32)

add_executable(FlatOut2W32Extractor main.cpp)
target_include_directories(FlatOut2W32Extractor PUBLIC ../nya-common)
target_include_directories(FlatOut2W32Extractor PUBLIC ../nya-common/3rdparty)
set_target_properties(FlatOut2W32Extractor PROPERTIES SUFFIX "_gcp.exe")

target_link_libraries(FlatOut2W32Extractor assimp)
target_link_libraries(FlatOut2W32Extractor zlib)
target_link_libraries(FlatOut2W32Extractor pugixml)
target_link_libraries(FlatOut2W32Extractor polyclipping)
target_link_libraries(FlatOut2W32Extractor minizip)
target_link_libraries(FlatOut2W32Extractor kubazip)
target_link_libraries(FlatOut2W32Extractor poly2tri)
target_link_libraries(FlatOut2W32Extractor draco)