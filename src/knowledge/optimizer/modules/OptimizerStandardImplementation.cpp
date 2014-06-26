/**
 * @file OptimizerStandardImplementation.cpp
 *
 * A simple and straightforward implementation of the abstract Optimizer class.
 *
 * @date 2013-08-01
 * @author Michaela Zimmer
 */

#include <knowledge/optimizer/OptimizerImplementation.hpp>

#include <core/component/Component.hpp>
#include <knowledge/optimizer/OptimizerPluginInterface.hpp>
#include <monitoring/ontology/OntologyDatatypes.hpp>

#include <unordered_map>

#include "OptimizerOptions.hpp"

namespace knowledge {

	class OptimizerStandardImplementation : public Optimizer {

		private:

			// Map to store plugins in, indexed by attributes' IDs
			unordered_map<OntologyAttributeID, OptimizerInterface *> expert;

		protected:

			ComponentOptions * AvailableOptions() {
				return new OptimizerOptions();
			}


		public:

			 void registerPlugin( OntologyAttributeID aid, const OptimizerInterface * plugin ) override {
				assert( plugin != nullptr );
				assert( expert[aid] == nullptr );
cout << "registerPlugin" << plugin << endl;
				expert[aid] = ( OptimizerInterface * ) plugin;
			}


			bool isPluginRegistered( OntologyAttributeID aid ) const override{
				return ( expert.find( aid ) != expert.end() );
			}


			void unregisterPlugin( OntologyAttributeID aid ) override {
				expert.erase( aid );
			}


			OntologyValue optimalParameter( OntologyAttributeID aid ) const throw( NotFoundError ) override{
				///@todo Check for registered plug-in?
				auto res = expert.find( aid );

				if( res != expert.end() ) {
					return res->second->optimalParameter( aid );
				} else {
					throw NotFoundError( "Illegal attribute!" );
				}
			}

			OntologyValue optimalParameterFor( OntologyAttributeID aid, const Activity * activityToStart ) const throw( NotFoundError ) override {
				///@todo Check for registered plug-in?
				bool flag = false;
				flag = isPluginRegistered(aid);
cout << "Is the optimizer plugin registered?" << boolalpha << flag << endl;
				if(!flag){
					cout << "TODO registerPlugin()" << endl;
				}

				auto res = expert.find( aid );
cout << "=============Test for optimalParameterFor" << endl;
cout << "***optimalParameterFor*** The attribute ID is: " << aid << endl;

				if( res != expert.end() ) {
cout << "optimalParameterFor res != expert.end()" << endl;
					return res->second->optimalParameterFor( aid, activityToStart );
				} else {
cout << "optimalParameterFor NotFoundError" << endl;
					throw NotFoundError( "Illegal attribute!" );
				}
			}


			virtual void init() {

			}
	};
} // namespace knowledge


extern "C" {
	void * KNOWLEDGE_OPTIMIZER_INSTANCIATOR_NAME()
	{
		return new knowledge::OptimizerStandardImplementation();
	}
}

// BUILD_TEST_INTERFACE knowledge/optimizer/modules/
