// Static Method
void Nemo_Connection::set_msb(unsigned int sig_num, unsigned long m){
	if (sig_num == 1){
		connection->set_msb_1(m);
	} else if (sig_num == 2){
		connection->set_msb_2(m);
	}
}

void Nemo_Connection::set_lsb(unsigned int sig_num, unsigned long l){
	if (sig_num == 1){
		connection->set_lsb_1(l);
	} else if (sig_num == 2){
		connection->set_lsb_2(l);
	}
}

bool Nemo_Connection::is_msb_modified(unsigned int sig_num){
	if (sig_num == 1){
		return connection->has_msb_1();
	} else if (sig_num == 2){
		return connection->has_msb_2();
	} else {
		return false;
	}
}

bool Nemo_Connection::is_lsb_modified(unsigned int sig_num){
	if (sig_num == 1){
		return connection->has_lsb_1();
	} else if (sig_num == 2){
		return connection->has_lsb_2();
	} else {
		return false;
	}
}

void Nemo_Design::serialize_pb_objs(const char* filename, vector<void*>* pb_objs_vector, unsigned int num_objs){
	int fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	ZeroCopyOutputStream* raw_output   = new FileOutputStream(fd);
	CodedOutputStream* 	  coded_output = new CodedOutputStream(raw_output);

	// Write Number of Messages to PB File
	coded_output->WriteLittleEndian32(num_objs);

	// Write Signal Objects to PB File
	for(unsigned int i = 0; i < pb_objs_vector->size(); i++){
		if (!(*pb_objs_vector)[i]->write_pb_to_file(coded_output)){
			fprintf(stderr, "ERROR: failed to write to protobuf file %s.\n", filename);
		 	exit(-4);
		}
	}
	
	delete coded_output;
	delete raw_output;
	close(fd);
}

// Static Method
void Nemo_Design::load_pb_objs(const char* filename, bool* loaded_from_pb, void* pb_objs_vector, unsigned int* num_objs){
	int fd = open(filename, O_RDONLY);
	ZeroCopyInputStream* raw_input   = new FileInputStream(fd);
	CodedInputStream*    coded_input = new CodedInputStream(raw_input);
	
	*loaded_from_pb = true;
	
	// Read num_objs from PB file
	coded_input->ReadLittleEndian32(num_objs);
	if (DEBUG_PRINTS){
		printf("Number of Objs to Load: %d\n", *num_objs);
	}

	// Read objects from PB file
	for(unsigned int i=0; i<*num_objs; i++){
		pb_objs_vector->push_back(new Nemo_Signal());
		
		if (!pb_objs_vector->back()->read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", filename);
		 	exit(-4);
		}
	}

	delete coded_input;
	delete raw_input;
	close(fd);	
}

void Nemo_Design::load_signals_from_pb(){
	// Read signal objects from PB file
	printf("Loading signal protobufs from %s ...\n", SIGS_PB_FILE);
	load_pb_objs(SIGS_PB_FILE, &signals_loaded_from_pb, nemo_sigs, &num_sigs)
	printf("Done.\n\n");
}

void Nemo_Design::load_connections_from_pb(){
	// Read connection objects from PB file
	printf("Loading connection protobufs from %s ...\n", CONS_PB_FILE);
	load_pb_objs(CONS_PB_FILE, &connections_loaded_from_pb, connections, &num_conns)
	printf("Done.\n\n");
}