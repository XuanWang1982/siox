project(SIOX)

include_directories(
	${SIOX_INCLUDE_DIRS}
)

add_library(
	siox-util-jobProcessors
	DefaultProcessorQueues.cpp
	SingleThreadedJobProcessor.cpp
)

target_link_libraries(
	siox-util-jobProcessors
	siox-util
)

install(TARGETS siox-util-jobProcessors LIBRARY DESTINATION ${SIOX_INSTALL_LIBRARY_DIR})
