# def test_binary_pow():
# 	x = 2 ** 3
# 	print(x)
# 
# test_binary_pow()

# def test_mul():
# 	x = 2 * 3
# 	print(x)
# 
# test_mul()

# def test_floor_div():
# 	x = 3 // 2
# 	print(x)
# 
# test_floor_div()

# def test_build_tuple():
# 	# y = 5
# 	# x = (y, )
# 	# print(type(x))
# 	# print(x)
# 	t = ()
# 	print(t)
# 
# test_build_tuple()

# def test_list_ops():
# 	a = [2]
# 	x = (*a, 3)
# 	print(x)
# 
# test_list_ops()

# def test_dict_ops():
# 	x = {}
# 	x[5] = 6
# 	print(x[5])
# 
# test_dict_ops()

# def test_build_set():
# 	x = {3}
# 	print(x)
# 
# test_build_set()

# def test_bitwise_ops():
# 	x = 3
# 	y = 2
# 	print(x & y)
# 	print(x | y)
# 	print(x ^ y)
# 
# test_bitwise_ops()

# def test_shift():
# 	x = 2
# 	print(x << 1)
# 	print(x >> 1)
# 
# test_shift()

# def test_unary_ops():
# 	x = 2
# 	print(-x)
# 	print(+x)
# 	print(~x)
# 	print(not x)
# 
# test_unary_ops()

# def test_more_cmp():
# 	a = 3
# 	b = [3]
# 	print(a is b)
# 	print(a is not b)
# 	print(a in b)
# 	print(a not in b)
# 
# test_more_cmp()

# def test_inplace_ops():
# 	a = 3
# 	a -= 2
# 	a &= 3
# 	a //= 1
# 	a <<= 2
# 	a %= 200
# 	a *= 3
# 	a |= 3
# 	a **= 2
# 	a >>= 1
# 	a ^= 4
# 	# a /= 2
# 	print(a)
# 
# test_inplace_ops()

# def test_slice():
# 	a = [2, 3, 5, 7]
# 	print(a[1:3])
# 
# test_slice()

# def test_unpack():
# 	l = [3, 5]
# 	a, b = l
# 	print(a, b)
# 
# test_unpack()

# def test_del():
# 	a = 3
# 	del a
# 
# test_del()

# def test_dup():
# 	x = y = 5
# 	print(x, y)
# 
# test_dup()

# def test_subscript_augassign():
# 	x = [2, 3]
# 	x[0] += 1
# 	print(x)
# 
# test_subscript_augassign()

# def test_unpack_set():
# 	a = {3}
# 	b = {*a, 5}
# 	print(b)
# 
# test_unpack_set()

# def test_unpack_dict():
# 	a = {3: 4}
# 	b = {**a, 5:6}
# 	print(b)
# 
# test_unpack_dict()

# def test_build_const_key_map():
#     a = {1: 2, 3: 4}
#     print(a)
# 
# test_build_const_key_map()

# def test_func_default_kwdefaults():
#     def f(a=3, *, b=5):
#         return a + b
# 
#     print(f())
# 
# test_func_default_kwdefaults()

# def test_iter():
#     print("Tuple:")
#     l = (3, 5)
#     for v in l:
#         print(v)
# 
#     print("List:")
#     l = (3, 5)
#     for v in l:
#         print(v)
# 
#     print("Set:")
#     s = {3, 5}
#     for v in s:
#         print(v)
# 
#     print("Dict:")
#     d = {3: 1, 5: 2}
#     for v in d:
#         print(v)
# 
# test_iter()

# def test_str_add():
#     a = "hello"
#     b = " world"
#     print(a + b)
#  
# test_str_add()

# def test_ctx_mgr():
#     class Ctx:
#         def __enter__(self):
#             print("Enter")
# 
#         def __exit__(self, a, b, c):
#             print("Exit")
# 
#     with Ctx():
#         print("Hi")
# 
# test_ctx_mgr()

# def test_type():
#     class Cls:
#         pass
#     obj = Cls()
#     print(type(obj))
# 
# test_type()

# def test_dict_items():
#     d = {'a': 3, 'b': 5}
#     for kv in d.items():
#         print(kv[0], kv[1])
# 
# test_dict_items()

# def test_isinstance():
#     x = 3
#     print(isinstance(x, int))
#     print(isinstance(x, dict))
# 
# test_isinstance()

# def test_exc():
#     x = 3
#     try:
#         print(x.no_such_attr)
#         print("Attr found")
#     except:
#         print("Attr not found")
#     finally:
#         print("Finally got called")
# 
#     try:
#         print(x.__class__)
#         print("Attr found")
#     except:
#         print("Attr not found")
#     finally:
#         print("Finally got called")
# 
# test_exc()

# def test_call_function_kw():
#     def f(a, b):
#         return a + b
# 
#     # print(f(9, 16))
#     print(f(9, b=16))
# 
# test_call_function_kw()


# def test_getattr():
#     x = 3
#     print(getattr(x, 'foo', 'bar'))
# 
# test_getattr()

# def test_rpartition():
#     print("hello-world".rpartition("-"))
# 
# test_rpartition()

# def test_in_dict():
#     d = {"x": 5}
#     print("x" in d)
#     print("y" in d)
# 
# test_in_dict()


# def test_raise():
#     try:
#         raise RuntimeError("a runtime error")
#     except RuntimeError as e:
#         print("Caught", str(e))
# 
# test_raise()

# def test_var_call():
#     def make_call(f, *args, **kwargs):
#         return f(*args, **kwargs)
# 
#     def inc(v):
#         return v + 1
# 
#     print(make_call(inc, 23))
# 
# test_var_call()

# def test_del_subscr():
#     l = [3, 5]
#     del l[1]
#     print(l)
# 
#     d = {"a": 0, "b": 1}
#     del d["b"]
#     print(d)
# 
# test_del_subscr()

# def test_startswith():
#     s = "/a/b/c"
#     sep = "/"
#     ret = s.startswith(sep)
#     print(ret)
# 
# test_startswith()

# def test_float_cmp():
#     a = 3.14
#     b = 3.15
#     print(a == a)
#     print(a == b)

# test_float_cmp()

# def test_to_set():
#     a = ["a", "a", "b"]
#     s = set(a)
#     print(s)
# 
# test_to_set()

# def test_ternary_conditional_expr():
#     a = True
#     v = 3 if a else 5
#     w = 3 if not a else 5
#     print(v, w)
# 
# test_ternary_conditional_expr()

# def test_assert():
#     assert False, "hi"
# 
# test_assert()

def test_getattr():
    class Cls:
        def __getattr__(self, name):
            if name == "foo":
                return "bar"
            raise RuntimeError("no such attribute") # TODO attribute error

    o = Cls()
    print(o.foo)

test_getattr()
