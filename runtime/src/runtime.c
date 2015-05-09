#include <string.h> // memset, strlen
#include <stdio.h> // fopen, fwrite, fclose

#include "moocovrt/runtime.h"
#include "moocovrt/likely.h"
#include "moocovrt/fastint.h"

// Whether moocov_enable() and moocov_disable() does anything.
// Allowing the data gathering to be disabled has a performance penalty.
#ifndef ALLOW_DISABLE
#	define ALLOW_DISABLE 0
#endif

// If ALLOW_DISABLE is set to 1, whether to have data collection enabled initially or not.
#ifndef INITIAL_ENABLED
#	define INITIAL_ENABLED 1
#endif

// The name of the output file.
#ifndef DUMPFILE_NAME
#	define DUMPFILE_NAME "coverage.mocd"
#endif

#if ALLOW_DISABLE

// A value indicating whether moocov is currently gathering data.
static int g_moocov_enabled = (INITIAL_ENABLED);

#endif

// Defines a singly linked list with a head item of the moocov_file_t structures containing data.
typedef struct {
	moocov_file_t* head;
	moocov_file_t* tail;
} moocov_index_t;

// The head item of the linked list.
static moocov_file_t g_head;

// The linked list.
static moocov_index_t g_index = { &g_head, &g_head };

void moocov_enable() {
#if ALLOW_DISABLE
	g_moocov_enabled = 1;
#endif
}

void moocov_disable() {
#if ALLOW_DISABLE
	g_moocov_enabled = 0;
#endif
}

// Resets a moocov_file_t object and returns the next item in the chain (if any).
static moocov_file_t* _reset_file(moocov_file_t* file) {
	file->linked = 0;
	memset(file->data, 0, file->dataLength);

	moocov_file_t* next = file->next;
	file->next = 0;
	return next;
}

// Resets the linked list's pointers.
static void _reset_links() {
	g_index.head->next = 0;
	g_index.tail = g_index.head;
}

// Clears all accummulated data.
void moocov_reset() {
	moocov_file_t* file;
	for(file = g_index.head->next; file; file = file->next) {
		_reset_file(file);
	}

	_reset_links();
}

// Dumps out accummulated data to disk and then clears all data (same as moocov_reset()).
// The data files are text files, written to DUMPFILE_NAME. Subsequent dumps append to the same way.
void moocov_dump() {
	FILE* fp = fopen((DUMPFILE_NAME), "a");
	if(!fp) return;

	// a buffer large enough to hold any 64-bit integer in a hexadecimal format
	char buffer[4 * 16 * sizeof(unsigned long long)];

	moocov_file_t* file;
	for(file = g_index.head->next; file; file = _reset_file(file)) {
		fwrite(file->id, sizeof(char), strlen(file->id), fp);
		fwrite("\n", sizeof(char), 1, fp);

		moocov_data_size_t i;
		for(i = 0; i < file->dataLength; ++i) {
			if(file->data[i] != 0) {
				fwrite(buffer, sizeof(char), render_uint64(i, buffer), fp);
				fwrite(" ", sizeof(char), 1, fp);
				fwrite(buffer, sizeof(char), render_uint32(file->data[i], buffer), fp);
				fwrite("\n", sizeof(char), 1, fp);
			}
		}

		fwrite(";\n", sizeof(char), 2, fp);
	}

	fclose(fp);
	_reset_links();
}

// Adds an object to the linked list.
void _moocov_link(moocov_file_t* file) {
	if(MOOCOV_UNLIKELY(file->linked)) return;

	g_index.tail->next = file;
	g_index.tail = file;

	file->linked = 1;
}

// Registers a hit on the specified signal. VERY performance-critical.
void _moocov_signal(moocov_file_t* file, moocov_data_size_t index) {
#if ALLOW_DISABLE
	file->data[index] += g_moocov_enabled;
#else
	file->data[index]++;
#endif
}
