/**
 * @file siox-ll.h SIOX low-level interface headers.
 *
 * @date 2013-08-16
 * @copyright GNU Public License
 * @authors Michaela Zimmer, Julian Kunkel, Alvaro Aguilera
 *
 *
 * Bridging the gap between the instrumented components and SIOX proper, the SIOX low-level
 * API is a centre piece of the system, as an overview of its interactions with other
 * SIOX modules and interfaces shows:

    @startuml{siox-ll-interactions.png}
        title Interactions between the Low-Level-C-API, SIOX-Components and Modules

        folder "Process" {

        component [Thread] #Wheat

        folder "Low-Level-C-API" {
            component [SIOX-LL] #PowderBlue
            interface "Monitoring" #Orange
            component [Activity Builder] #Orange

            note left of [SIOX-LL]
             Bridge between C
             and C++ datatypes
            end note

            [Thread] ..> [SIOX-LL] : use
        }

        interface "ConfigurationProvider"
        interface "Ontology" #Yellow
        'component [FileOntology] #Yellow
        'component [DBOntology] #Orange
        component [Optimizer] #Plum
        component [AMux] #Orange
        component [AForwarder] #Orange
        component [ModuleLoader]
        component [AutoConfigurator]

        component [SOPI] #Plum
        component [ADPI] #Plum

        [AutoConfigurator] ..> [ConfigurationProvider] : use
        [AutoConfigurator] ..> [ModuleLoader] : use
        [SIOX-LL] ..> [Ontology] : use
        'Ontology - [FileOntology]
        'Ontology - [DBOntology]
        [SIOX-LL] ..> [Optimizer] : use
        [SIOX-LL] ..> [AutoConfigurator] : use
        [SIOX-LL] ..> [Monitoring] : use

        [Monitoring] ..> [Activity Builder] : use
        [Monitoring] --> [AMux] : Activity


        [AMux] --> [SOPI]
        [AMux] --> [ADPI]
        [AMux] --> [AForwarder]
        [Optimizer] --> [SOPI] : chooses\nrelevant

        }
    @enduml

 *
 * A note on storage mangement:
 * ----------------------------------
 * Right now, our philosophy is that SIOX will take care of
 * all objects created by calls to it.
 * Thus, instrumenting a component for SIOX will not
 * burden you with calls to malloc() and free(); the library
 * will do it all for you.
 *
 */



#ifndef siox_LL_H
#define siox_LL_H

#include <C/siox-types.h>

/// @name Typedefs to Hide Implementation from C Interface
/// <i>*Jedi Hand Wave*</i>
/// "These aren't the typedefs you're looking for..."
/** @{ */
// We hide all types from the implementation
#ifndef SIOX_INTERNAL_USAGE
typedef void siox_activity;
typedef void siox_component;
typedef void siox_remote_call;
#endif

typedef void siox_attribute;
typedef void siox_component_activity;
typedef void siox_node;
typedef void siox_associate;
typedef void siox_unique_interface;

// to be adjusted depending on ID-type.
typedef struct {
	char padding[24];
} siox_activity_ID;

/** @} */

#define SIOX_INVALID_ID 0


//@test
//@null 1
int siox_is_monitoring_enabled();

//@test
//@null 0
int siox_is_monitoring_permanently_disabled();

//@test
void siox_disable_monitoring();

//@test
void siox_enable_monitoring();

// these two functions can be used to finalize / re-initialize monitoring, which is useful e.g. for fork()
//@test
void siox_finalize_monitoring();
//@test
void siox_initialize_monitoring();
//@test
void siox_handle_prepare_fork();
//@test
void siox_handle_fork_complete(int im_the_child);

// Return the number of times SIOX has been initialized, this is useful to detect fork().
//@test
//@null 1
int siox_initialization_count();

// this internal function allows to check if we are calling a function within SIOX
//@test
//@null 1
int siox_monitoring_namespace_deactivated();

