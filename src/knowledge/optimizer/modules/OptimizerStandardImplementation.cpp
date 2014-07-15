/**
 * @file OptimizerStandardImplementation.cpp
 *
 * A simple and straightforward implementation of the abstract Optimizer class.
 *
 * @date 2013-08-01
 * @author Michaela Zimmer
 */
#include <iostream>
#include <sstream>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <mutex>

#include <util/ExceptionHandling.hpp>
#include <core/reporting/ComponentReportInterface.hpp>
#include <monitoring/activity_multiplexer/ActivityMultiplexerPluginImplementation.hpp>
#include <monitoring/system_information/SystemInformationGlobalIDManager.hpp>
#include <monitoring/datatypes/Activity.hpp>

#include <workarounds.hpp>

#include <knowledge/optimizer/OptimizerImplementation.hpp>

#include <core/component/Component.hpp>
#include <knowledge/optimizer/OptimizerPluginInterface.hpp>
#include <monitoring/ontology/OntologyDatatypes.hpp>
#include <knowledge/optimizer/Optimizer.hpp>

#include <unordered_map>

#include "OptimizerOptions.hpp"

using namespace std;
using namespace monitoring;
using namespace core;
using namespace knowledge;

//namespace knowledge {

	class OptimizerStandardImplementation : public ActivityMultiplexerPlugin, public OptimizerInterface {

		private:
			// Map to store plugins in, indexed by attributes' IDs
			unordered_map<OntologyAttributeID, OptimizerInterface *> expert;
			UniqueInterfaceID uiid;
			// AttID of the attribute user-id
			OntologyAttributeID uidAttID;	///@todo TODO: This variable is currently dead code. Remove it or use it.

/*		protected:

			ComponentOptions * AvailableOptions() {
				return new OptimizerOptions();
			}*/

		public:
			void init();
			ComponentOptions * AvailableOptions() override;
			void registerPlugin(OntologyAttributeID aid, const OptimizerInterface * plugin) override;
			bool isPluginRegistered(OntologyAttributeID aid) const override;
			void unregisterPlugin(OntologyAttributeID aid) override;
			OntologyValue optimalParameter(OntologyAttributeID aid) const throw(NotFoundError) override;
			OntologyValue optimalParameterFor(OntologyAttributeID aid, const Activity * activityToStart) const throw(NotFoundError) override;
			~OptimizerStandardImplementation();
	};
	//void registerPlugin( OntologyAttributeID aid, const OptimizerInterface * plugin ) override {
			 void OptimizerStandardImplementation::registerPlugin( OntologyAttributeID aid, const OptimizerInterface * plugin ) override {
				assert( plugin != nullptr );
				assert( expert[aid] == nullptr );
cout << "****SIOX DEBUG**** registerPlugin" << plugin << endl;
				expert[aid] = ( OptimizerInterface * ) plugin;
			}


			//bool isPluginRegistered( OntologyAttributeID aid ) const override{
			bool OptimizerStandardImplementation::isPluginRegistered( OntologyAttributeID aid ) const override {
				return ( expert.find( aid ) != expert.end() );
			}


			//void unregisterPlugin( OntologyAttributeID aid ) override {
			void OptimizerStandardImplementation::unregisterPlugin( OntologyAttributeID aid ) override {
				expert.erase( aid );
			}


			//OntologyValue optimalParameter( OntologyAttributeID aid ) const throw( NotFoundError ) override{
			OntologyValue OptimizerStandardImplementation::optimalParameter( OntologyAttributeID aid ) const throw( NotFoundError ) override {
				///@todo Check for registered plug-in?
				auto res = expert.find( aid );

				if( res != expert.end() ) {
					return res->second->optimalParameter( aid );
				} else {
					throw NotFoundError( "Illegal attribute!" );
				}
			}

			//OntologyValue optimalParameterFor( OntologyAttributeID aid, const Activity * activityToStart ) const throw( NotFoundError ) override {
			OntologyValue OptimizerStandardImplementation::optimalParameterFor( OntologyAttributeID aid, const Activity * activityToStart ) const throw( NotFoundError ) override {
				///@todo Check for registered plug-in?
				bool flag = false;
				flag = isPluginRegistered(aid);
cout << "****SIOX DEBUG**** Is the optimizer plugin registered?" << boolalpha << flag << endl;
				if(!flag){
					//registerPlugin(aid, this);
cout << "****SIOX DEBUG**** TODO registerPlugin()" << endl;
				}

				auto res = expert.find( aid );
cout << "****SIOX DEBUG**** Test for optimalParameterFor" << endl;
cout << "****SIOX DEBUG**** optimalParameterFor The attribute ID is: " << aid << endl;

				if( res != expert.end() ) {
cout << "****SIOX DEBUG**** optimalParameterFor res != expert.end()" << endl;
					return res->second->optimalParameterFor( aid, activityToStart );
				} else {
cout << "****SIOX DEBUG**** optimalParameterFor NotFoundError" << endl;
					throw NotFoundError( "Illegal attribute!" );
				}
			}


			void OptimizerStandardImplementation::init() {
cout<< "****SIOX DEBUG**** virtual void init() in OptimizerStandardImplementation.cpp"<< endl;

				// Retrieve options
				OptimizerOptions & o = getOptions<OptimizerOptions> ();
				SystemInformationGlobalIDManager * sysinfo = facade->get_system_information();
				assert(sysinfo);
				// Retrieve uiid
				RETURN_ON_EXCEPTION( uiid = sysinfo->lookup_interfaceID(o.interface, o.implementation); );
cout<< "****SIOX DEBUG**** virtual void init() in OptimizerStandardImplementation.cpp uiid is "<< uiid << endl;

				try{
					Optimizer * optimizer = GET_INSTANCE(Optimizer, o.optimizer);
					assert(optimizer);
					for( auto itr = o.hintAttributes.begin(); itr != o.hintAttributes.end(); itr++ ) {
						string domain = itr->first;
						string attribute = itr->second;

						OntologyAttribute ontatt = facade->lookup_attribute_by_name(domain, attribute);
						optimizer->registerPlugin( ontatt.aID, this );
cout<< "****SIOX DEBUG**** void init() in OptimizerStandardImplementation.cpp aID is "<< ontatt.aID << endl;
					}
				} catch(...) {
					assert(0 && "Fatal error, cannot look up an attribute that could be looked up previously. Please report this bug."), abort();
				}
			}
			OptimizerStandardImplementation::~OptimizerStandardImplementation(){
cout<< "****SIOX DEBUG**** OptimizerStandardImplementation::~OptimizerStandardImplementation()" << endl;
			}

//} // namespace knowledge


extern "C" {
	void * KNOWLEDGE_OPTIMIZER_INSTANCIATOR_NAME()
	{
		return new OptimizerStandardImplementation();
		//return new knowledge::OptimizerStandardImplementation();
	}
}

// BUILD_TEST_INTERFACE knowledge/optimizer/modules/
