# Inspired by https://gitlab.tune-it.ru/jackalope/dict-it/-/blob/dev/semnet/CMakeLists.txt


cmake_minimum_required(VERSION 3.5)
include(commons.cmake)

project(jackalope_storage)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-std=c++17)

set(STATIC_BUILD TRUE)
set(CONAN_SYSTEM_INCLUDES ON)

if (STATIC_BUILD)
    set(CONAN_OPTIONS "${CONAN_OPTIONS};boost:shared=False")
    set(PACKAGE_OPTIONS "STATIC;FPIC")
else()
    set(CONAN_OPTIONS "${CONAN_OPTIONS};boost:shared=True")
    add_definitions(-DBOOST_LOG_DYN_LINK)
endif()


if (PRODUCTION_PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O2")
    set(CMAKE_BUILD_TYPE Release)
    message("Build is in production mode.")
elseif (OPTIMIZATION_PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3")
    set(CMAKE_BUILD_TYPE Release)
    message("Build is in optimization mode. Be careful!")
else()
    message("Build is in debug mode.")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO,"true")
    set(CMAKE_CXX_FLAGS_DEBUG,"true")
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb3 -Wall -Wextra -pedantic")
    find_file (FEDORA_FOUND fedora-release
            PATHS /etc
            )
    if (FEDORA_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
    endif (FEDORA_FOUND)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt  CMAKE_TARGETS  BUILD boost BUILD missing OPTIONS ${CONAN_OPTIONS})
conan_basic_setup()

#external things
set(lib lib)
set(src src)
set(include include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/semql)

link_directories(${lib})
include_directories(${include})
include_directories(${src})

list(APPEND PACKAGE_OPTIONS ${CONAN_LIBS})

add_package("example" ${PACKAGE_OPTIONS})

enable_testing()
add_subdirectory(tests tests)