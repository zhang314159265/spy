class Base:
    @staticmethod
    def greeting():
        print("Hello from Base class")

class Cls(Base):
    pass

Cls.greeting()
