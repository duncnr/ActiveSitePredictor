cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CONFIGURATION_TYPES Release Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ideally would use find_package, unsure how that's supposed to work on windows though
#add_subdirectory(include/glfw)

#target_include_directories(${PROJECT_NAME} PRIVATE
#	include/glfw/include
#)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/cpp/*.cpp")
list(APPEND SOURCES "src/main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
#	lib/glfw/include
	src
)

#target_link_libraries(${PROJECT_NAME} PRIVATE
#	glfw
#)

# for later.. copy resources (models, shaders) after build
#[[
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/src/resource"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/resource"
)

# runs executable after build
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND $<TARGET_FILE:${PROJECT_NAME}>
	WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
#]]
