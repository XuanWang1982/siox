#ifndef STATISTICS_COLLECTOR_H
#define STATISTICS_COLLECTOR_H

#include "../datatypes/Statistics.hpp"

namespace monitoring{

class StatisticsCollector {
public:

	virtual void register_metrics(String metricname, string type, string domain, string hostname, string si_unit, string description, uint32_t min_poll_interval_ms);
// Hier kann auch der predefined variant-Datatype MetricAttributes genutzt werden.

// Description  means local or remote metric
// Type = gauge if interval - We are converting incremental to gauge

	virtual void get_metrics(StatisticsValue * statvalue);



};


#define STATISTICS_INTERFACE "monitoring_statistics_collector"

}

#endif /* STATISTICS_COLLECTOR_H */
