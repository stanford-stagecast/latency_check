cmake_minimum_required ( VERSION 2.8.5 )
cmake_policy ( SET CMP0054 NEW )
project ( Latency-check )

include ( FindJPEG )

find_package ( JPEG REQUIRED )
find_package ( OpenGL REQUIRED )
find_package ( GLEW REQUIRED )
find_package ( glfw3 REQUIRED )


include ( etc/build_defs.cmake )
include ( etc/build_type.cmake )
include ( etc/cflags.cmake )

include ( etc/clang_format.cmake )
include ( etc/clang_tidy.cmake )

file ( GLOB CHECKER_SRC
    src/util/*
    #src/display/*
    src/video/*
)

include_directories ( src )
include_directories ("${PROJECT_SOURCE_DIR}/src/util")

add_library ( checker STATIC
    ${CHECKER_SRC}
)

set ( CHECKER_LIBS
    checker
    glfw
    GLEW
    GL
    GLU
    jpeg
    pthread
)

add_executable ( real-webcam src/frontend/real-webcam.cc )
target_link_libraries ( real-webcam ${CHECKER_LIBS} )
