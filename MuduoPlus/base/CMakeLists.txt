aux_source_directory(. BASE_SRCS)
file(GLOB BASE_HEADERS "*.h")
ADD_LIBRARY(base ${BASE_SRCS} ${BASE_HEADERS})