project(SIOX)

add_subdirectory(activity_forwarder)
add_subdirectory(activity_net_forwarder)
add_subdirectory(anomaly-skeleton)
add_subdirectory(filewriter)
add_subdirectory(fadviseReadAhead)
add_subdirectory(file-surveyor)
add_subdirectory(activity-histogram-adpi)
add_subdirectory(anomaly-injector)
add_subdirectory(activityMonitor)

add_subdirectory(binwriter)

if (PostgreSQL_FOUND)
	add_subdirectory(postgres_writer)
endif (PostgreSQL_FOUND)

if (BDB_FOUND)
	add_subdirectory(bdbwriter)
endif (BDB_FOUND)
