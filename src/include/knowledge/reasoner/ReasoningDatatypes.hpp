#ifndef KNOWLEDGE_REASONER_DATATYPES_HPP
#define KNOWLEDGE_REASONER_DATATYPES_HPP

#include <monitoring/datatypes/ids.hpp>

#include <list>
#include <array>
#include <unordered_map>
#include <sstream>

using namespace monitoring;

namespace knowledge{


//@serializable
enum HealthState{
	ABNORMAL_FAST = 0,
	FAST = 1,
	OK = 2,
	SLOW = 3,
	ABNORMAL_SLOW = 4,
	ABNORMAL_OTHER = 5,
	HEALTH_STATE_COUNT = 6
};


//@serializable
enum UtilizationIndex{
	CPU = 0,
	MEMORY = 1,
	IO = 2,
	NETWORK = 3,
	UTILIZATION_STATISTIC_COUNT = 4
};


//@serializable
struct HealthIssue{
	string name;

	uint32_t occurrences;

	// Estimation in wasted time in ms.
	// Could be negative because we win time due to fast occurrences.
	// How can we derive this value?
	int32_t delta_time_ms;

	bool operator==(const HealthIssue & hi) const{
		return this->name == hi.name && this->occurrences == hi.occurrences && this->delta_time_ms == hi.delta_time_ms;
	}

	void add( const HealthIssue & hi ) {
		if ( this->name == hi.name ){
			this->occurrences += hi.occurrences;
			this->delta_time_ms += hi.delta_time_ms;
		}
		else {
			cout << "Error: HealthIssue \"" << hi.name << "\" cannot be aggregated into \"" << this->name << "\"!" << endl;
		}
	}

	string to_string(){
		ostringstream result;

		result << "[\"" << name << "\", ";
        result << occurrences << "x, ";
	    result << delta_time_ms << " ms";
	    result << "]";

		return result.str();
	}

};


//@serializable
struct HealthIssueList{
	list<HealthIssue> issues;

	HealthIssueList() : issues() {}


	bool operator==( const HealthIssueList & hil ){
		return this->issues == hil.issues;
	}


	void add( const HealthIssue & hi ){
		for( auto itr = this->issues.begin(); itr != this->issues.end(); itr++ ){
			if( itr->name == hi.name ) {
				itr->add(hi);
				return;
			}
		}
		// Not found
		this->issues.push_back(hi);
	}


	void add( const HealthIssueList hil ){
		for( auto itr = hil.issues.begin(); itr != hil.issues.end(); itr++ ){
			this->add(*itr);
		}
	}


	string to_string(){
		ostringstream result;

		result << "[ " << issues.size() << "issues:" << endl;
        for( auto itr = issues.begin(); itr != issues.end(); itr++)
        	result << "\t" << itr->to_string() << endl;
	    result << "]";

		return result.str();
	}
};


struct HealthIssueMap{
	unordered_map<string, HealthIssue> issues;

	HealthIssueMap() : issues() {}

	bool operator==( const HealthIssueMap & him ){
		return this->issues == him.issues;
	}

	void add( const HealthIssueList & hil ){
		for( auto itr = hil.issues.begin(); itr != hil.issues.end(); itr++ ){
			auto issue = this->issues.find( itr->name );
			if( issue != this->issues.end() ){
				issue->second.add(*itr);
			}else{
				this->issues[itr->name] = *itr;
			}
		}
	}

	void add( const HealthIssueMap & him ){
		for( auto itr = him.issues.begin(); itr != him.issues.end(); itr++ ){
			auto issue = this->issues.find( itr->first );
			if( issue != this->issues.end() ){
				issue->second.add(itr->second);
			}else{
				this->issues[itr->first] = itr->second;
			}
		}
	}


	string to_string(){
		ostringstream result;

		result << "[ " << issues.size() << "issues:" << endl;
        for( auto itr = issues.begin(); itr != issues.end(); itr++)
        	result << "\t" << itr->second.to_string() << endl;
	    result << "]";

		return result.str();
	}
};


//@serializable
struct Health{
	HealthState overallState;

	array<uint32_t, HEALTH_STATE_COUNT> occurrences; // indexed by HealthState

	list<HealthIssue> positiveIssues;
	list<HealthIssue> negativeIssues;
};

typedef Health SystemHealth;

typedef Health ProcessHealth;

//@serializable
struct NodeHealth : public Health{
	array<uint8_t, UTILIZATION_STATISTIC_COUNT> utilization; // UtilizationIndex

	NodeHealth() : Health({HealthState::OK, {{0}}, {}, {} }), utilization({{0, 0, 0, 0}}) {}
};


/*
	struct HealthIssueWithExplaination : HealthIssue{
		// The node and global issues, that might explain the performance issues:
		unordered_map<string, HealthIssue> globalIssues;
		unordered_map<string, HealthIssue> nodeLocalIssues;
	};
*/

	/*
	 Remember and add the local issues forever.
	 */
/*
	struct HealthStatistic{
		UniqueInterfaceID cid; // can be mapped to the name later
		unordered_map<string, HealthIssueWithExplaination> issues;
	};
*/


// May be a good idea to provide an overloaded outputstream for the enums to make them "human" readable.
// TODO: Offer a CPP for this reason.
}


#endif
