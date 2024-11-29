class Cls:
    def __init__(self, name):
        self._name = name

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        self._name = name

o = Cls('init_name')
print("name is", o.name)
o.name = "new_name"
print("new name is", o.name)
