project( IGL VERSION 1.0.0 LANGUAGES CXX )

set( IGL_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IGL/IGlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IGL/BOX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IGL/PLANE.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/IGL/auxiliar.h
)

set( IGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IGlib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OGLObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auxiliar.cpp
)

set(IGL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/" CACHE INTERNAL "" )

add_library( IGL  ${IGL_HEADERS} ${IGL_SOURCES})

include_directories(${FREEIMAGE_INCLUDE_DIRS})

target_include_directories(IGL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(TARGET FreeImage::FreeImage)
    set(_IGL_FREEIMAGE_TARGET FreeImage::FreeImage)
else()
    set(_IGL_FREEIMAGE_TARGET ${FreeImage_LIBRARIES})
endif()

if(WIN32)
    target_link_libraries( IGL PUBLIC opengl32 GLEW::GLEW GLUT::GLUT ${_IGL_FREEIMAGE_TARGET} )
else()
    target_link_libraries( IGL PUBLIC OpenGL::GL GLEW::GLEW GLUT::GLUT ${_IGL_FREEIMAGE_TARGET} )
endif()

