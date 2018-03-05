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

# build the SOEM driver and zeromq driver
add_subdirectory(SOEM)
add_subdirectory(libzmq)

# build the executable
#include_directories(SOEM)
#link_directories(SOEM)
file(GLOB YCOE_SOURCES ycoe/*.c)
add_executable(yaskawaCoE
  ${YCOE_SOURCES}
  main.c)

# link the object file with the soem and libzmq library
target_link_libraries(yaskawaCoE m)
target_link_libraries(yaskawaCoE soem)
target_link_libraries(yaskawaCoE libzmq)

# set install path
install(TARGETS yaskawaCoE DESTINATION bin)
