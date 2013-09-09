#include <mutex>
#include <iostream>

#include <core/autoconfigurator/AutoConfigurator.hpp>
#include <core/module/ModuleLoader.hpp>
#include <core/autoconfigurator/AutoConfigurator.hpp>
#include <core/container/container-serializer.hpp>

#include "ModuleOptions.hpp"

using namespace std;

using namespace core;

// store a global registrar while the data is deserialized
// this is a workaround, since deserialization templates of boost do not take additional arguments.
//
// With Vala the whole code was much easier to understand, shorter and ways better to maintain.
ComponentRegistrar * autoConfiguratorRegistrar;
int autoConfiguratorOffset;
static mutex registrarMutex;

namespace core {
	/**
	 * Describes the module to load in the configuration.
	 */

	AutoConfigurator::~AutoConfigurator()
	{
		delete( configurationProvider );
	}

	AutoConfigurator::AutoConfigurator( ComponentRegistrar * r, string conf_provider_module, string module_path, string configuration_entry_point )
	{
		configurationProvider = module_create_instance<ConfigurationProvider>( module_path, conf_provider_module, CONFIGURATION_PROVIDER_INTERFACE );
		configurationProvider->connect( configuration_entry_point );
		registrar = r;
	}

	/*
	 Ownership of the conf_provider is given to the AutoConfigurator
	 */
	AutoConfigurator::AutoConfigurator( ComponentRegistrar * r, ConfigurationProvider * conf_provider )
	{
		registrar = r;
		string val = "";
		configurationProvider = conf_provider;
		configurationProvider->connect( val );
	}

	vector<Component *> AutoConfigurator::LoadConfiguration( string type, string matchingRules ) throw( InvalidComponentException, InvalidConfiguration )
	{
		string config = configurationProvider->getConfiguration( type, matchingRules );
		if( config.length() < 10 ) {
			throw InvalidConfiguration( "Configuration is unavailable or too short", "" );
		}

		// replace <X> with  <object class_id="1" class_name="X">
		// replace </X> with Container></Container></object>

		stringstream transformed_config;

		size_t current;
		size_t next = std::string::npos;
		size_t container_set_pos;
		do {
			current = next + 1;
			next = config.find( "\n<", current );
			size_t end_pos = config.find( ">", current );


			if( config[current + 1] == '/' ) {
				if( container_set_pos >= next ) {
					transformed_config << "\t<Container></Container>" << endl;
				}
				transformed_config << "</object>" << endl;
			} else {
				container_set_pos = config.find( "<Container></Container>", current );
				transformed_config << "<object class_id=\"1\" class_name=\"" << config.substr( current + 1, end_pos - current - 1 ) << "\">" << endl;
				transformed_config << config.substr( end_pos + 2, next - end_pos - 2 ) << endl;
			}

			//cout << "  [" << config.substr(current, next - current) << "]" << endl;
		} while( next != std::string::npos );


		// parse string
		vector<Component *> components;

		registrarMutex.lock();
		autoConfiguratorOffset = registrar->number_of_registered_components();
		autoConfiguratorRegistrar = registrar;

		ContainerSerializer cs = ContainerSerializer();
		stringstream already_parsed_config( transformed_config.str() );
		//cout << transformed_config.str() << endl;
		while( ! already_parsed_config.eof() ) {
			// check if the stream is empty
			char c;
			already_parsed_config >> c;
			already_parsed_config.unget();
			if( !already_parsed_config.good() )
				break;

			// no, so we hope that the configuration is complete
			LoadModule * module;
			ComponentOptions * options;
			Component * component;

			//cout << "Parsing a module" << endl;
			try {
				module = dynamic_cast<LoadModule *>( cs.parse( already_parsed_config ) );
			} catch( exception & e ) {
				autoConfiguratorRegistrar = nullptr;
				registrarMutex.unlock();

				cerr << "Error: " << e.what() << endl;
				throw InvalidConfiguration( "Error while parsing module configuration", "");
			}

			//cout << "Parsed module description: "  << already_parsed_config.tellg() << endl;
			component = module_create_instance<Component>( module->path, module->name, module->interface );
			//cout << DumpConfiguration(component->get_options()) << endl;

			try {
				options = cs.parse( already_parsed_config );
			} catch( exception & e ) {
				autoConfiguratorRegistrar = nullptr;
				registrarMutex.unlock();
				ComponentOptions & availableOptions = component->getOptions();
				string  str = cs.serialize( & availableOptions );
				cerr << endl << "Expected configuration: " << str << endl;

				throw InvalidConfiguration( "Error during parsing of options", module->name );
			}

			try {
				component->init( options );
			} catch( exception & e ) {
				autoConfiguratorRegistrar = nullptr;
				registrarMutex.unlock();

				string  str = cs.serialize( options );
				cerr << "Configuration values: " << str << endl;
				throw InvalidConfiguration( string( "Error during initialization: " ) + e.what(), module->name );
			}
			registrar->register_component( module->componentID + autoConfiguratorOffset, component );

			components.push_back( component );

			delete( module );
		}
		autoConfiguratorRegistrar = nullptr;
		registrarMutex.unlock();

		return components;
	}

	string AutoConfigurator::DumpConfiguration( ComponentOptions * options )
	{
		ContainerSerializer cs = ContainerSerializer();
		string  str = cs.serialize( options );
		return str;
	}

}