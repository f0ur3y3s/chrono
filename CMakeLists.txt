cmake_minimum_required(VERSION 3.16)

project(chrono
    VERSION 1.0.0
    DESCRIPTION "A transparent floating time display for Windows"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "chrono is a Windows-only application")
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES
    "src/*.c"
)

file(GLOB_RECURSE HEADERS
    "include/*.h"
)

set(ALL_FILES ${SOURCES} ${HEADERS})

add_executable(chrono WIN32 ${ALL_FILES})

target_link_libraries(chrono
    PRIVATE
    user32
    gdi32
    kernel32
)

if(NOT MSVC)
    message(FATAL_ERROR "chrono can only be built with MSVC")
endif()

target_compile_options(chrono PRIVATE /W4)
set_target_properties(chrono PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

set_target_properties(chrono PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
string(REPLACE "/MDd" "/MD" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set_target_properties(chrono PROPERTIES
    COMPILE_FLAGS_DEBUG "/Od /Zi /D_DEBUG"
    COMPILE_FLAGS_RELEASE "/O2 /DNDEBUG"
    COMPILE_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG"
    COMPILE_FLAGS_MINSIZEREL "/Os /DNDEBUG"
)

target_include_directories(chrono PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

install(TARGETS chrono
    RUNTIME DESTINATION bin
    COMPONENT applications
)

set(CPACK_PACKAGE_NAME "chrono")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Transparent floating time display")
set(CPACK_PACKAGE_VENDOR "chrono Project")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "chrono")

include(CPack)

message(STATUS "chrono Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin")