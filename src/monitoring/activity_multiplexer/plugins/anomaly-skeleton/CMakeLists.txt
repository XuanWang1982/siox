project(SIOX)

include_directories(
	${SIOX_INCLUDE_DIRS}
	${Boost_INCLUDE_DIR}
)

add_library(
	siox-monitoring-activityPlugin-AnomalySkeleton
	AnomalySkeleton.cpp
	BoostXMLSerializable.cpp
)

target_link_libraries(
	siox-monitoring-activityPlugin-AnomalySkeleton 
	siox-core-AutoConfigurator
)

siox_run_serializer(AnomalySkeletonOptions.hpp BoostXMLSerializable.cpp)


add_subdirectory(test)

install(TARGETS siox-monitoring-activityPlugin-AnomalySkeleton LIBRARY DESTINATION ${SIOX_INSTALL_LIBRARY_DIR})
