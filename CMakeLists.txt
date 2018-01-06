SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_C_COMPILER /opt/Xilinx/SDK/2017.3/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /opt/Xilinx/SDK/2017.3/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_FIND_ROOT_PATH /opt/Xilinx/SDK/2017.3/gnu/aarch32/lin/gcc-arm-linux-gnueabi)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

cmake_minimum_required (VERSION 2.8.4)
project (YaskawaCoE)

# The version number.
set (YaskawaCoE_VERSION_MAJOR 1)
set (YaskawaCoE_VERSION_MINOR 0)

if(WIN32)
  set(OS "win32")
elseif(UNIX)
  set(OS "linux")
endif()

# add include folder for finding *.h files
include_directories(SOEM/soem)
include_directories(SOEM/osal)
include_directories(SOEM/osal/${OS})
include_directories(SOEM/oshw/${OS})
include_directories(ycoe)

# build the SOEM driver
add_subdirectory(SOEM)

# build the executable
#include_directories(SOEM)
#link_directories(SOEM)
file(GLOB YCOE_SOURCES ycoe/*.c)
add_executable(yaskawaCoE 
  ${YCOE_SOURCES}
  main.c)

# link the object file with the soem library
target_link_libraries(yaskawaCoE soem)

# set install path
install(TARGETS yaskawaCoE DESTINATION bin)
