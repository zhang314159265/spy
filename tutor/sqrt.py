# Compute sqrt using Newton's method
# y = x*x
# given y (y >= 0), compute x
# f(x) = x * x - y
# f'(x) = 2 * x
# x0 -> f'(x0) = 2 * x0
# x1 = x0 - (x0 * x0 - y) / (2 * x0)
#    = x0 / 2 + y / (2 * x0)
#    = (x0 + y / x0) / 2

def sqrt(y):
	x = 1.0

	while True:
		x1 = (x + y / x) / 2.0
		if abs(x1 - x) < 1e-6:
			break
		x = x1
	return x

y = 2
print("sqrt of {} is {}".format(y, sqrt(y)))
