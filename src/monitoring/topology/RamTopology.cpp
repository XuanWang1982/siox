/**
 * A Topology implementation that keeps all data in main memory.
 *
 * @author Nathanael Hübbe
 * @date 2013
 */

#include <util/ExceptionHandling.hpp>
#include <monitoring/topology/TopologyImplementation.hpp>
#include <workarounds.hpp>

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>

#include "RamTopologyOptions.hpp"

using namespace std;
using namespace core;
using namespace monitoring;


class RamTopology : public Topology {
	public:
		RamTopology();

		virtual void init();
		virtual ComponentOptions * AvailableOptions();

		virtual TopologyType registerType( const string& name ) throw();
		virtual TopologyType lookupTypeByName( const string& name ) throw( NotFoundError );
		virtual TopologyType lookupTypeById( TopologyTypeId anId ) throw( NotFoundError );

		virtual TopologyObject registerObject( TopologyObjectId parent, TopologyTypeId objectType, TopologyTypeId relationType, const string& childName ) throw( IllegalStateError );
		virtual TopologyObject lookupObjectByPath( const string& Path ) throw( NotFoundError );
		virtual TopologyObject lookupObjectById( TopologyObjectId anId ) throw( NotFoundError );

		virtual TopologyRelation registerRelation( TopologyTypeId relationType, TopologyObjectId parent, TopologyObjectId child, const string& childName ) throw( IllegalStateError );
		virtual TopologyRelation lookupRelation( TopologyObjectId parent, const string& childName ) throw( NotFoundError );

		// TopologyRelationType may be 0 which indicates any parent/child.
		virtual TopologyRelationList enumerateChildren( TopologyObjectId parent, TopologyTypeId relationType ) throw();
		virtual TopologyRelationList enumerateParents( TopologyObjectId child, TopologyTypeId relationType ) throw();


		virtual TopologyAttribute registerAttribute( TopologyTypeId domain, const string& name, VariableDatatype::Type datatype ) throw( IllegalStateError );
		virtual TopologyAttribute lookupAttributeByName( TopologyTypeId domain, const string& name ) throw( NotFoundError );
		virtual TopologyAttribute lookupAttributeById( TopologyAttributeId attributeId ) throw( NotFoundError );
		virtual void setAttribute( TopologyObjectId object, TopologyAttributeId attribute, const TopologyValue& value ) throw( IllegalStateError );
		virtual TopologyValue getAttribute( TopologyObjectId object, TopologyAttributeId attribute ) throw( NotFoundError );
		virtual TopologyValueList enumerateAttributes( TopologyObjectId object ) throw();

	private:
		//These classes are just combinations of a container and a lock that protects it.
		class ChildMap : public unordered_map<string, TopologyRelation>, public boost::shared_mutex {};
		class ParentVector : public vector<TopologyRelation>, public boost::shared_mutex {};

		boost::shared_mutex typesLock, objectsLock, relationsLock, valuesLock, attributesLock, childsLock;

		unordered_map<string, TopologyType> typesByName;	//Protected by typesLock.
		vector<TopologyType> typesById;	//Protected by typesLock.

		vector<TopologyObject> objectsById;	//Protected by objectsLock.
		vector<ChildMap*> childMapsById;	//Protected by objectsLock.
		vector<ParentVector*> parentVectorsById;	//Protected by objectsLock.

		unordered_map<pair<TopologyTypeId, string>, TopologyAttribute> attributesByName;	//Protected by attributesLock.
		vector<TopologyAttribute> attributesById;	//Protected by attributesLock.
};


RamTopology::RamTopology() {
	//Add dummy objects to the ...ById vectors, so that we don't have to differentiate between IDs and indices.
	typesById.resize( 1 );
	objectsById.resize( 1 );
	attributesById.resize( 1 );
}


void RamTopology::init() {
}


ComponentOptions* RamTopology::AvailableOptions() {
	return new RamTopologyOptions;
}


