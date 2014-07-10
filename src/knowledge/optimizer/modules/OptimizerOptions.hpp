#include <core/component/component-options.hpp>
#include <knowledge/optimizer/OptimizerPluginOptions.hpp>
#include <monitoring/activity_multiplexer/ActivityMultiplexerPluginOptions.hpp>

#include <vector>
#include <utility>

//@serializable
class OptimizerOptions : public ActivityMultiplexerPluginOptions{
	ComponentReference optimizer;

	// the layer we are operating on must be the layer we are connected to.
	string interface;
	string implementation;

	vector<pair<string,string> > hintAttributes;	// attributes containing hints, e.g. ('MPI','hint/noncollReadBuffSize')

};

