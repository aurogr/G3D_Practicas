project(p3OGL VERSION 1.0.0 LANGUAGES CXX)

set(p3OGL_SOURCES 
    main.cpp
)

include_directories(${FREEIMAGE_INCLUDE_DIRS})

add_executable(p3OGL ${p3OGL_SOURCES})

target_include_directories(p3OGL PUBLIC ${IGL_INCLUDE})

if(WIN32)
    # Windows system OpenGL
    target_link_libraries(p3OGL IGL opengl32 GLEW::GLEW GLUT::GLUT)
else()
    # Linux / MacOS
    find_package(OpenGL REQUIRED)
    target_link_libraries(p3OGL IGL OpenGL::GL GLEW::GLEW GLUT::GLUT)
endif()

# Define absolute path to shaders folder
target_compile_definitions(p3OGL PRIVATE SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders_P3")

# Define absolute path to textures folder
target_compile_definitions(p3OGL PRIVATE TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../img")