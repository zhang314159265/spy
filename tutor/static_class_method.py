class Cls:
    @staticmethod
    def static_method():
        print("This is a static method")

    @classmethod
    def class_method(cls):
        print("This is a class method")

Cls.static_method()
Cls.class_method()