TopologyType RamTopology::registerType( const string& name ) throw() {
	TopologyType result;
	typesLock.lock_shared();
	IGNORE_EXCEPTIONS( result = typesByName.at( name ); );
	typesLock.unlock_shared();
	if( !result ) {
		Release<TopologyTypeImplementation> newType( new TopologyTypeImplementation( name ) );	//The new operator can take a _lot_ of time, don't let other processes wait for that!

		typesLock.lock();
		IGNORE_EXCEPTIONS( result = typesByName.at( name ); );	//Someone else might have registered it in the mean time.
		if( !result ) {
			newType->id = typesById.size();
			typesById.resize( typesById.size() + 1 );
			typesById.back().setObject( newType );
			typesByName[name].setObject( newType );
			result.setObject( newType );
		}
		typesLock.unlock();
	}
	return result;
}


TopologyType RamTopology::lookupTypeByName( const string& name ) throw( NotFoundError ) {
	TopologyType result;
	typesLock.lock_shared();
	IGNORE_EXCEPTIONS( result = typesByName.at( name ); );
	typesLock.unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


TopologyType RamTopology::lookupTypeById( TopologyTypeId anId ) throw( NotFoundError ) {
	if( !anId ) throw NotFoundError();
	TopologyType result;
	typesLock.lock_shared();
	if( anId < typesById.size() ) result = typesById[anId];
	typesLock.unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


TopologyObject RamTopology::registerObject( TopologyObjectId parentId, TopologyTypeId objectType, TopologyTypeId relationType, const string& childName ) throw( IllegalStateError ) {
	//First look up the child map in which the requested relation should be stored.
	ChildMap* childMap = NULL;
	objectsLock.lock_shared();
	if( parentId < objectsById.size() ) childMap = childMapsById[parentId];
	objectsLock.unlock_shared();
	if( !childMap ) throw IllegalStateError( "RamTopology::registerObject(): the requested parent does not exist" );

	//Look up the relation, and create it if it doesn't exist.
	TopologyRelation resultRelation;
	TopologyObject result;
	childMap->lock_shared();
	IGNORE_EXCEPTIONS( resultRelation = childMap->at( childName ); );
	childMap->unlock_shared();
	if( !resultRelation ) {
		//Looks like we have to allocate something
		Release<TopologyRelationImplementation> newRelation( new TopologyRelationImplementation( childName, parentId, 0, relationType ) );
		Release<TopologyObjectImplementation> newObject( new TopologyObjectImplementation( objectType ) );
		ChildMap* newChildMap( new ChildMap() );
		ParentVector* newParentVector( new ParentVector() );
		newParentVector->resize( 1 );
		(*newParentVector)[0].setObject( newRelation );

		//Regrap the locks for writing, and check if someone else was quicker in registering this object.
		objectsLock.lock();
		childMap->lock();
		IGNORE_EXCEPTIONS( resultRelation = childMap->at( childName ); );
		if( !resultRelation ) {
			//Finalize the object description.
			newRelation->childId = newObject->id = objectsById.size();
			size_t objectCount = objectsById.size() + 1;

			//Make the new object available.
			objectsById.resize( objectCount );
			objectsById.back().setObject( newObject );
			childMapsById.resize( objectCount );
			childMapsById.back() = newChildMap;
			parentVectorsById.resize( objectCount );
			parentVectorsById.back() = newParentVector;
			(*childMap)[childName].setObject( newRelation );

			//Stuff for our personal cleanup.
			resultRelation.setObject( newRelation );
			result.setObject( newObject );
			newChildMap = NULL;
			newParentVector = NULL;
		}
		childMap->unlock();
		objectsLock.unlock();

		delete newChildMap;
		delete newParentVector;
	}

	//In case we succeeded in looking up the object, actually lookup the TopologyObject itself, and check consistency.
	if( !result ) {
		if( resultRelation.type() != relationType ) throw IllegalStateError( "RamTopology::registerObject(): The requested relation type does not match with what is already registered." );
		objectsLock.lock_shared();
		result = objectsById[resultRelation.child()];
		objectsLock.unlock_shared();
		if( result.type() != objectType ) throw IllegalStateError( "RamTopology::registerObject(): The requested object type does not match with what is already registered." );
	}
	return result;
}


TopologyObject RamTopology::lookupObjectByPath( const string& Path ) throw( NotFoundError ) {
	assert(0 && "TODO"), abort();
}


TopologyObject RamTopology::lookupObjectById( TopologyObjectId anId ) throw( NotFoundError ) {
	TopologyObject result;
	objectsLock.lock_shared();
	if( anId < objectsById.size() ) result = objectsById[anId];
	objectsLock.unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


TopologyRelation RamTopology::registerRelation( TopologyTypeId relationType, TopologyObjectId parent, TopologyObjectId child, const string& childName ) throw( IllegalStateError ) {
	assert(0 && "TODO"), abort();
}


TopologyRelation RamTopology::lookupRelation( TopologyObjectId parent, const string& childName ) throw( NotFoundError ) {
	ChildMap* childMap = NULL;
	objectsLock.lock_shared();
	if( parent < objectsById.size() ) childMap = childMapsById[parent];
	objectsLock.unlock_shared();
	if( !childMap ) throw NotFoundError();

	TopologyRelation result;
	childMap->lock_shared();
	IGNORE_EXCEPTIONS( result = childMap->at( childName ); );
	childMap->unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


Topology::TopologyRelationList RamTopology::enumerateChildren( TopologyObjectId parent, TopologyTypeId relationType ) throw() {
	assert(0 && "TODO"), abort();
}


Topology::TopologyRelationList RamTopology::enumerateParents( TopologyObjectId child, TopologyTypeId relationType ) throw() {
	assert(0 && "TODO"), abort();
}


TopologyAttribute RamTopology::registerAttribute( TopologyTypeId domain, const string& name, VariableDatatype::Type datatype ) throw( IllegalStateError ) {
	TopologyAttribute result;
	pair<TopologyTypeId, string> key( domain, name );
	attributesLock.lock_shared();
	IGNORE_EXCEPTIONS( result = attributesByName.at( key ); );
	attributesLock.unlock_shared();
	if( !result ) {
		Release<TopologyAttributeImplementation> newAttribute( new TopologyAttributeImplementation( name, domain, datatype ) );	//Don't let other processes wait for this.
		attributesLock.lock();
		IGNORE_EXCEPTIONS( result = attributesByName.at( key ); );	//Someone else might have registered it in the mean time.
		if( !result ) {
			newAttribute->id = attributesById.size();
			attributesById.resize( attributesById.size() + 1 );
			attributesById.back().setObject( newAttribute );
			attributesByName[key].setObject( newAttribute );
			result.setObject( newAttribute );
		}
		attributesLock.unlock();
	}
	if( result.dataType() != datatype ) throw IllegalStateError( "RamTopology::registerAttribute(): requested attribute datatype does not match previously registered one." );
	return result;
}


TopologyAttribute RamTopology::lookupAttributeByName( TopologyTypeId domain, const string& name ) throw( NotFoundError ) {
	TopologyAttribute result;
	pair<TopologyTypeId, string> key( domain, name );
	attributesLock.lock_shared();
	IGNORE_EXCEPTIONS( result = attributesByName.at( key ); );
	attributesLock.unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


TopologyAttribute RamTopology::lookupAttributeById( TopologyAttributeId attributeId ) throw( NotFoundError ) {
	if( !attributeId ) throw NotFoundError();
	TopologyAttribute result;
	attributesLock.lock_shared();
	if( attributeId < attributesById.size() ) result = attributesById[attributeId];
	attributesLock.unlock_shared();
	if( !result ) throw NotFoundError();
	return result;
}


void RamTopology::setAttribute( TopologyObjectId object, TopologyAttributeId attribute, const TopologyValue& value ) throw( IllegalStateError ) {
	assert(0 && "TODO"), abort();
}


TopologyValue RamTopology::getAttribute( TopologyObjectId object, TopologyAttributeId attribute ) throw( NotFoundError ) {
	assert(0 && "TODO"), abort();
}


Topology::TopologyValueList RamTopology::enumerateAttributes( TopologyObjectId object ) throw() {
	assert(0 && "TODO"), abort();
}


extern "C" {
	void* TOPOLOGY_INSTANCIATOR_NAME()
	{
		return new RamTopology();
	}
}
