cmake_minimum_required(VERSION 3.15)

project(reactorlib
	VERSION 0.1
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(GCC_COMPILE_FLAGS -Werror -Wall -Wextra -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Woverloaded-virtual -Wunused -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wdouble-promotion -Wformat=2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set (REACTOR_SRCS
	${PROJECT_SOURCE_DIR}/libreactor/reactor.h
	${PROJECT_SOURCE_DIR}/libreactor/reactor.cpp
	${PROJECT_SOURCE_DIR}/libreactor/event_handler.h
	${PROJECT_SOURCE_DIR}/libreactor/utils.h)

set (TEST_SRCS
	${PROJECT_SOURCE_DIR}/unit-tests/test_main.cpp
	${PROJECT_SOURCE_DIR}/unit-tests/test_reactor.cpp
)


# Adding reactor library
add_library(libreactor 
	${REACTOR_SRCS})

target_compile_options(libreactor PRIVATE  ${GCC_COMPILE_FLAGS})

add_custom_target(
	clang-tidy-check clang-tidy -extra-arg=-Wno-unknown-warning-option -p ${CMAKE_BINARY_DIR}/compile_commands.json -header-filter=.* ${REACTOR_SRCS} ${TEST_SRCS}
	DEPENDS ${REACTOR_SRCS} ${TEST_SRCS} )

add_custom_target(
        clangformat
		COMMAND clang-format -style=WebKit -i ${REACTOR_SRCS} ${TEST_SRCS}
		DEPENDS ${REACTOR_SRCS} ${TEST_SRCS}
)

add_subdirectory(unit-tests)

enable_testing()

add_test(
NAME unit_test_reactor
COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=check_initialization_order=true:strict_init_order=true 
$<TARGET_FILE:test_reactor>) 


