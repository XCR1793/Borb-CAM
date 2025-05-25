cmake_minimum_required(VERSION 3.16)
project(main LANGUAGES C CXX)

# ========== Config ==========
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ========== Raylib Config ==========
# Override with: -DRAYLIB_PATH=...
set(RAYLIB_PATH "C:/raylib/raylib" CACHE PATH "Path to raylib source")

# ========== Source Files ==========
file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    src/*.cpp
)

# ========== Include Directories ==========
# Recursively include all src/ folders
file(GLOB_RECURSE ALL_INCLUDE_DIRS
    CONFIGURE_DEPENDS
    src/*
)

foreach(dir ${ALL_INCLUDE_DIRS})
    if(IS_DIRECTORY ${dir})
        include_directories(${dir})
    endif()
endforeach()

include_directories(
    src
    ${RAYLIB_PATH}/src
    ${RAYLIB_PATH}/src/external
)

# ========== Executable ==========
add_executable(${PROJECT_NAME} ${SRC_FILES})

# ========== Platform-Specific Linking ==========
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ${RAYLIB_PATH}/src/raylib.rc.data
        ${RAYLIB_PATH}/src/libraylib.a
        opengl32 gdi32 winmm
    )
else()
    target_link_libraries(${PROJECT_NAME}
        ${RAYLIB_PATH}/src/libraylib.a
        GL m pthread dl rt X11
    )
endif()

# ========== Definitions ==========
target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_DESKTOP)
