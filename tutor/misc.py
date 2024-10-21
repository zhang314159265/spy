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

def test_unpack_dict():
	a = {3: 4}
	b = {**a, 5:6}
	print(b)

test_unpack_dict()
