cmake_minimum_required (VERSION 2.8.4)
project (YaskawaCoE)

# The version number.
set (YaskawaCoE_VERSION_MAJOR 1)
set (YaskawaCoE_VERSION_MINOR 0)

if(WIN32)
  set(OS "win32")
  include_directories(SOEM/oshw/win32/wpcap/Include)
  link_directories(${CMAKE_SOURCE_DIR}/SOEM/oshw/win32/wpcap/Lib)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /WX")
  set(OS_LIBS wpcap.lib Packet.lib Ws2_32.lib Winmm.lib)
elseif(UNIX)
  set(OS "linux")

  #xenomai cobalt flags
  #set Xenomai Search Paths
  set (XENOMAI_SEARCH_PATH /usr/local/xenomai /usr/xenomai /usr/include/xenomai $ENV{XENOMAI_ROOT_DIR})

  #find xeno-config.h
  find_path ( XENOMAI_DIR
    NAMES include/xeno_config.h xeno_config.h
    PATHS ${XENOMAI_SEARCH_PATH} )

  # did we find xeno_config.h?
  if ( XENOMAI_DIR )
    message (STATUS "xenomai found: \"${XENOMAI_DIR}\"")
    set(XENOMAI_XENO_CONFIG /usr/xenomai/bin/xeno-config)
    execute_process(COMMAND ${XENOMAI_XENO_CONFIG} --posix --ldflags OUTPUT_VARIABLE XENOMAI_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${XENOMAI_XENO_CONFIG} --posix --cflags OUTPUT_VARIABLE XENOMAI_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("CFLAGS ${XENOMAI_CFLAGS}")
    message("LDFLAGS ${XENOMAI_LDFLAGS}")
    set(CMAKE_C_FLAGS ${XENOMAI_CFLAGS})
    set(CMAKE_EXE_LINKER_FLAGS ${XENOMAI_LDFLAGS})
  endif()
  #xenomai cobalt flags
endif()

# add include folder for finding *.h files
include_directories(SOEM/soem)
include_directories(SOEM/osal)
include_directories(SOEM/osal/${OS})
include_directories(SOEM/oshw/${OS})
include_directories(libzmq/include)
include_directories(ycoe)
include_directories(.)

# build the SOEM driver and zeromq driver
add_subdirectory(SOEM)
add_subdirectory(libzmq)

#build the executable foreach main file
file(GLOB YCOE_SOURCES ycoe/*.c)
add_library(ycoe STATIC ${YCOE_SOURCES})
# link the object file with the soem and libzmq library
if(UNIX)
  target_link_libraries(ycoe m)
  if (XENOMAI_DIR)
    #target_link_libraries(ycoe rt)
    target_link_libraries(ycoe alchemy)
    target_link_libraries(ycoe copperplate)
    #target_link_libraries(ycoe cobalt)
  endif()
endif()
target_link_libraries(ycoe soem)
target_link_libraries(ycoe libzmq)

file(GLOB MAIN_FILES main/*.c)
foreach (file ${MAIN_FILES})
  get_filename_component(filename ${file} NAME)
  string( REPLACE ".c" "" appname ${filename} )
  message("ycoe_${appname}")

  # build the executable
  add_executable(ycoe_${appname} ${file})

  # link the object file with the ycoe library
  target_link_libraries(ycoe_${appname} ycoe)

  # set install path
  install(TARGETS ycoe_${appname} DESTINATION bin)
endforeach()
