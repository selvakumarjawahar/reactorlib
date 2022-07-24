add_executable(test_reactor
	${PROJECT_SOURCE_DIR}/unit-tests/test_main.cpp
	${PROJECT_SOURCE_DIR}/unit-tests/test_reactor.cpp)

target_compile_options(test_reactor PRIVATE  ${GCC_COMPILE_FLAGS} -g -fsanitize=address,undefined -fno-omit-frame-pointer)
target_link_options(test_reactor PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)

target_include_directories(test_reactor
	PRIVATE ${PROJECT_SOURCE_DIR}/libreactor)

target_link_libraries(test_reactor
	PRIVATE libreactor)

# Add Ctest and catch
include(CTest)
