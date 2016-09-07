cmake_minimum_required(VERSION 3.0)

if(WIN32)
    ADD_DEFINITIONS(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS) 
else()
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
endif()

aux_source_directory(. ROOT_SRCS)
file(GLOB ROOT_HEADERS "*.h")

INCLUDE_DIRECTORIES("./")

ADD_SUBDIRECTORY(net) 
ADD_SUBDIRECTORY(base) 

ADD_EXECUTABLE(MuduoPlus ${ROOT_SRCS} ${ROOT_HEADERS})

if(WIN32)
     target_link_libraries(MuduoPlus ws2_32.lib net base)
else()
	target_link_libraries(MuduoPlus net base)
endif()