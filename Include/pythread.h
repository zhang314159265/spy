#pragma once

#include <pthread.h>


#define CHECK_STATUS(name) if (status != 0) { perror(name); error = 1; }

typedef void *PyThread_type_lock;

// defined in cpy/Python/thread.c
static int initialized;

static void
init_condattr(void)
{
	// nothing to be done.
}

static void
PyThread__init_thread(void) {
	init_condattr();
}

// defined in cpy/Python/thread.c
void
PyThread_init_thread(void) {
	if (initialized)
		return;
	initialized = 1;
	PyThread__init_thread();
}

// NOTE: cpy/Python/thread_pthread.h has 2 implementation for
// PyThread_allocate_lock depends on if USE_SEMAPHORES is defined.
// The version with USE_SEMAPHORES un-defined is picked.
#undef USE_SEMAPHORES

// defined in cpy/Python/thread_pthread.h
PyThread_type_lock PyThread_allocate_lock() {
	// pthread_lock *lock;
	int status, error = 0;

	// printf("WARNING: PyThread_allocate_lock is not implemented yet\n");
	return NULL;
}

#define WAIT_LOCK 1
#define NOWAIT_LOCK 0

int PyThread_acquire_lock(PyThread_type_lock lock, int waitflag) {
	// printf("WARNING: PyThread_acquire_lock not implemented yet\n");
	return 0;
}

void PyThread_release_lock(PyThread_type_lock lock) {
	// printf("WARNING: PyThread_release_lock not implemented yet\n");
}
