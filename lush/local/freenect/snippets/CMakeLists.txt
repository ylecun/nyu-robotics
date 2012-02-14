######################################################################################
# Packages needed for examples
######################################################################################

set(CMAKE_C_FLAGS "-Wall")

add_executable(regview regview.c)
add_executable(test test.c)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${USB_INCLUDE_DIRS})

if (WIN32)
  set(MATH_LIB "")
else(WIN32)
  set(MATH_LIB "m")
endif()

target_link_libraries(regview freenect ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})
target_link_libraries(test freenect ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})


