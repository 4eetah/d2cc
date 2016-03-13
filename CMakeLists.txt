cmake_minimum_required(VERSION 3.0)
project(d2cc)


# some checks are probably not needed here
include(CheckIncludeFiles)
check_include_files("sys/time.h;time.h" TIME_WITH_SYS_TIME)
check_include_files("sys/time.h"        HAVE_SYS_TIME_H)
check_include_files("sys/select.h"      HAVE_SYS_SELECT_H)
check_include_files("sys/sysctl.h"      HAVE_SYS_SYSCTL_H)
check_include_files("sys/param.h"       HAVE_SYS_PARAM_H)
check_include_files("poll.h"            HAVE_POLL_H)
check_include_files("sys/event.h"       HAVE_SYS_EVENT_H)
check_include_files("strings.h"         HAVE_STRINGS_H)
check_include_files("sys/ioctl.h"       HAVE_SYS_IOCTL_H)
check_include_files("sys/filio.h"       HAVE_SYS_FILIO_H)
check_include_files("sys/sockio.h"      HAVE_SYS_SOCKIO_H)
check_include_files("pthread.h"         HAVE_PTHREAD_H)
check_include_files("net/if_dl.h"       HAVE_NET_IF_DL_H)
check_include_files("netinet/sctp.h"    HAVE_NETINET_SCTP_H)
configure_file(${CMAKE_SOURCE_DIR}/config.h.in
               ${CMAKE_BINARY_DIR}/config.h
               )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")           

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${SRC_DIR})
include_directories(${TOOLCHAIN_DIR})

set(D2CCD_SRC
    ${SRC_DIR}/daemon.cpp
    ${SRC_DIR}/main.cpp
)
set(WRAP_SRC
    ${SRC_DIR}/wrapsocket.cpp
    ${SRC_DIR}/wrapunix.cpp
    ${SRC_DIR}/wraperror.cpp
)
set(TOOLCHAIN_SRC
    ${SRC_DIR}/compiler.cpp
)

add_executable(d2ccd ${D2CCD_SRC}
                     ${WRAP_SRC}
)

add_executable(compiler ${TOOLCHAIN_SRC}
                        ${WRAP_SRC}
)

set(COMPILER ${CMAKE_BINARY_DIR}/compiler)
add_custom_command(TARGET compiler POST_BUILD
    COMMAND ln -sf ${COMPILER} d2cc-g++
    COMMAND ln -sf ${COMPILER} d2cc-gcc
    COMMAND ln -sf ${COMPILER} d2cc-clang
    COMMAND ln -sf ${COMPILER} d2cc-clang++
)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(LIBEVENT REQUIRED libevent)
#target_link_libraries(d2cc ${LIBEVENT_LIBRARIES})
