project(SIOX)

include_directories(
	${SIOX_INCLUDE_DIRS}
	${SIOX_SOURCES}
)

 add_library(
 	OptimizerSkeleton SHARED
 	OptimizerSkeleton.hpp
 	OptimizerSkeleton.cpp
 )
 
target_link_libraries(
	siox-knowledge-optimizer-optimizer-skeleton
)

siox_run_serializer(OptimizerSkeleton.hpp OptimizerSkeleton.cpp)

# add_subdirectory(test)

install(TARGETS siox-knowledge-optimizer-optimizer-skeleton LIBRARY DESTINATION ${SIOX_INSTALL_LIBRARY_DIR})
