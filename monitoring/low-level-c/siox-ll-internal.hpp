#ifndef SIOX_LL_INTERNAL_H_
#define SIOX_LL_INTERNAL_H_


#include <C/siox-types.h>
#include <monitoring/datatypes/ids.hpp>
#include <monitoring/ontology/Ontology.hpp>
#include <monitoring/system_information/SystemInformationGlobalIDManager.hpp>
#include <monitoring/association_mapper/AssociationMapper.hpp>

#include <core/autoconfigurator/AutoConfigurator.hpp>

#include <monitoring/activity_multiplexer/ActivityMultiplexer.hpp>
#include <monitoring/activity_builder/ActivityBuilder.hpp>


using namespace core;
using namespace monitoring;

// define all types for CPP
typedef AssociateID siox_associate;
typedef OntologyAttribute siox_attribute;
typedef UniqueComponentActivityID siox_component_activity;
typedef NodeID siox_node;

typedef UniqueInterfaceID siox_unique_interface; // will be stuffed.
typedef ActivityID siox_activity;
typedef RemoteCallIdentifier siox_remote_call;



struct siox_component{
    ComponentID cid;
    UniqueInterfaceID uid;
    AssociateID instance_associate;

    ActivityMultiplexer * amux;
    // ActivityBuilder abuilder;
};

/**
 * Implementation of the low-level API
 * The C-implementation serves as lightweight wrapper for C++ classes.
 */

struct process_info{
	NodeID nid;
	ProcessID pid;

	// Loaded ontology implementation
	monitoring::Ontology * ontology;
    // Loaded system information manager implementation
    monitoring::SystemInformationGlobalIDManager * system_information_manager;
    // Loaded association mapper implementation
    monitoring::AssociationMapper * association_mapper;

    // MZ: TODO Add code to load this somewhere!
    // Loaded activity builder implementation
    // monitoring::ActivityBuilder * activity_builder;
    ActivityMultiplexer * amux;

    // Contains all components
    core::ComponentRegistrar * registrar;

    // Loads all component modules
    core::AutoConfigurator * configurator;
};


/*
 * Create a local ProcessID
 */
ProcessID create_process_id(NodeID node);


/*
 These functions are mainly used for testing purposes and since testing is based on C++ classes it is not useful to keep them.
 void siox_process_register(siox_nodeID * siox_nodeID, ProcessID * pid);
 void siox_process_finalize();
 */

#endif
