def deco(Cls):
    def fn(self, arg):
        self.name = arg
    
    Cls.__init__ = fn
    return Cls

@deco
class Cls:
    pass

obj = Cls("cpy")
print("obj.name is", obj.name)
