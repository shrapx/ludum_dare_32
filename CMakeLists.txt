cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(TARG ludum_dare_32)
project(${TARG} CXX)

add_definitions(-std=c++11)

# in linux find_package and static didnt work automatically
# i added the libs manually instead

if (UNIX)
	find_package(JSONCPP)
	include_directories("/home/shrapx/Documents/dev/SFML/include")
endif()


if (WIN32)
	include_directories("e:/dev/usr/include")
	include_directories("e:/dev/lib/")

	# im not using find_package for windows jsoncpp
	set(JSONCPP_LIBRARY "e:/dev/lib/jsoncpp/libjson_mingw_libmt.a")

	set(SFML_ROOT "e:/dev/lib/SFML-2.2")
	set(SFML_STATIC_LIBRARIES TRUE)
	find_package(SFML 2.2 COMPONENTS audio graphics window system)
	include_directories(${SFML_INCLUDE_DIR})
endif()

set(LIBS
	${JSONCPP_LIBRARY}
)

if (WIN32)
	set(LIBS ${LIBS}
		${SFML_LIBRARIES}
		${SFML_DEPENDENCIES}
	)
endif()


if (UNIX)
	set(LIBS ${LIBS}
		/home/shrapx/Documents/dev/SFML/lib/libsfml-audio.so
		/home/shrapx/Documents/dev/SFML/lib/libsfml-graphics.so
		/home/shrapx/Documents/dev/SFML/lib/libsfml-window.so
		/home/shrapx/Documents/dev/SFML/lib/libsfml-system.so
	)
endif()

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
