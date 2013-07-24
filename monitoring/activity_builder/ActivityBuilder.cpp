#include <monitoring/activity_builder/ActivityBuilder.hpp>
#include <monitoring/helpers/siox_time.h>

using namespace std;

static void fillRemoteCallIdentifier(RemoteCallIdentifier &rcid, NodeID* nid, UniqueInterfaceID* uiid, AssociateID* assid)
{
	if(nid != NULL) {
		rcid.hwid = *nid;
	}
	else {
		rcid.hwid = 0;
	}
	if(uiid != NULL) {
		rcid.uuid = *uiid;
	}
	else {
		rcid.uuid.implementation = 0;
		rcid.uuid.interface = 0;
	}
	if(assid != NULL) {
		rcid.instance = *assid;
	}
	else {
		rcid.instance = 0;
	}
}


namespace monitoring {

ActivityBuilder::ActivityBuilder()
{
	next_activity_id = 1;
	printf( "ActivityBuilder %p ready.\n", this );
}

ActivityBuilder::~ActivityBuilder()
{
	printf( "ActivityBuilder %p has been shut down.\n", this );
}

ActivityBuilder* ActivityBuilder::getInstance()
{
	static __thread ActivityBuilder* myAB = NULL;
	if(myAB == NULL) {
		myAB = new ActivityBuilder;
	}
	return myAB;
}

Activity* ActivityBuilder::startActivity(ComponentID* cid, UniqueComponentActivityID* ucaid, siox_timestamp* t)
{
	uint32_t aid;
	Activity* a;

	assert(cid != NULL);
	assert(ucaid != NULL);

	aid = next_activity_id++;
	a = new Activity;
	a->aid_.id = aid;
	a->aid_.cid = *cid;
	a->ucaid_ = *ucaid;
	a->remoteInvoker_ = NULL;
	a->time_stop_ = 0;
	// Initially, we assume successful outcome of an actvity
	// a->errorValue_ = SIOX_ACTIVITY_SUCCESS;

	// Add this activity to the list of currently active activities
	activities_in_flight[aid] = a;

	// Get last activity in call stack of this thread
	if(!activity_stack.empty()) {
		a->parentArray_.push_back(activity_stack.back()->aid_);
	}
	activity_stack.push_back(a);

	if(t != NULL) {
		a->time_start_ = *t;
	}
	else {
		a->time_start_ = siox_gettime();
	}
	return a;
}

void ActivityBuilder::stopActivity(Activity* a, siox_timestamp* t)
{
	assert(a != NULL);

	if(t != NULL) {
		a->time_stop_ = *t;
	}
	else {
		a->time_stop_ = siox_gettime();
	}

	assert(a == activity_stack.back());
	activity_stack.pop_back();
}

/* endActivity: All data for this activity has been collected. The Activity can be constructed now and then be sent to the Muxer.
 */
void ActivityBuilder::endActivity(Activity* a)
{
	assert(a != NULL);

	// @TODO: Send "a" to Muxer
	// Remark: Muxer has to deallocate the instance

	// Remove Activity from in-flight list
	activities_in_flight.erase(a->aid_.id);
}

void ActivityBuilder::addActivityAttribute(Activity* a, Attribute *attribute)
{
	assert(a != NULL);
	assert(attribute != NULL);

	a->attributeArray_.push_back(*attribute);
}

void ActivityBuilder::reportActivityError(Activity* a, siox_activity_error error)
{
	assert(a != NULL);
	assert(a->errorValue_ == 0);

	a->errorValue_ = error;
}

void ActivityBuilder::linkActivities(Activity* child, ActivityID* parent)
{
	assert(child != NULL);
	assert(parent != NULL);

	child->parentArray_.push_back(*parent);
}

RemoteCall* ActivityBuilder::setupRemoteCall(Activity* a, NodeID* target_node_id, UniqueInterfaceID* target_unique_interface_id, AssociateID* target_associate_id)
{
	assert(a != NULL);

	RemoteCall* rc = new RemoteCall;
	rc->caller_activity = a;
	rc->target_node_id = target_node_id;
	rc->target_unique_interface_id = target_unique_interface_id;
	rc->target_associate_id = target_associate_id;

	return rc;
}

void ActivityBuilder::addRemoteCallAttribute(RemoteCall* rc, Attribute* attribute)
{
	assert(rc != NULL);
	assert(attribute != NULL);

	rc->attributes.push_back(*attribute);
}

void ActivityBuilder::startRemoteCall(RemoteCall* rc, siox_timestamp* t)
{
	assert(rc != NULL);

	RemoteCallID rcid;
	fillRemoteCallIdentifier(rcid.target, rc->target_node_id, rc->target_unique_interface_id, rc->target_associate_id);

	rc->caller_activity->remoteCallsArray_.push_back(rcid);
}

Activity* ActivityBuilder::startActivityFromRemoteCall(ComponentID* cid, UniqueComponentActivityID* ucaid, NodeID* nid, UniqueInterfaceID* uiid, AssociateID* assid, siox_timestamp* t)
{
	// REMARK: If t == NULL, then startActivity will draw the current timestamp and all code in this function will count towards the time of the activity. As of now, it will be left this way for the sake of simplicity.

	Activity* a;

	a = startActivity(cid, ucaid, t);
	a->remoteInvoker_ = new RemoteCallIdentifier;
	fillRemoteCallIdentifier(*(a->remoteInvoker_), nid, uiid, assid);

	return a;
}


}