//////////////////////////////////////////////////////////////////////////////
/// Retrieve the node id object for a given hardware component.
//////////////////////////////////////////////////////////////////////////////
/// @param [in] hostname The hardware node's host name, which has to be
///     system-wide unique. If set to NULL, SIOX will use the local hostname.
//////////////////////////////////////////////////////////////////////////////
/// @return A node id, which is system-wide unique
//////////////////////////////////////////////////////////////////////////////
//@test ''%s'' hostname
//@null
siox_node * siox_lookup_node_id( const char * hostname );


//////////////////////////////////////////////////////////////////////////////
/// Set a global attribute and its value for a process.
/// Adding an attribute to the process allows different layers to add globally
/// valid runtime information (such as MPI rank).
//////////////////////////////////////////////////////////////////////////////
/// @param [in] attribute The attribute to set
/// @param [in] value The attribute's new value
//////////////////////////////////////////////////////////////////////////////
//@test ''%p,%p'' attribute,value
void siox_process_set_attribute( siox_attribute * attribute, const void * value );



//////////////////////////////////////////////////////////////////////////////
/// Look up an attribute in the SIOX ontology, creating a new entry if not
/// found.
//////////////////////////////////////////////////////////////////////////////
/// @param [in] domain The contextual domain this attribute belongs to,
///     e.g. "Node", "Process", "POSIX", "MPI" AND sth. like "OpenMPIV1"
/// @param [in] name The attribute's name
/// @param storage_type [in] The minimum storage type required to represent
///     data of this attribute
//////////////////////////////////////////////////////////////////////////////
/// @return A pointer to a globally unique representation in a siox_attribute,
///     unless there already exists an attribute with identical domain and
///     name but different storage_type.
///     In the latter case, a @c nullptr is returned.
//////////////////////////////////////////////////////////////////////////////
/// @note The key (domain, name) must be unique; it is not allowed to use
///     different storage types and/or attributes with the same attribute.
//////////////////////////////////////////////////////////////////////////////
//@test ''%s,%s,%d'' domain,name,storage_type
//@null (siox_attribute*) name
siox_attribute * siox_ontology_register_attribute( const char * domain, const char * name, enum siox_ont_storage_type storage_type );

/**@}*/

//////////////////////////////////////////////////////////////////////////////
/// Look up an attribute with a unit in the SIOX ontology, creating a new
/// entry if not found.
/// This is merely a convenience funtion for the very common case of an
/// attribute that is measures in a unit.
//////////////////////////////////////////////////////////////////////////////
/// @param [in] domain The domain the attribute belongs to.
/// @param [in] name The attribute's full qualified name.
/// @param [in] unit The unit used to measure values in the attribute.
/// @param storage_type [in] The minimum storage type required to store values
///     of the attribute. This type will also be assumed for type casts!
//////////////////////////////////////////////////////////////////////////////
/// @return A pointer to the @em siox_attribute for the attribute or NULL,
///     if the data given is inconsistent with an existing attribute of the
///     same name.
//////////////////////////////////////////////////////////////////////////////
/// @note Units themselves are implemented as meta attributes with storage
///     type SIOX_STORAGE_STRING, domain "Meta" and name "Unit".
//////////////////////////////////////////////////////////////////////////////
//@test ''%s,%s,%s,%d'' domain,name,unit,storage_type
//@null  (siox_attribute*) name
siox_attribute * siox_ontology_register_attribute_with_unit( const char * domain, const char * name, const char * unit, enum siox_ont_storage_type storage_type );


//////////////////////////////////////////////////////////////////////////////
/// Attach a meta attribute and its value to a given attribute, e.g. "Unit".
/// Return @c false if a conflict with a currently set attribute occurs, as
/// there currently is no way to change, unset or overwrite a set attribute.
//////////////////////////////////////////////////////////////////////////////
/// @param [in] parent_attribute
/// @param [in] meta_attribute
/// @param [in] value
//////////////////////////////////////////////////////////////////////////////
/// @return @c true if everything went well; otherwise, @c false.
//////////////////////////////////////////////////////////////////////////////
//@test ''%p,%p,%p'' parent_attribute,meta_attribute,value
//@null 0
int siox_ontology_set_meta_attribute( siox_attribute * parent_attribute, siox_attribute * meta_attribute, const void * value );

