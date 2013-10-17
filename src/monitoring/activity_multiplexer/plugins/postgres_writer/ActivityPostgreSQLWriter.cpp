#include <iostream>
#include <fstream>
#include <list>
#include <mutex>

#include "libpq-fe.h"

#include <boost/archive/text_oarchive.hpp>

#include <monitoring/datatypes/Activity.hpp>
#include <monitoring/activity_multiplexer/ActivityMultiplexerPluginImplementation.hpp>
#include <monitoring/activity_multiplexer/ActivityMultiplexerListener.hpp>
#include <monitoring/activity_multiplexer/plugins/postgres_writer/PostgreSQLQuerier.hpp>

#include "ActivityPostgreSQLWriterOptions.hpp"

#include <core/component/ActivitySerializableText.cpp>

using namespace std;
using namespace monitoring;
using namespace core;

// It is important that the first parent class is of type ActivityMultiplexerPlugin
class PostgreSQLWriterPlugin: public ActivityMultiplexerPlugin {
private:
	PostgreSQLQuerier *querier_;
	PGconn *dbconn_;
public:

	void Notify(Activity *activity) 
	{
		querier_->insert_activity(*activity);
	}

	ComponentOptions *AvailableOptions() 
	{
		return new PostgreSQLWriterPluginOptions();
	}

	void initPlugin( ) 
	{
		PostgreSQLWriterPluginOptions &o = getOptions<PostgreSQLWriterPluginOptions>();
		
		dbconn_ = PQconnectdb(o.db_info.c_str());
		querier_ = new PostgreSQLQuerier(*dbconn_);
	}

	~PostgreSQLWriterPlugin() 
	{
		PQfinish(dbconn_);
	}
};

extern "C" {
	void *MONITORING_ACTIVITY_MULTIPLEXER_PLUGIN_INSTANCIATOR_NAME()
	{
		return new PostgreSQLWriterPlugin();
	}
}
