#ifndef CONTAINER_BINARY_SERIALIZER_H
#define CONTAINER_BINARY_SERIALIZER_H

/* 
 * This include files contains functions for (de)serialization of primitive types.
 */

#include <assert.h>
#include <string.h>
#include <string>
#include <inttypes.h>

#include <exception>

using namespace std;

#define CHECK_LENGTH(COUNT) assert(pos + COUNT <= length);

namespace j_serialization{

inline void serialize(const uint8_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos++;
}

inline void serialize(const int8_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos++;
}

inline void serialize(const int16_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 2;
}

inline void serialize(const uint16_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 2;
}

inline void serialize(const int32_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 4;
}

inline void serialize(const uint32_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 4;
}

inline void serialize(const float & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 4;
}

inline void serialize(const int64_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 8;
}

inline void serialize(const uint64_t & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 8;
}

inline void serialize(const double & obj, char * buffer, uint64_t & pos){
	buffer[pos] = obj;
	pos += 4;
}

inline void serialize(const long double & obj, char * buffer, uint64_t & pos){	
	memcpy(& buffer[pos], & obj, sizeof(obj) );
	pos += sizeof(obj);
}


inline void deserialize(uint8_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(1)
	obj = buffer[pos];
	pos++;	
}

inline void deserialize(int8_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(1)
	obj = buffer[pos];
	pos++;	
}

inline void deserialize(int16_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(2)
	obj = buffer[pos];
	pos += 2;	
}

inline void deserialize(uint16_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(2)
	obj = buffer[pos];
	pos += 2;	
}

inline void deserialize(int32_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(4)
	obj = buffer[pos];
	pos += 4;	
}

inline void deserialize(uint32_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(4)
	obj = buffer[pos];
	pos += 4;	
}

inline void deserialize(float & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(4)
	obj = buffer[pos];
	pos += 4;	
}

inline void deserialize(int64_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(8)
	obj = buffer[pos];
	pos += 8;	
}

inline void deserialize(uint64_t & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(8)
	obj = buffer[pos];
	pos += 8;	
}

inline void deserialize(double & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(8)
	obj = buffer[pos];
	pos += 4;	
}

inline void deserialize(long double & obj, const char * buffer, uint64_t & pos, uint64_t length){
	CHECK_LENGTH(sizeof(obj))
	memcpy(& obj, & buffer[pos], sizeof(obj) );
	pos += sizeof(obj);	
}

inline uint64_t serializeLen(const uint32_t & obj){
	return 4;
}

inline uint64_t serializeLen(const uint64_t & obj){
	return 8;
}

/////////// STRING /////////////////////////

inline uint64_t serializeLen(const string & obj){
	uint32_t len = 0;
	return serializeLen(len) + obj.length();
}

using namespace std;

inline void serialize(const string & obj, char * buffer, uint64_t & pos){
	uint32_t len = obj.length();
	serialize(len, buffer, pos);
	memcpy(buffer + pos, obj.c_str(), len);
	pos += len;	
}

inline void deserialize(string & obj, const char * buffer, uint64_t & pos, uint64_t length){
	uint32_t len;
	CHECK_LENGTH(4)
	deserialize(len, buffer, pos, length);

	//cout << len << " " << (& buffer[pos]) << " " << pos << endl;
	CHECK_LENGTH(len)
    obj.resize(0);
	obj.append(& buffer[pos], len);
	pos += len;
}



}

#undef CHECK_LENGTH

#endif