/*
 * Retrieve an attribute by its name.
 * @todo Use its attributes to retrieve it.
 *
 * @param [in]   domain     The domain the attribute belongs to.
 * @param [in]   name       The metric's full qualified name.
 *
 * @returns                 The @em siox_metric with the name given or NULL, if no such metric
 *                          could be found.
 */
//////////////////////////////////////////////////////////////////////////////
/// MZ: Nonsense right now; identical to ..._register_... w/o the check
/// for existence! Should maybe look up the att's *value* or return the whole
/// attribute object, as we still lack functions for both?!
//////////////////////////////////////////////////////////////////////////////
/// @param domain
/// @param name
//////////////////////////////////////////////////////////////////////////////
/// @return
//////////////////////////////////////////////////////////////////////////////
//@test ''%s,%s'' domain,name
//@null  (siox_attribute*) name
siox_attribute * siox_ontology_lookup_attribute_by_name( const char * domain, const char * name );

/*
 * Example: interface_name POSIX, MPI, MPIv3 ...
 * Interface is "POSIX"
 * Implementation could be MPICHv3 (major number, please ensure compatibility)
 * If implementation_identifier is not set, ANY implementation of the interface is valid.
 */
//////////////////////////////////////////////////////////////////////////////
/// Find the (or, if necessary, create a) unique ID for a given interface
/// and implementation identifier.
//////////////////////////////////////////////////////////////////////////////
/// @param interface_name
/// @param implementation_identifier
//////////////////////////////////////////////////////////////////////////////
/// @return An ID that will be unique system-wide.
//////////////////////////////////////////////////////////////////////////////
//@test ''%s,%s'' interface_name,implementation_identifier
//@null
siox_unique_interface * siox_system_information_lookup_interface_id( const char * interface_name, const char * implementation_identifier );

/**
 * Report performance data @em not associated with a single activity.
 *
 * @em Example:   Every second, the component reports its average processor load,
 *                maximum memory usage and total idle time to SIOX.
 *
 * @param[in]   hw   The Node to report for.
 * @param[in]   metric     The metric.
 * @param[in]   value   A pointer to the metric's actual value.
 *
 * The value describes the difference / delta of the statistic metric between start and end, e.g. 54 MiB has been read between start and end.
 *
 */
//#test ''%p,%p,%p'' component,metric,value
//void siox_report_node_statistics(siox_node hw, siox_attribute * statistic, siox_timestamp start_of_interval, siox_timestamp end_of_interval, void * value);


/**
 * @name Components
 * Components represent entities in the IOPm graph, e.g., a software layer such as HDF5 or
 * a hardware component such as a block storage device.
 *
 * A component's typical life cycle would consist of:
 * <ol>
 * <li>Checking in with SIOX in its initalisation function via siox_component_register().</li>
 * <li>Also there, reporting any additional attributes via siox_component_set_attribute().</li>
 * <li>Performing its duties as usual.</li>
 * <li>Checking out with SIOX in its finalization function via siox_component_unregister().</li>
 * </ol>
 */
/**@{*/


/*
 * This function creates a process-local ID in the association mapper and relates it to the string.
 * Therefore, subsequent usages of the string can be replaced by the id.
 * The same string may be linked to the same ID.
 */
//////////////////////////////////////////////////////////////////////////////
/// Associate some instance information (such as IP port or thread ID) with
/// the current invocation.
//////////////////////////////////////////////////////////////////////////////
/// @param instance_information [in]
//////////////////////////////////////////////////////////////////////////////
/// @return
//////////////////////////////////////////////////////////////////////////////
//@test ''%s'' instance_information
//@null
siox_associate * siox_associate_instance( const char * instance_information );

