cmake_minimum_required(VERSION 3.12)

project(Vault VERSION 0.0)

file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)