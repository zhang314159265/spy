def trace(f):
    # def wrapper(*args, **kwargs):
    def wrapper(arg):
        print("Enter function", f.__name__)
        # f(*args, **kwargs)
        f(arg)
        print("Leave function", f.__name__)
    return wrapper

@trace
def greeting(name):
    print("Hey", name)

greeting("Jude")