/*
 * Register this component with SIOX.
 *
 * SIOX will use the information given to create a new component in its IOPm model of the system.
 *
 * @param[in]   uid    The component's @em SoftwareInterface identifier, e.g. „MPI2“ oder „POSIX“.
 * @param[in]   instance_name     The component's @em InstanceID (according to component type,
 *                                e.g. the ProcessID of a Thread or the IP address and Port of
 *                                a server cache).
 * These arguments provide a human readable name for the software layer and are used for matching of remote calls.
 *
 * @return  A fresh @em siox_component to be used in all the component's future communications
 *          with SIOX.
 *
 *
 */
//////////////////////////////////////////////////////////////////////////////
/// Register the current component with SIOX.
//////////////////////////////////////////////////////////////////////////////
/// @param uiid [in] The unique ID of the component's interface (such as
///     POSIX or MPI-I/O)
/// @param instance_name [in] Further information identifying the component's
///     current instance; this may be @c NULL.
//////////////////////////////////////////////////////////////////////////////
/// @return A pointer to the component's information, as represented by SIOX
//////////////////////////////////////////////////////////////////////////////
/// @note Any parameter SIOX can find out on its own (such as a ProcessID)
/// may be @c NULL.
//////////////////////////////////////////////////////////////////////////////

//@test ''%p,%s'' uiid,instance_name
//@null  (siox_component*) instance_name
siox_component * siox_component_register( siox_unique_interface * uiid, const char * instance_name );

int siox_component_is_registered( siox_unique_interface * uiid );

/**
 * Report an attribute of this component to SIOX.
 *
 * @em Example:    A cache reporting its capacity as 10,000 bytes.
 *
 * @param[in]   component   The component.
 * @param[in]   attribute   The attribute to be set.
 * @param[in]   value       A pointer to the attribute's value.
 */
//@test ''%p,%p,%p'' component,attribute,value
void siox_component_set_attribute( siox_component * component, siox_attribute * attribute, const void * value );

/**
 * Register a potential activity on a component.
 */
//////////////////////////////////////////////////////////////////////////////
/// Register a component-activity-type with SIOX.
//////////////////////////////////////////////////////////////////////////////
/// @param uiid [in]
/// @param activity_name [in]
//////////////////////////////////////////////////////////////////////////////
/// @return
//////////////////////////////////////////////////////////////////////////////
//@test ''%p,%s'' uiid,activity_name
//@null (siox_component_activity*) activity_name
siox_component_activity * siox_component_register_activity( siox_unique_interface * uiid, const char * activity_name );

/**
 * Unregister a component with SIOX.
 *
 * @param[in]   component   The component.
 */
//@test ''%p' component
void siox_component_unregister( siox_component * component );


/**@}*/




/**
 * @name Activities
 * Activities bundle a series of actions so that they can be related to one or more
 * performance metrics of the system influenced by these actions.
 *
 * As an example, an activity might bundle calls to open a file, write to it and close it again.
 * This way, the performance metrics of maximum throughput and average write speed can be linked
 * to this chain of actions.
 *
 * When defining an activity, the usual course of action is as follows:
 * <ol>
 * <li>Start the activity in its active phase via siox_activity_start().</li>
 * <li>Execute any usual commands, calls, etc. pertaining to activity.</li>
 * <li>End active phase and switch to reporting phase via siox_activity_stop().</li>
 * <li>Gather any performance data associated with the activity and report it
 * via siox_activity_report().</li>
 * <li>Close the activity via siox_activity_end().</li>
 * </ol>
 */
/**@{*/

/**
 * Report the start of an activity.
 *
 * You may report attributes such as function parameters then.
 * 
 * SIOX will use the @em siox_activity to correctly assign attributes used and performance
 * metrics influenced by this activity.
 * As any activity is linked to its component by SIOX, functions supplied with a
 * @em siox_activity do not use a @em siox_component.
 *
 *
 * @param[in]   component  The component carrying out the activity.
 * @param[in]   activity   The activity's component-activity-type.
 * @return                  A fresh @em siox_activity, to be used in all the activity's
 *                          further dealings with SIOX.
 */
//@test ''%p,%p'' component,activity
//@null
siox_activity * siox_activity_begin( siox_component * component, siox_component_activity * activity );

