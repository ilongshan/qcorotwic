cmake_minimum_required(VERSION 3.14)
set(qcorotwic_VERSION 1.0)
project(qcorotwic LANGUAGES CXX VERSION ${qcorotwic_VERSION})

include (ExternalProject)

#-----------------------------------------------------------#
# Compiler Settings
#-----------------------------------------------------------#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts -stdlib=libc++")
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not currently suported.")
endif()

#-----------------------------------------------------------#
# Dependencies
#-----------------------------------------------------------#

set(REQUIRED_QT_COMPONENTS Core Gui Quick Network)
if (BUILD_TESTING)
    list(APPEND REQUIRED_QT_COMPONENTS Test)
endif()

if (NOT USE_QT_VERSION)
    # FIXME: find_package(QT NAMES Qt6 Qt5 ...) seems to prefer Qt5 on my system
    find_package(Qt6 COMPONENTS ${REQUIRED_QT_COMPONENTS})
    if (NOT Qt6_FOUND)
        find_package(Qt5 COMPONENTS ${REQUIRED_QT_COMPONENTS} REQUIRED)
        set(QT_VERSION_MAJOR 5)
    else()
        set(QT_VERSION_MAJOR 6)
    endif()
else()
    find_package(Qt${USE_QT_VERSION} COMPONENTS ${REQUIRED_QT_COMPONENTS} REQUIRED)
    set(QT_VERSION_MAJOR ${USE_QT_VERSION})
endif()


set(QCoro_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qcoro)


#-----------------------------------------------------------#
# Sources
#-----------------------------------------------------------#

add_subdirectory(qcorotwic)

