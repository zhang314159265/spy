#pragma once

#include "frameobject.h"
#include "funcobject.h"
#include "tupleobject.h"
#include "sliceobject.h"

PyObject *PyObject_GetAttr(PyObject *v, PyObject *name);
#define GETLOCAL(i) (fastlocals[i])

#define SETLOCAL(i, value) do { PyObject *tmp = GETLOCAL(i); \
		GETLOCAL(i) = value; \
		Py_XDECREF(tmp); } while (0)

/* Stack manipulation macros */

#define STACK_LEVEL() ((int)(stack_pointer - f->f_valuestack))

#define BASIC_STACKADJ(n) (stack_pointer += n)
#define STACK_GROW(n) BASIC_STACKADJ(n)
#define STACK_SHRINK(n) BASIC_STACKADJ(-n)

#define JUMPTO(x) (next_instr = first_instr + (x))
#define JUMPBY(x) (next_instr += (x))

/* Code access macros */
#define INSTR_OFFSET() ((int)(next_instr - first_instr))

#define BASIC_PUSH(v) (*stack_pointer++ = (v))
#define PUSH(v) BASIC_PUSH(v)

#define BASIC_POP() (*--stack_pointer)
#define POP() BASIC_POP()
#define TOP() (stack_pointer[-1])
#define SECOND() (stack_pointer[-2])
#define THIRD() (stack_pointer[-3])
#define PEEK(n) (stack_pointer[-(n)])
#define SET_TOP(v) (stack_pointer[-1] = (v))
#define SET_SECOND(v) (stack_pointer[-2] = (v))
#define SET_THIRD(v) (stack_pointer[-3] = (v))

#define EXT_POP(STACK_POINTER) (*--(STACK_POINTER))

#define DISPATCH() goto predispatch;

#define EMPTY() (STACK_LEVEL() == 0)

#define NEXTOPARG() do { \
	_Py_CODEUNIT word = *next_instr; \
	opcode = _Py_OPCODE(word); \
	oparg = _Py_OPARG(word); \
	next_instr++; \
} while (0)

/* Tuple access macros */
#define GETITEM(v, i) PyTuple_GetItem((v), (i))

#define PREDICT_ID(op) PRED_##op
#define PREDICTED(op) PREDICT_ID(op):

// make it no-op for now
#define PREDICT(op)

#if USE_COMPUTED_GOTOS
#define TARGET(op) op: TARGET_##op
#else
#define TARGET(op) op
#endif

PyFrameObject *
_PyEval_MakeFrameVector(PyThreadState *tstate,
		PyFrameConstructor *con, PyObject *locals,
		PyObject *const *args, Py_ssize_t argcount,
		PyObject *kwnames) {
	PyCodeObject *co = (PyCodeObject*) con->fc_code;
	const Py_ssize_t total_args; // TODO follow cpy

	/* Create the frame */
	PyFrameObject *f = _PyFrame_New_NoTrack(tstate, con, locals);
	if (f == NULL) {
		return NULL;
	}
	PyObject **fastlocals = f->f_localsplus;
	PyObject **freevars = f->f_localsplus + co->co_nlocals;

	// Create a dictionary for keyword parameters (**kwargs)
	PyObject *kwdict = NULL;

	// Copy all positional arguments into local variables
	Py_ssize_t j, n;
	#if 0
	if (argcount > co->co_argcount) {
		n = co->co_argcount;
	} else 
	#endif
	{
		n = argcount;
	}
	for (j = 0; j < n; j++) {
		PyObject *x = args[j];
		Py_INCREF(x);
		SETLOCAL(j, x);
	}

	return f;
}

static inline PyObject *
_PyEval_EvalFrame(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	return tstate->interp->eval_frame(tstate, f, throwflag);
}

