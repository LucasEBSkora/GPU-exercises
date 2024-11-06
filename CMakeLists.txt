cmake_minimum_required(VERSION 3.0)
project(sort)
set (CMAKE_CXX_STANDARD 11)

include_directories(
  lib/
  common/
)

link_directories(
	${PROJECT_SOURCE_DIR}/OpenCL-ICD-Loader/inc
	${PROJECT_SOURCE_DIR}/OpenCL-ICD-Loader/inc/CL
)

IF (WIN32)
  SET(CL_LIBRARIES OpenCL)
ELSE (WIN32)
  SET(CL_LIBRARIES OpenCL)
ENDIF(WIN32)

add_definitions(-DSRC_PATH=\"${CMAKE_SOURCE_DIR}/\")
add_executable(sort main.cpp common/clutils.cpp)
if(APPLE)
	target_link_libraries(sort "-framework OpenCL" )
else()
	target_link_libraries(sort ${CL_LIBRARIES})
endif()

add_subdirectory(OpenCL-ICD-Loader)
