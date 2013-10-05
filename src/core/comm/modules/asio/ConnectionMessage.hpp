#ifndef CONNECTION_MESSAGE_H
#define CONNECTION_MESSAGE_H

/**
 * Helper class to use Google Protocol Buffers with Boost::asio by prepending
 * a length-prefixed buffer.
 *
 * This code is derived from sample code provided by Eli Bendersky.
 *
 * @author Alvaro Aguilera
 *
 * @sa http://eli.thegreenplace.net/2011/03/20/boost-asio-with-protocol-buffers-code-sample/
 */

#include <cstdio>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include "siox.pb.h"

typedef std::vector<boost::uint8_t> data_buffer;


/** Generic function to show contents of a container holding byte data
 *  as a string with hex representation for each byte.
 */
template <class CharContainer>
std::string show_hex( const CharContainer & c )
{
	std::string hex;
	char buf[16];

	typename CharContainer::const_iterator i;
	for( i = c.begin(); i != c.end(); ++i ) {
		std::sprintf( buf, "%02X ", static_cast<unsigned>( *i ) & 0xFF );
		hex += buf;
	}

	return hex;
}


/** The header size for packed messages */
const unsigned HEADER_SIZE = 4;

/** ConnectionMessage implements a simple packing of protocol buffers messages
 * into a string prepended by a header specifying the message length. SioxBuffer
 * should be a message class generated by the protobuf compiler.
 */

class ConnectionMessage {

	public:
		typedef boost::shared_ptr<buffers::MessageBuffer> MessagePointer;

		ConnectionMessage( MessagePointer msg  = MessagePointer() );

		ConnectionMessage( const ConnectionMessage & cm );

		void set_msg( MessagePointer msg );
		MessagePointer get_msg() const;

		/** Packs the message into the given data_buffer. The buffer is resized
		 * to exactly fit the message.
		 *
		 * @return False in case of an error, true if successful.
		 */
		bool pack( data_buffer & buf ) const;

		/** Given a buffer with the first HEADER_SIZE bytes representing the
		 * header, decode the header and return the message length.
		 *
		 * @return 0 in case of an error.
		 * */
		unsigned decode_header( const data_buffer & buf ) const;

		/** Unpacks and store a message from the given packed buffer.
		 *
		 * @return True if unpacking successful, false otherwise.
		 */
		bool unpack( const data_buffer & buf );

	private:
		MessagePointer msg_;

		/** Encodes the side into a header at the beginning of buf.
		 */
		void encode_header( data_buffer & buf, unsigned size ) const;

};

#endif