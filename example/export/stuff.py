

def global_function(a, b):
    """
    global_function(float, float) -> float
    Adds two numbers
    """
    return a + b


class Class:
    """A versatile class"""
    def __init__(self):
        self._x = 0.
        pass

    def method(self, a, b):
        """
        method(string, int) -> self
        A cool function"""
        return self

    @property
    def x(self):
        return self._x

    @x.setter
    def x(self, val):
        self._x = val


