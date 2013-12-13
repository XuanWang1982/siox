#include <util/OverheadStatistics.hpp>
#include <util/time.h>

using namespace std;
using namespace util;
using namespace core;



Timestamp OverheadStatistics::startMeasurement(){
	return siox_gettime();
}

void OverheadStatistics::stopMeasurement(const std::string & what, const Timestamp & start){
	stopMeasurement(getOverheadFor(what), start);
}

void OverheadStatistics::stopMeasurement(OverheadEntry & entry, const Timestamp & start){
	Timestamp delta = siox_gettime() - start;
	entry.occurence++;
	entry.time += delta;

	all.time += delta;
	all.occurence++;
}

OverheadEntry & OverheadStatistics::getOverheadFor(const std::string & what){
	assert(what.size() > 2);
	
	auto itr = entries.find(what);
	if ( itr == entries.end() ){
		entries[what] = OverheadEntry();
		return entries[what];
	}
	return itr->second;
}


void OverheadStatistics::appendReport(unordered_map<core::GroupEntry* , ReportEntry> & map){
	for( auto itr = entries.begin(); itr != entries.end() ; itr++ ){
		const string & name = itr->first;
		const OverheadEntry & entry = itr->second;

		auto group = new GroupEntry(name);
		auto count = new GroupEntry("count", group);
		auto time = new GroupEntry("time", group);

		map[count] = {ReportEntry::Type::SIOX_INTERNAL_PERFORMANCE, entry.occurence };
		map[time] = {ReportEntry::Type::SIOX_INTERNAL_PERFORMANCE, entry.time / 1000000000.0};
	}

	// add sum with a high priority:
	{
	auto group = new GroupEntry("all");
	auto count = new GroupEntry("count", group);
	auto time = new GroupEntry("time", group);

	map[count] = {ReportEntry::Type::SIOX_INTERNAL_PERFORMANCE, all.occurence, 200 };
	map[time] = {ReportEntry::Type::SIOX_INTERNAL_PERFORMANCE, all.time / 1000000000.0, 200 };
	}
}

core::ComponentReport OverheadStatisticsDummy::prepareReport(){
	ComponentReport rep;
	stats.appendReport( rep.data );
	return rep;
}