tot = 0

# Not simpliy iterating thru odd numbers on purpose to exercise thru
# if-statement
for i in range(200):
	if i % 2 == 1:
		tot += i

print("tot is {}".format(tot))
