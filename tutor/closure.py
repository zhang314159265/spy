def f(a):
    def g(b):
        return a + b
    return g

print(f(3)(5))
