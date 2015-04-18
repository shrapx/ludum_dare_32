cmake_minimum_required(VERSION 3.1)

set(DEV_DIR "e:/dev")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(TARG ludum_dare_32)
project(ludum_dare_32)

# libraries

set(SFML_ROOT "e:/dev/lib/SFML-2.2")
#set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.2 COMPONENTS audio network graphics window system)
include_directories(${SFML_INCLUDE_DIR})

include_directories("e:/dev/usr/include")

#find_package(Box2D)
#include_directories(${BOX2D_INCLUDE_DIR})
include_directories("e:/dev/lib/")
include_directories("e:/dev/lib/Box2D")

# list of libraries

set(LIBS
	"e:/dev/lib/jsoncpp/libjson_mingw_libmt.a"
	"e:/dev/lib/Box2D/libBox2D.a"
	${BOX2D_LIBRARY}
	${SFML_LIBRARIES}
	${SFML_DEPENDENCIES}
)

# list of source files

set(SRC
	src/main.cpp
	src/app.cpp
	src/mytime.hpp
	src/loader.hpp
	src/entity.hpp
	src/world.hpp
)

add_executable(${TARG} ${SRC})
target_link_libraries(${TARG} ${LIBS})
