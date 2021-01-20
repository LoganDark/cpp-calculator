cmake_minimum_required(VERSION 3.12)
project(calculator)

set(CMAKE_CXX_STANDARD 14)

# @formatter:off
add_custom_target(
	emscripten
	ALL
	COMMAND
		/Users/LoganDark/emsdk/emscripten/1.38.20/em++
			--bind -std=c++14 -o emscripten/calc.js -s ENVIRONMENT=web
			#-Oz
			-s DISABLE_EXCEPTION_CATCHING=0
			-s TOTAL_MEMORY=268435456 # 256 MB
			#-g4
			emscripten/emscripten.cpp
	SOURCES emscripten/emscripten.cpp
	WORKING_DIRECTORY ..
)
# @formatter:on

add_executable(calculator main.cpp)

add_dependencies(calculator emscripten)