PyObject *
call_function(PyThreadState *tstate,
		PyObject ***pp_stack,
		Py_ssize_t oparg,
		PyObject *kwnames) {
	PyObject **pfunc = (*pp_stack) - oparg - 1;
	PyObject *func = *pfunc;
	PyObject *x, *w;
	Py_ssize_t nkwargs = (kwnames == NULL) ? 0 : PyTuple_GET_SIZE(kwnames);
	Py_ssize_t nargs = oparg - nkwargs;
	PyObject **stack = (*pp_stack) - oparg;

	x = PyObject_Vectorcall(func, stack, nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
	assert(x != NULL);

	while ((*pp_stack) > pfunc) {
		w = EXT_POP(*pp_stack);
		Py_DECREF(w);
	}
	return x;
}

PyObject *
_PyEval_EvalFrameDefault(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	PyObject **stack_pointer;
	const _Py_CODEUNIT *next_instr;
	int opcode;
	int oparg;
	PyObject **fastlocals;
	const _Py_CODEUNIT *first_instr;
	PyObject *retval = NULL;
	PyCodeObject *co;

	PyObject *names;
	PyObject *consts;

	/* push frame */
	tstate->frame = f;
	co = f->f_code;

	names = co->co_names;
	consts = co->co_consts;
	fastlocals = f->f_localsplus;

	first_instr = (_Py_CODEUNIT *) PyBytes_AS_STRING(co->co_code);
	assert(f->f_lasti >= -1);
	next_instr = first_instr + f->f_lasti + 1;
	stack_pointer = f->f_valuestack + f->f_stackdepth;
	f->f_stackdepth = -1;
	f->f_state = FRAME_EXECUTING;

	if (throwflag) {
		assert(false);
	}

main_loop:
	for (;;) {
		assert(stack_pointer >= f->f_valuestack); // else underflow
		assert(STACK_LEVEL() <= co->co_stacksize); // else overflow
		// assert(!_PyErr_Occurred(tstate));

		tracing_dispatch:
		{
			int instr_prev = f->f_lasti;
			f->f_lasti = INSTR_OFFSET();
			NEXTOPARG();
		}

		goto dispatch_opcode;

		predispatch:
			f->f_lasti = INSTR_OFFSET();
			NEXTOPARG();

		dispatch_opcode:

		switch (opcode) {
		case TARGET(LOAD_METHOD): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *obj = TOP();
			PyObject *meth = NULL;

			int meth_found = _PyObject_GetMethod(obj, name, &meth);

			if (meth == NULL) {
				assert(false);
			}

			if (meth_found) {
				SET_TOP(meth);
				PUSH(obj); // self
			} else {
				assert(false);
			}
			DISPATCH();
		}
		case TARGET(SET_ADD): {
			PyObject *v = POP();
			PyObject *set = PEEK(oparg);
			int err;
			err = PySet_Add(set, v);
			Py_DECREF(v);
			if (err != 0)
				goto error;
			DISPATCH();
		}
		case TARGET(SET_UPDATE): {
			PyObject *iterable = POP();
			PyObject *set = PEEK(oparg);
			int err = _PySet_Update(set, iterable);
			Py_DECREF(iterable);
			if (err < 0) {
				goto error;
			}
			DISPATCH();
		}

		case TARGET(ROT_THREE): {
			PyObject *top = TOP();
			PyObject *second = SECOND();
			PyObject *third = THIRD();
			SET_TOP(second);
			SET_SECOND(third);
			SET_THIRD(top);
			DISPATCH();
		}

		case TARGET(DUP_TOP_TWO): {
			PyObject *top = TOP();
			PyObject *second = SECOND();
			Py_INCREF(top);
			Py_INCREF(second);
			STACK_GROW(2);
			SET_TOP(top);
			SET_SECOND(second);
			DISPATCH();
		}

		case TARGET(DUP_TOP): {
			PyObject *top = TOP();
			Py_INCREF(top);
			PUSH(top);
			DISPATCH();
		}

		case TARGET(UNPACK_SEQUENCE): {
			PyObject *seq = POP(), *item, **items;
			if (PyTuple_CheckExact(seq) && PyTuple_GET_SIZE(seq) == oparg) {
				assert(false);
			} else if (PyList_CheckExact(seq) &&
					PyList_GET_SIZE(seq) == oparg) {
				items = ((PyListObject *) seq)->ob_item;
				while (oparg--) {
					item = items[oparg];
					Py_INCREF(item);
					PUSH(item);
				}
			} else {
				assert(false);
			}
			Py_DECREF(seq);
			DISPATCH();
		}

		case TARGET(BUILD_SLICE): {
			PyObject *start, *stop, *step, *slice;
			if (oparg == 3)
				step = POP();
			else
				step = NULL;
			stop = POP();
			start = TOP();
			slice = PySlice_New(start, stop, step);
			Py_DECREF(start);
			Py_DECREF(stop);
			Py_XDECREF(step);
			SET_TOP(slice);
			if (slice == NULL)
				goto error;
			DISPATCH();
		}

#define HANDLE_BINARY_OP(meth) \
		  PyObject *right = POP(); \
			PyObject *left = TOP(); \
			PyObject *res = meth(left, right); \
			Py_DECREF(left); \
			Py_DECREF(right); \
			SET_TOP(res); \
			if (res == NULL) \
				goto error; \
			DISPATCH()

		case TARGET(INPLACE_SUBTRACT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceSubtract);
		}
		case TARGET(INPLACE_FLOOR_DIVIDE): {
			HANDLE_BINARY_OP(PyNumber_InPlaceFloorDivide);
		}
		case TARGET(INPLACE_TRUE_DIVIDE): {
			HANDLE_BINARY_OP(PyNumber_InPlaceTrueDivide);
		}
		case TARGET(INPLACE_MULTIPLY): {
			HANDLE_BINARY_OP(PyNumber_InPlaceMultiply);
		}
		case TARGET(INPLACE_MODULO): {
			HANDLE_BINARY_OP(PyNumber_InPlaceRemainder);
		}
		case TARGET(INPLACE_POWER): {
		  PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_InPlacePower(left, right, Py_None);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH()
		}
		case TARGET(INPLACE_LSHIFT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceLshift);
		}
		case TARGET(INPLACE_RSHIFT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceRshift);
		}
		case TARGET(INPLACE_AND): {
			HANDLE_BINARY_OP(PyNumber_InPlaceAnd);
		}
		case TARGET(INPLACE_XOR): {
			HANDLE_BINARY_OP(PyNumber_InPlaceXor);
		}
		case TARGET(INPLACE_OR): {
			HANDLE_BINARY_OP(PyNumber_InPlaceOr);
		}
		case TARGET(IS_OP): {
			PyObject *right = POP();
			PyObject *left = TOP();
			int res = Py_Is(left, right) ^ oparg;
			PyObject *b = res ? Py_True : Py_False;
			Py_INCREF(b);
			SET_TOP(b);
			Py_DECREF(left);
			Py_DECREF(right);
			DISPATCH();
		}
		case TARGET(CONTAINS_OP): {
			PyObject *right = POP();
			PyObject *left = POP();
			int res = PySequence_Contains(right, left);
			Py_DECREF(left);
			Py_DECREF(right);
			if (res < 0) {
				goto error;
			}
			PyObject *b = (res ^ oparg) ? Py_True : Py_False;
			Py_INCREF(b);
			PUSH(b);
			DISPATCH();
		}

		case TARGET(UNARY_NOT): {
			PyObject *value = TOP();
			int err = PyObject_IsTrue(value);
			Py_DECREF(value);
			if (err == 0) {
				Py_INCREF(Py_True);
				SET_TOP(Py_True);
				DISPATCH();
			} else if (err > 0) {
				Py_INCREF(Py_False);
				SET_TOP(Py_False);
				DISPATCH();
			}
			STACK_SHRINK(1);
			goto error;
		}
		case TARGET(UNARY_POSITIVE): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Positive(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(UNARY_NEGATIVE): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Negative(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(UNARY_INVERT): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Invert(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_LSHIFT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Lshift(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_RSHIFT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Rshift(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_XOR): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Xor(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_OR): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Or(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_AND): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_And(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BUILD_SET): {
			PyObject *set = PySet_New(NULL);
			int err = 0;
			int i;

			if (set == NULL)
				goto error;
			for (i = oparg; i > 0; i--) {
				PyObject *item = PEEK(i);
				if (err == 0)
					err = PySet_Add(set, item);
				Py_DECREF(item);
			}
			STACK_SHRINK(oparg);
			if (err != 0) {
				Py_DECREF(set);
				goto error;
			}
			PUSH(set);
			DISPATCH();
		}
		case TARGET(BINARY_SUBSCR): {
			PyObject *sub = POP();
			PyObject *container = TOP();
			PyObject *res = PyObject_GetItem(container, sub);
			Py_DECREF(container);
			Py_DECREF(sub);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(STORE_SUBSCR): {
			PyObject *sub = TOP();
			PyObject *container = SECOND();
			PyObject *v = THIRD();
			int err;
			STACK_SHRINK(3);
			// container[sub] = v
			err = PyObject_SetItem(container, sub, v);
			Py_DECREF(v);
			Py_DECREF(container);
			Py_DECREF(sub);
			if (err != 0)
				goto error;
			DISPATCH();
		}
		case TARGET(BUILD_MAP): {
			Py_ssize_t i;
			PyObject *map = _PyDict_NewPresized((Py_ssize_t) oparg);
			if (map == NULL)
				goto error;
			for (i = oparg; i > 0; i--) {
				assert(false);
			}
			while (oparg--) {
				Py_DECREF(POP());
				Py_DECREF(POP());
			}
			PUSH(map);
			DISPATCH();
		}
		case TARGET(LIST_TO_TUPLE): {
			PyObject *list = POP();
			PyObject *tuple = PyList_AsTuple(list);
			Py_DECREF(list);
			if (tuple == NULL) {
				goto error;
			}
			PUSH(tuple);
			DISPATCH();
		}
		case TARGET(LIST_EXTEND): {
			PyObject *iterable = POP();
			PyObject *list = PEEK(oparg);
			PyObject *none_val = _PyList_Extend((PyListObject *) list, iterable);
			if (none_val == NULL) {
				assert(false);
			}
			Py_DECREF(none_val);
			Py_DECREF(iterable);
			DISPATCH();
		}
		case TARGET(LIST_APPEND): {
			PyObject *v = POP();
			PyObject *list = PEEK(oparg);
			int err;
			err = PyList_Append(list, v);
			Py_DECREF(v);
			if (err != 0)
				assert(false);
			DISPATCH();
		}
		case TARGET(BUILD_LIST): {
			PyObject *list = PyList_New(oparg);
			if (list == NULL)
				assert(false);
			while (--oparg >= 0) {
				PyObject *item = POP();
				PyList_SET_ITEM(list, oparg, item);
			}
			PUSH(list);
			DISPATCH();
		}
		case TARGET(BUILD_TUPLE): {
			PyObject *tup = PyTuple_New(oparg);
			if (tup == NULL)
				assert(false);
			while (--oparg >= 0) {
				PyObject *item = POP();
				PyTuple_SET_ITEM(tup, oparg, item);
			}
			PUSH(tup);
			DISPATCH();
		}
		case TARGET(BINARY_FLOOR_DIVIDE): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *quotient = PyNumber_FloorDivide(dividend, divisor);
			Py_DECREF(dividend);
			Py_DECREF(divisor);
			SET_TOP(quotient);
			if (quotient == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_MULTIPLY): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Multiply(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case JUMP_FORWARD: {
			JUMPBY(oparg);
			DISPATCH();
		}
		case TARGET(BINARY_POWER): {
			PyObject *exp = POP();
			PyObject *base = TOP();
			PyObject *res = PyNumber_Power(base, exp, Py_None);
			Py_DECREF(base);
			Py_DECREF(exp);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_SUBTRACT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *diff = PyNumber_Subtract(left, right);
			Py_DECREF(right);
			Py_DECREF(left);
			SET_TOP(diff);
			if (diff == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_TRUE_DIVIDE): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *quotient = PyNumber_TrueDivide(dividend, divisor);
			Py_DECREF(dividend);
			Py_DECREF(divisor);
			SET_TOP(quotient);
			if (quotient == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(POP_JUMP_IF_FALSE): {
			PyObject *cond = POP();
			if (Py_IsTrue(cond)) {
				Py_DECREF(cond);
				DISPATCH();
			}
			if (Py_IsFalse(cond)) {
				Py_DECREF(cond);
				JUMPTO(oparg);
				DISPATCH();
			}
			assert(false);
		}
		case TARGET(POP_JUMP_IF_TRUE): {
			PyObject *cond = POP();
			if (Py_IsFalse(cond)) {
				Py_DECREF(cond);
				DISPATCH();
			}
			if (Py_IsTrue(cond)) {
				Py_DECREF(cond);
				JUMPTO(oparg);
				DISPATCH();
			}
			assert(false);
		}
		case TARGET(COMPARE_OP): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyObject_RichCompare(left, right, oparg);
			SET_TOP(res);
			Py_DECREF(left);
			Py_DECREF(right);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_MODULO): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *res;
			if (PyUnicode_CheckExact(dividend) && (
				!PyUnicode_Check(divisor) || PyUnicode_CheckExact(divisor))) {
				assert(false);
			} else {
				res = PyNumber_Remainder(dividend, divisor);
			}
			Py_DECREF(divisor);
			Py_DECREF(dividend);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(CALL_METHOD): {
			PyObject **sp, *res, *meth;

			sp = stack_pointer;

			meth = PEEK(oparg + 2);
			if (meth == NULL) {
				assert(false);
			} else {
				res = call_function(tstate, &sp, oparg + 1, NULL);
				stack_pointer = sp;
			}

			PUSH(res);
			if (res == NULL) {
				assert(false);
			}
			DISPATCH();
		}
		case TARGET(LOAD_ATTR): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *owner = TOP();

			PyTypeObject *type = Py_TYPE(owner);
			PyObject *res;

			// Slow path
			res = PyObject_GetAttr(owner, name);
			Py_DECREF(owner);
			SET_TOP(res);
			if (res == NULL) {
				assert(false);
			}
			DISPATCH();
		}
		case TARGET(JUMP_ABSOLUTE): {
			// printf("JUMP_ABSOLUTE oparg is %d\n", oparg);
			JUMPTO(oparg);
			DISPATCH();
		}
		case TARGET(LOAD_FAST): {
			PyObject *value = GETLOCAL(oparg);
			if (value == NULL) {
				printf("LOAD_FAST miss, oparg %d\n", oparg);
				assert(false);
			}
			Py_INCREF(value);
			PUSH(value);
			DISPATCH();
		}
		case TARGET(FOR_ITER): {
			// printf("FOR_ITER oparg is %d\n", oparg);
			PREDICTED(FOR_ITER);
			/* before: [iter]; after: [iter, iter()] *or* [] */
			PyObject *iter = TOP();
			PyObject *next = (*Py_TYPE(iter)->tp_iternext)(iter);
			if (next != NULL) {
				PUSH(next);
				PREDICT(STORE_FAST);
				// PREDICT(UNPACK_SEQUENCE);
				DISPATCH();
			}
			if (_PyErr_Occurred(tstate)) {
				assert(false);
			}
			// iterator ended normally
			STACK_SHRINK(1);
			Py_DECREF(iter);
			JUMPBY(oparg);
			DISPATCH();
		}
		case TARGET(GET_ITER): {
			// before: [obj]; after [getiter(obj)]
			PyObject *iterable = TOP();
			PyObject *iter = PyObject_GetIter(iterable);
			Py_DECREF(iterable);
			SET_TOP(iter);
			if (iter == NULL)
				assert(false);
			PREDICT(FOR_ITER);
			PREDICT(CALL_FUNCTION);
			DISPATCH();
		}
		case TARGET(LOAD_NAME): {
			PyObject *name = GETITEM(names, oparg);
			// printf("LOAD_NAME: name is %s\n", PyUnicode_1BYTE_DATA(name));
			PyObject *locals = f->f_locals;
			PyObject *v;
			if (locals == NULL) {
				assert(false);
			}
			if (PyDict_CheckExact(locals)) {
				v = PyDict_GetItemWithError(locals, name);
				if (v != NULL) {
					Py_INCREF(v);
				} else if (_PyErr_Occurred(tstate)) {
					assert(false);
				}
			} else {
				assert(false);
			}
			if (v == NULL) {
				v = PyDict_GetItemWithError(f->f_globals, name);
				if (v != NULL) {
					Py_INCREF(v);
				} else if (_PyErr_Occurred(tstate)) {
					assert(false);
				} else {
					if (PyDict_CheckExact(f->f_builtins)) {
						v = PyDict_GetItemWithError(f->f_builtins, name);
						if (v == NULL) {
							assert(false);
						}
						Py_INCREF(v);
					} else {
						assert(false);
					}
				}
			}
			assert(v);
			PUSH(v);
			DISPATCH();
		}
		case TARGET(BINARY_ADD): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *sum;
			if (PyUnicode_CheckExact(left) && PyUnicode_CheckExact(right)) {
				assert(false);
			} else {
				sum = PyNumber_Add(left, right);
				Py_DECREF(left);
			}
			Py_DECREF(right);
			SET_TOP(sum);
			if (sum == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(INPLACE_ADD): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *sum;
			if (PyUnicode_CheckExact(left) && PyUnicode_CheckExact(right)) {
				assert(false);
			} else {
				sum = PyNumber_InPlaceAdd(left, right);
				Py_DECREF(left);
			}
			Py_DECREF(right);
			SET_TOP(sum);
			if (sum == NULL)
				assert(false);
			DISPATCH();
		}

		case TARGET(LOAD_GLOBAL): {
			PyObject *name;
			PyObject *v;
			if (PyDict_CheckExact(f->f_globals)
					&& PyDict_CheckExact(f->f_builtins)) {
				name = GETITEM(names, oparg);
				v = _PyDict_LoadGlobal((PyDictObject *) f->f_globals,
						(PyDictObject *) f->f_builtins,
						name);
				if (v == NULL) {
					printf("LOAD_GLOBAL miss symbol %s\n", (char *) PyUnicode_DATA(name));
					assert(false);
				}
				Py_INCREF(v);
			} else {
				assert(false);
			}
			PUSH(v);
			DISPATCH();
		}
		
		case TARGET(LOAD_CONST): {
			PREDICTED(LOAD_CONST);
			PyObject *value = GETITEM(consts, oparg);
			Py_INCREF(value);
			PUSH(value);
			DISPATCH();
		}

		case TARGET(CALL_FUNCTION): {
			PREDICTED(CALL_FUNCTION);
			PyObject **sp, *res;
			sp = stack_pointer;
			res = call_function(tstate, &sp, oparg, NULL);
			stack_pointer = sp;
			PUSH(res);
			if (res == NULL) {
				assert(false);
			}
			DISPATCH();
		}

		case TARGET(POP_TOP): {
			PyObject *value = POP();
			Py_DECREF(value);
			DISPATCH();
		}

		case TARGET(RETURN_VALUE): {
			retval = POP();
			// assert(f->f_iblock == 0);
			assert(EMPTY());
			f->f_state = FRAME_RETURNED;
			// goto exiting;
			goto exiting;
		}

		case TARGET(MAKE_FUNCTION): {
			PyObject *qualname = POP();
			PyObject *codeobj = POP();
			PyFunctionObject *func = (PyFunctionObject *)
					PyFunction_NewWithQualName(codeobj, f->f_globals, qualname);

			Py_DECREF(codeobj);
			Py_DECREF(qualname);
			if (func == NULL) {
				assert(false);
			}

			if (oparg & 0x08) {
				assert(false);
			}
			if (oparg & 0x04) {
				assert(false);
			}
			if (oparg & 0x02) {
				assert(false);
			}
			if (oparg & 0x01) {
				assert(false);
			}

			PUSH((PyObject *) func);
			DISPATCH();
		}
		case TARGET(STORE_FAST): {
			PREDICTED(STORE_FAST);
			PyObject *value = POP();
			SETLOCAL(oparg, value);
			DISPATCH();
		}
		case TARGET(STORE_NAME): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *v = POP();
			PyObject *ns = f->f_locals;
			int err;
			if (ns == NULL) {
				assert(false);
			}
			if (PyDict_CheckExact(ns)) {
				err = PyDict_SetItem(ns, name, v);
			} else {
				assert(false);
			}
			Py_DECREF(v);
			if (err != 0) {
				assert(false);
			}
			DISPATCH();
		}
		case TARGET(DELETE_FAST): {
			PyObject *v = GETLOCAL(oparg);
			if (v != NULL) {
				SETLOCAL(oparg, NULL);
				DISPATCH();
			}
			assert(false);
		}
		default:
			printf("Can not handle opcode %d\n", opcode);
			assert(false);
		} /* switch */

		/* This should never be reached. Every opcode should end with DISPATCH()
			or goto error. */
		Py_UNREACHABLE();
error:
		assert(false); 
	} // main loop

	assert(false);

exiting:
	/* pop frame */

exit_eval_frame:
	tstate->frame = f->f_back;

	return _Py_CheckFunctionResult(tstate, NULL, retval, __func__);
}

PyObject *
_PyEval_Vector(PyThreadState *tstate, PyFrameConstructor *con,
		PyObject *locals,
		PyObject *const* args, size_t argcount,
		PyObject *kwnames) {
	PyFrameObject *f = _PyEval_MakeFrameVector(
		tstate, con, locals, args, argcount, kwnames);
	if (f == NULL) {
		return NULL;
	}

	PyObject *retval = _PyEval_EvalFrame(tstate, f, 0);

	Py_DECREF(f);
	return retval;
}

// defined in cpy/Python/ceval.c
PyObject *
PyEval_EvalCode(PyObject *co, PyObject *globals, PyObject *locals) {
	PyThreadState *tstate = PyThreadState_GET();
	if (locals == NULL) {
		locals = globals;
	}

	PyObject *builtins = _PyEval_BuiltinsFromGlobals(tstate, globals); // borrowed ref
	PyDict_New(); // TODO follow cpy
	if (builtins == NULL) {
		return NULL;
	}

	PyFrameConstructor desc = {
		.fc_globals = globals,
		.fc_builtins = builtins,
		.fc_name = ((PyCodeObject *) co)->co_name,
		.fc_qualname = ((PyCodeObject *) co)->co_name,
		.fc_code = co,
		.fc_defaults = NULL,
		.fc_kwdefaults = NULL,
		.fc_closure = NULL,
	};
	return _PyEval_Vector(tstate, &desc, locals, NULL, 0, NULL);
}

PyObject *
_PyEval_GetBuiltins(PyThreadState *tstate) {
	PyFrameObject *frame = tstate->frame;
	if (frame != NULL) {
		return frame->f_builtins;
	}
	return tstate->interp->builtins;
}

PyObject *PyEval_GetGlobals(void) {
	PyThreadState *tstate = _PyThreadState_GET();
	PyFrameObject *current_frame = tstate->frame;
	if (current_frame == NULL) {
		return NULL;
	}
	assert(false);
}

// return 0 on error, 1 on success
int _PyEval_SliceIndex(PyObject *v, Py_ssize_t *pi) {
	PyThreadState *tstate = _PyThreadState_GET();
	if (!Py_IsNone(v)) {
		Py_ssize_t x;
		if (_PyIndex_Check(v)) {
			x = PyNumber_AsSsize_t(v, NULL);
			if (x == -1 && _PyErr_Occurred(tstate))
				return 0;
		} else {
			assert(false);
		}
		*pi = x;
	}
	return 1;
}
