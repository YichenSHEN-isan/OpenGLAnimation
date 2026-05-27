cmake_minimum_required(VERSION 3.20)

project(OpenGLAniScene
    VERSION 1.0.0
    DESCRIPTION "Custom 3D animation pipeline implemented in C++ and OpenGL"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(OpenGLAniScene
    src/main.cpp
)

target_include_directories(OpenGLAniScene
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
)

find_package(OpenGL REQUIRED)

target_link_libraries(OpenGLAniScene PRIVATE OpenGL::GL)

if(TARGET OpenGL::GLU)
    target_link_libraries(OpenGLAniScene PRIVATE OpenGL::GLU)
elseif(OPENGL_glu_LIBRARY)
    target_link_libraries(OpenGLAniScene PRIVATE "${OPENGL_glu_LIBRARY}")
elseif(WIN32)
    target_link_libraries(OpenGLAniScene PRIVATE glu32)
else()
    message(FATAL_ERROR "GLU was not found. Install the GLU development package for your platform.")
endif()

option(OPENGL_ANISCENE_USE_BUNDLED_FREEGLUT
    "Use the bundled Windows FreeGLUT import library and DLL"
    ${WIN32}
)

if(WIN32 AND OPENGL_ANISCENE_USE_BUNDLED_FREEGLUT)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "The bundled FreeGLUT import library is 32-bit. Configure with "
            "-A Win32, or install a 64-bit FreeGLUT package and set "
            "OPENGL_ANISCENE_USE_BUNDLED_FREEGLUT=OFF."
        )
    endif()

    add_library(freeglut::freeglut SHARED IMPORTED)
    set_target_properties(freeglut::freeglut PROPERTIES
        IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/third_party/freeglut/lib/freeglut.lib"
        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/third_party/freeglut/bin/freeglut.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include"
    )
    target_compile_definitions(OpenGLAniScene PRIVATE FREEGLUT_LIB_PRAGMAS=0)
    target_link_libraries(OpenGLAniScene PRIVATE freeglut::freeglut)

    add_custom_command(TARGET OpenGLAniScene POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${PROJECT_SOURCE_DIR}/third_party/freeglut/bin/freeglut.dll"
            "$<TARGET_FILE_DIR:OpenGLAniScene>"
        VERBATIM
    )
else()
    find_package(GLUT REQUIRED)

    if(TARGET GLUT::GLUT)
        target_link_libraries(OpenGLAniScene PRIVATE GLUT::GLUT)
    else()
        target_include_directories(OpenGLAniScene PRIVATE ${GLUT_INCLUDE_DIR})
        target_link_libraries(OpenGLAniScene PRIVATE ${GLUT_LIBRARIES})
    endif()
endif()

if(MSVC)
    target_compile_options(OpenGLAniScene PRIVATE /W4 /permissive-)
else()
    target_compile_options(OpenGLAniScene PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(OpenGLAniScene PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenGLAniScene>"
)

add_custom_command(TARGET OpenGLAniScene POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:OpenGLAniScene>/assets"
    VERBATIM
)
