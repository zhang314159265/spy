#pragma once

#define HAVE_BUILTIN_ATOMIC 1

#ifdef HAVE_BUILTIN_ATOMIC

static inline Py_ssize_t _Py_atomic_size_get(Py_ssize_t *var)
{
	return __atomic_load_n(var, __ATOMIC_SEQ_CST);
}

static inline void _Py_atomic_size_set(Py_ssize_t *var, Py_ssize_t value)
{
	__atomic_store_n(var, value, __ATOMIC_SEQ_CST);
}

#else
#error don't HAVE_BUILTIN_ATOMIC
#endif
