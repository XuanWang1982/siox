/**
 * This file contains structures and constructors for SIOX ID, e.g.
 * hardware, processes, components and attributes.
 * @Author JK
 */

#ifndef SIOX_IDS_SERIALIZE_H
#define SIOX_IDS_SERIALIZE_H

#include <monitoring/datatypes/ids.hpp>

#define SER(x,y) ar & boost::serialization::make_nvp(x, y);

namespace monitoring{
	template<class Archive>
	void serialize(Archive & ar, ComponentID & id, const unsigned int file_version){
		SER("pid", id.pid)
		SER("uuid", id.uuid)
	}

	template<class Archive>
	void serialize(Archive & ar, UniqueInterfaceID & id, const unsigned int file_version){
		SER("i", id.interface)
		SER("v", id.implementation)
	}


	template<class Archive>
	void serialize(Archive & ar, ProcessID & id, const unsigned int file_version){
		SER("hw", id.hw)
		SER("pid", id.pid)
		SER("t", id.time);
	}


}

#endif