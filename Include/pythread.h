#pragma once

#include <pthread.h>

#define PyCOND_T pthread_cond_t

#define PYTHREAD_INVALID_THREAD_ID ((unsigned long) -1)


#define CHECK_STATUS(name) if (status != 0) { perror(name); error = 1; }
#define CHECK_STATUS_PTHREAD(name) if (status != 0) { fprintf(stderr, "%s: %s\n", name, strerror(status)); error = 1; }

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

typedef struct {
  char locked; // 0=unlocked, 1=locked
  pthread_cond_t lock_released;
  pthread_mutex_t mut;
} pthread_lock;

// NOTE: cpy/Python/thread_pthread.h has 2 implementation for
// PyThread_allocate_lock depends on if USE_SEMAPHORES is defined.
// The version with USE_SEMAPHORES un-defined is picked.
#undef USE_SEMAPHORES

// NULL when pthread_condattr_setclock(CLOCK_MONOTONIC) is not supported.
static pthread_condattr_t *condattr_monotonic = NULL;

int
_PyThread_cond_init(PyCOND_T *cond) {
  return pthread_cond_init(cond, condattr_monotonic);
}

// defined in cpy/Python/thread_pthread.h
PyThread_type_lock PyThread_allocate_lock() {
	pthread_lock *lock;
	int status, error = 0;

  if (!initialized)
    PyThread_init_thread();

  lock = (pthread_lock *) PyMem_RawCalloc(1, sizeof(pthread_lock));
  if (lock) {
    lock->locked = 0;

    status = pthread_mutex_init(&lock->mut, NULL);
    CHECK_STATUS_PTHREAD("pthread_mutex_init");

    status = _PyThread_cond_init(&lock->lock_released);
    CHECK_STATUS_PTHREAD("pthread_cond_init");

    if (error) {
      fail(0);
    }
  }

  return (PyThread_type_lock) lock;
}

#define WAIT_LOCK 1
#define NOWAIT_LOCK 0

int PyThread_acquire_lock(PyThread_type_lock lock, int waitflag) {
	// printf("WARNING: PyThread_acquire_lock not implemented yet\n");
	// return PY_LOCK_ACQUIRED;
	return 1;
}

void PyThread_free_lock(PyThread_type_lock lock) {
  // TODO not implemented yet
}

void PyThread_release_lock(PyThread_type_lock lock) {
	// printf("WARNING: PyThread_release_lock not implemented yet\n");
}

unsigned long PyThread_get_thread_ident(void) {
  volatile pthread_t threadid;
  if (!initialized)
    PyThread_init_thread();
  threadid = pthread_self();
  return (unsigned long) threadid;
}