/**
 * Report the start of an activity's active phase, beginning time measurement.
 * @param[in]   activity    The activity.
 */
//@test ''%p'' activity
void siox_activity_start( siox_activity * activity );

/**
 * Report the end of an activity's active phase, beginning its reporting phase.
 *
 * This will mark the time stamp given as the end of the activity, influencing any
 * performance metrics associated that are measured over time.
 *
 * @param[in]   activity    The activity.
 */
//@test ''%p'' activity
void siox_activity_stop( siox_activity * activity );

/**
 * Report an attribute of an activity.
 *
 * Attributes for activities can be either its parameters and other values computed from them,
 * or metrics and statistics resulting from the call.
 *
 * @param[in]   activity    The activity.
 * @param[in]   attribute   The attribute.
 * @param[in]   value       A pointer to the attribute's value.
 */
//@test ''%p,%p,%p'' activity,attribute,value
void siox_activity_set_attribute( siox_activity * activity, siox_attribute * attribute, const void * value );

/**
 * Report that the current call resulted in the error code @em error so that SIOX can mark any
 * performance metrics gathered accordingly.
 * This should be done as part of any regular error processing by calls issued by the activity.
 *
 * @param[in]   activity    The activity.
 * @param[in]   error       The error code returned by the function. A value of 0 indicates SUCCESS.
 */
//@test ''%p,%d'' activity,error
void siox_activity_report_error( siox_activity * activity, siox_activity_error error );

/**
 * Mark the end of an activity's report phase and close it.
 *
 * After calling this function, the behaviour of further calls to siox_activity_report()
 * with the same activity will be undefined.
 *
 * @param[in]   activity    The activity.
 */
//@test ''%p'' activity
void siox_activity_end( siox_activity * activity );

/**
 * Causally link an activity to another.
 *
 * This will enable SIOX to relate the activities' influence on system performance.
 * While functions directly called by other functions can be linked by SIOX automatically,
 * the relation between two calls related "horizontally" (such as one to open a file
 * and one to write to it) is hard to impossible to determine automatically.
 * This function will give SIOX the information necessary to do so.
 *
 * @param[in]   activity_child   The current activity.
 * @param[in]   activity_parent  An activity causally preceding the current one.
 */
//@test ''%p,%p'' activity_child,activity_parent
void siox_activity_link_to_parent( siox_activity * activity_child, siox_activity_ID * activity_parent );


/*
 The ownership of the returned datastructure is given to the caller.
 */
//@test ''%p'' activity
//@null
siox_activity_ID * siox_activity_get_ID( const siox_activity * activity );

// TODO maybe just using ID?
//void siox_activity_link_to_parent(siox_activity * activity_child, siox_activity_ID?);


/**
 * @name Remote Calls
 * Remote calls allow SIOX to reconstruct causal relations of caller and callee
 * via physical system boundaries.
 *
 * While locally, SIOX can follow caller-callee relations by itself, for calls crossing
 * physical system boundaries, this is much harder.
 * The path chosen here requires both caller and callee to report to SIOX the attributes
 * of the call (such as key parameters passed) to allow for association by best match.
 *
 * The usual sequence is as follows:
 * <ul>
 * <li>On caller's side:</li>
 * <ol>
 * <li>Open attribute list and indicate target via siox_remote_call_setup().</li>
 * <li>Report attributes to be passed via siox_remote_call_set_attribute().</li>
 * <li>Indicate start of remote call via siox_remote_call_start().</li>
 * <li>Call callee.</li>
 * </ol>
 * <li>On callee's side:</li>
 * <ol>
 * <li>Indicate activity triggered by a remote call via siox_activity_start_from_remote_call().</li>
 * <li>Execute usual code.</li>
 * <li>siox_activity_stop().</li>
 * <li>siox_activity_set_attribute().</li>
 * <li>siox_activity_end().</li>
 * </ol>
 * </ul>
 */
/**@{*/

