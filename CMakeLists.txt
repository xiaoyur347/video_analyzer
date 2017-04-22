CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(video_analyzer)

SET (CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -g -O0")

SET (SRC_FILES
	main.cpp
	buffer.cpp
	ts.cpp
)

include_directories(
)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_FILES})
