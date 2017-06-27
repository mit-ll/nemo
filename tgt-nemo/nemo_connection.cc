#include "nemo_connection.h"

Nemo_Connection::Nemo_Connection(){
	connection = NULL;
}

Nemo_Connection::Nemo_Connection(unsigned int s1, unsigned int s2){
	connection = new Nemo_Connection_PB();
	connection->set_id_1(s1);
	connection->set_id_2(s2);
}

Nemo_Connection::~Nemo_Connection(){
	if (connection){
		delete connection;
	}
}

bool Nemo_Connection::write_pb_to_file(CodedOutputStream* strm){
	strm->WriteLittleEndian32(connection->ByteSize());
	if (strm->HadError()){
		return false;
	}
	if (!connection->SerializeToCodedStream(strm)){
		return false;
	}
	return true;
}

bool Nemo_Connection::read_pb_from_file(CodedInputStream* strm){
	unsigned int size;
	int 		 read_limit;
	
	// Read size of protobuf
	if (!strm->ReadLittleEndian32(&size)){
		return false;
	}

	// Set read byte limit
	read_limit = strm->PushLimit(size);
	
	// Parse the protobuf message
	if(!connection->ParseFromCodedStream(strm)){
		return false;
	}
	if (!strm->ConsumedEntireMessage()){
		return false;
	}
	strm->PopLimit(read_limit);
	return true;
}