/**
 * Open attribute list for a remote call, indicate its target and receive @em RCID.
 *
 * @param[in]   activity
 * @param[in]   target_node   The target component's @em NodeID (e.g. „Blizzard Node 5“ or the MacID
 *                            of an NAS hard drive), if known; otherwise, @c NULL.
 * @param[in]   target_unique_interface   The target component's @em Software interface (e.g. „MPI“ oder „POSIX“),
 *                            if known; otherwise, @c NULL.
 * @param[in]   target_associate    The target component's @em InstanceID (according to component type,
 *                            e.g. the ProcessID of a Thread or the IP address and Port
 *                            of a server cache), if known; otherwise, @c NULL.
 *
 * @return                    A fresh @em RCID to be used in all the remote call's
 *                            future communications with SIOX.
 */
//@test ''%p,%p-%p-%p'' activity,target_node,target_unique_interface,target_associate
//@null
siox_remote_call * siox_remote_call_setup( siox_activity * activity, siox_node * target_node, siox_unique_interface * target_unique_interface, siox_associate * target_associate );

/**
 * Report an attribute to be sent via a remote call.
 *
 * @param[in]   remote_call    The remote call.
 * @param[in]   attribute   The attribute.
 * @param[in]   value       A pointer to the attribute's value.
 */
//@test ''%p,%p,%p'' remote_call,attribute,value
void siox_remote_call_set_attribute( siox_remote_call * remote_call, siox_attribute * attribute, const void * value );

/**
 * Close attribute list for a remote call.
 * This is necessary as there may be various remote calls originating from a single activity.
 *
 * @param[in]   remote_call    The remote call.
 */
//@test ''%p'' remote_call
void siox_remote_call_start( siox_remote_call * remote_call );


// if the out_value is of type string, a pointer of type char** must be provided. The ownership is given to the callee.
// return true if an optimal value is found
//@test ''%p,%p,%p'' component,attribute,out_value
//@null 0
int siox_suggest_optimal_value( siox_component * component, siox_attribute * attribute, void * out_value );

// copy the string value into target_str with the maxLength.
//@test ''%p,%p,%s,%d'' component,attribute,target_str,maxLength
//@null 0
int siox_suggest_optimal_value_str( siox_component * component, siox_attribute * attribute, char * target_str, int maxLength );


// This function is usefull to give the optimizer more insight what is going on.
// Especially, if parameters need to be changed for the call/activity to make.
// If the out_value is of type string, a pointer of type char** must be provided. The ownership is given to the callee.
// return true if an optimal value is found
//@test ''%p,%p,%p'' component,attribute,out_value
//@null 0
int siox_suggest_optimal_value_for( siox_component * component, siox_attribute * attribute, siox_activity * activity, void * out_value );

//////////////////////////////////////////////////////////////////////////////
/// Report that a local activity was initiated via a remote call.
/// While during a single activity, many remote calls may be sent, only one
/// (the one initiating the current function invocation) may be received.
/// Hence, on the callee's side, the delimiting functions
/// siox_remote_call_start() and siox_remote_call_submitted() are not needed.
/// Also, its defining attributes are set for the activity, so no
/// siox_remote_call is involved on the callee's side at all!
//////////////////////////////////////////////////////////////////////////////
/// @param [in] component The component on which the activity is carried out
/// @param [in] activity The activity started by remote call
/// @param [in] caller_node May be @c NULL
/// @param [in] caller_unique_interface May be @c NULL
/// @param [in] caller_associate May be @c NULL
//////////////////////////////////////////////////////////////////////////////
//@test ''%p,%p,%p,%p,%p'' component,activity,caller_node,caller_unique_interface,caller_associate
//@null
siox_activity * siox_activity_start_from_remote_call( siox_component * component, siox_component_activity * activity, siox_node * caller_node, siox_unique_interface * caller_unique_interface, siox_associate * caller_associate );

//@test ''%p'' func
void siox_register_termination_signal( void (*func)(void) );

//@test ''%p'' func
void siox_register_termination_complete_signal( void (*func)(void) );

//@test ''%p'' func
void siox_register_initialization_signal( void (*func)(void) );

#endif
