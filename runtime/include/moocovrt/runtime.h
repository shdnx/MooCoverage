#ifndef MOOCOV_RUNTIME_H
#define MOOCOV_RUNTIME_H

// Exposes declarations to be used internally by MooCoverage.
// For the public interface, see interface.h.

#include "moocovrt/interface.h"

typedef char* moocov_fileid_t;
typedef unsigned int moocov_data_t;
typedef unsigned long long moocov_data_size_t;

typedef struct _moocov_file_t {
	moocov_fileid_t id;

	int linked;
	moocov_data_t* data;
	moocov_data_size_t dataLength;

	struct _moocov_file_t* next;
} moocov_file_t;

MOOCOV_EXTERN_C void _moocov_link(moocov_file_t* file);
MOOCOV_EXTERN_C void _moocov_signal(moocov_file_t* file, moocov_data_size_t index);

#define MOOCOV_FILE(ID) _moocov_file##ID
#define MOOCOV_FILEREF(ID) &MOOCOV_FILE(ID)

#define MOOCOV_DEFINE_FILE(ID, NUMSIGNALS) \
	static moocov_data_t _moocov_data##ID[NUMSIGNALS]; \
	static moocov_file_t MOOCOV_FILE(ID) = { \
		#ID, \
		0, \
		_moocov_data##ID, \
		NUMSIGNALS, \
		0 \
	};

// TODO: modify moocov-instrument (FileInstrumentation.cpp) to emit these macros instead of direct calls

#define MOCOOV_LINK(FILEID) \
	_moocov_link(MOOCOV_FILEREF(FILEID))

#define MOCOOV_SIGNAL(FILEID, ID) \
	_moocov_signal(MOOCOV_FILEREF(FILEID), ID)

#endif // MOOCOV_RUNTIME_H
