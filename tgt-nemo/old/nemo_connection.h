#ifndef __NEMO_CONNECTION_HEADER__
#define __NEMO_CONNECTION_HEADER__

#include "nemo_connection.pb.h"
#include <google/protobuf/io/coded_stream.h>

using namespace nemo_pb;
using namespace google::protobuf::io;

class Nemo_Connection{
	public:
		Nemo_Connection();
		Nemo_Connection(unsigned int s1, unsigned int s2);
		~Nemo_Connection();
		bool write_pb_to_file(CodedOutputStream* strm);
		bool read_pb_from_file(CodedInputStream* strm);
	private:
		Nemo_Connection_PB* connection;
};

#endif