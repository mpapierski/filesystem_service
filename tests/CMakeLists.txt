add_subdirectory (gmock)

find_package (Boost REQUIRED COMPONENTS
	system
	filesystem
	thread)

include_directories (
	gmock/include/
	gtest/include/
	${Boost_INCLUDE_DIRS})
add_executable (operations
	operations.cpp)
target_link_libraries (operations
	${Boost_LIBRARIES}
	gmock
	gtest
	gtest_main)