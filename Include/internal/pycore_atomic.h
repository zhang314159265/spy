#include "pyconfig.h"

#ifdef HAVE_STD_ATOMIC
#include <stdatomic.h>
#endif

#if defined(HAVE_STD_ATOMIC)

typedef enum _Py_memory_order {
  _Py_memory_order_relaxed = memory_order_relaxed,
} _Py_memory_order;

typedef struct _Py_atomic_address {
  atomic_uintptr_t _value;
} _Py_atomic_address;

#define _Py_atomic_load_explicit(ATOMIC_VAL, ORDER) \
  atomic_load_explicit(&((ATOMIC_VAL)->_value), ORDER)

#define _Py_atomic_store_explicit(ATOMIC_VAL, NEW_VAL, ORDER) \
  atomic_store_explicit(&((ATOMIC_VAL)->_value), NEW_VAL, ORDER)

#else
#error only work with HAVE_STD_ATOMIC defined for now
#endif

#define _Py_atomic_store_relaxed(ATOMIC_VAL, NEW_VAL) \
  _Py_atomic_store_explicit((ATOMIC_VAL), (NEW_VAL), _Py_memory_order_relaxed)

#define _Py_atomic_load_relaxed(ATOMIC_VAL) \
  _Py_atomic_load_explicit((ATOMIC_VAL), _Py_memory_order_relaxed)
