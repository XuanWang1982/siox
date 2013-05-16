#ifndef ACTIVITYMULTIPLEXER_IMPL1_H
#define ACTIVITYMULTIPLEXER_IMPL1_H 

#include <list>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "../../include/monitoring/activity_multiplexer/ActivityMultiplexer.hpp"

#include "ActivityMultiplexerListener_Impl1.hpp"


/**
 * ActivityMultiplexerQueue_Impl1 Considerations
 * 
 */
class ActivityMultiplexerQueue_Impl1 : public ActivityMultiplexerQueue
{
public:
	ActivityMultiplexerQueue_Impl1();
	virtual ~ActivityMultiplexerQueue_Impl1();

	bool Full();	
	void Push(Activity * activity);
	Activity * Pull();

private:
	// boost::message_queue? -> has priorities already built in
	std::queue<Activity*> activities;

	int capacity;

	boost::mutex xmutex;                               
	boost::condition_variable is_not_full;  
	boost::condition_variable is_not_empty;
};



class ActivityMultiplexerNotifier_Impl1 : public ActivityMultiplexerNotifier
{
public:
	ActivityMultiplexerNotifier_Impl1 ();
	ActivityMultiplexerNotifier_Impl1( 
			ActivityMultiplexerQueue * queue, 
			std::list<ActivityMultiplexerListener*> * list);

	virtual ~ActivityMultiplexerNotifier_Impl1 ();

	void setQueue(ActivityMultiplexerQueue * queue); 
	void setListenerList(std::list<ActivityMultiplexerListener*> * list);
	void Wake();

private:	
	ActivityMultiplexerQueue * activities;	
	std::list<ActivityMultiplexerListener*> * listeners_async;

};




class ActivityMultiplexer_Impl1 : public ActivityMultiplexer
{
public:
	ActivityMultiplexer_Impl1 ();
	virtual ~ActivityMultiplexer_Impl1 ();
	
	void Log(Activity * activity);

	void registerListener(ActivityMultiplexerListener * listener, bool async);
	void unregisterListener(ActivityMultiplexerListener * listener, bool async);

	void print();

private:
	// debug
	static int nextID;
	int ID;

	// queue
	std::list<ActivityMultiplexerListener*> listeners_sync;
	std::list<ActivityMultiplexerListener*> listeners_async;

	// async path
	ActivityMultiplexerQueue * activities;
	ActivityMultiplexerNotifier * notifier;
	
};


#endif /* ACTIVITYMULTIPLEXER_IMPL1_H */
