def _sum(n):
	tot = 0
	for i in range(n + 1):
		tot += i
	return tot
n = 100
# print(f"sum between 1 and {n} is {_sum(n)}")
print(_sum(n))
