cmake_minimum_required(VERSION 3.16)

project(chrono
    VERSION 0.0.2
    DESCRIPTION "A transparent floating time display for Windows"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES
    "src/*.c"
)

file(GLOB_RECURSE HEADERS
    "include/*.h"
)

set(ALL_FILES ${SOURCES} ${HEADERS})

add_executable(chrono WIN32 ${ALL_FILES})

string(REPLACE "/MD" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "/MDd" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "/MT" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "/MTd" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(REPLACE "/MD" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    string(REPLACE "/MDd" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    string(REPLACE "/MT" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    string(REPLACE "/MTd" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
endforeach()

target_compile_options(chrono PRIVATE
    /W4 # Warning level 4
    /GS- # Disable buffer security checks
    /Gy # Enable function-level linking
    /Gm- # Disable minimal rebuild
    /Zc:wchar_t # wchar_t is native type
    /Zc:inline # Remove unreferenced COMDAT
    /fp:precise # Floating point model
)

target_link_libraries(chrono
    PRIVATE
    user32
    gdi32
    kernel32
    dwmapi
)

target_link_options(chrono PRIVATE
    /SUBSYSTEM:WINDOWS # Windows subsystem
    /NODEFAULTLIB # Don't link default libraries (including CRT)
    /ENTRY:WinMain # Entry point (or use WinMainCRTStartup for WinMain)

    # /OPT:REF # Eliminate unreferenced functions/data
    # /OPT:ICF # Enable COMDAT folding
)

# # Linker options for no-CRT build
# target_link_options(chrono PRIVATE
# /SUBSYSTEM:WINDOWS # Windows subsystem
# /NODEFAULTLIB # Don't link default libraries (including CRT)
# /ENTRY:WinMain # Entry point (or use WinMainCRTStartup for WinMain)
# /OPT:REF # Eliminate unreferenced functions/data
# /OPT:ICF # Enable COMDAT folding
# )

# Configuration-specific compile options
target_compile_options(chrono PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /D_DEBUG>
    $<$<CONFIG:Release>:/O2 /DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG>
    $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG>
)

# set_target_properties(chrono PROPERTIES
# RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )

# target_include_directories(chrono PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# install(TARGETS chrono
# RUNTIME DESTINATION bin
# COMPONENT applications
# )

# set(CPACK_PACKAGE_NAME "chrono")
# set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
# set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
# set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Transparent floating time display")
# set(CPACK_PACKAGE_VENDOR "chrono Project")
# set(CPACK_PACKAGE_INSTALL_DIRECTORY "chrono")

# include(CPack)

# message(STATUS "chrono Configuration:")
# message(STATUS "  Version: ${PROJECT_VERSION}")
# message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
# message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
# message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin")
# message(STATUS "  CRT: Disabled (No-CRT build)")