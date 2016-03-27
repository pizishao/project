cmake_minimum_required(VERSION 3.2)
aux_source_directory(. SERIALIZE_SRCS)
file(GLOB SERIALIZE_SRCS_HEADERS "*.h")

INCLUDE_DIRECTORIES("./")
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib)
ADD_EXECUTABLE(Serialize ${SERIALIZE_SRCS} ${SERIALIZE_SRCS_HEADERS})

if(WIN32)
     target_link_libraries(Serialize libyaml-cppmdd.lib)
else()
     target_link_libraries(Serialize libyaml-cpp.a)
endif()