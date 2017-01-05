

def global_function_no_param():
    """
    global_function_no_param() -> float
    Adds two numbers
    """
    return a + b

def global_function_one(a):
    """
    global_function(float) -> float
    Returns inverse of input
    """
    return 1. / a

def global_function_two(a, b):
    """
    global_function_two(float, float) -> float
    Adds two numbers
    """
    return a + b

def global_function_defaults(a, b=1, c=2):
    """
    global_function_defaults(float, float, float) -> float
    Adds all numbers
    """
    return a + b + c

def global_function_kwargs(a, b, **kwargs):
    """
    global_function_defaults(float, float, **kwargs) -> float
    Adds two numbers
    """
    return a + b


class ClassA:
    """A versatile class"""
    def __init__(self):
        self._x = 0.
        pass

    def amethod(self, a, b):
        """
        method(string, int) -> self
        A cool member function"""
        return self

    @property
    def readable(self):
        return self._x

    @property
    def writeable(self):
        return self._x

    @writeable.setter
    def writeable(self, val):
        self._x = val

class ClassB(ClassA):
    """A derived class"""

    def bmethod(self, a, b):
        """Another fine method"""
        pass


class ClassC(ClassB):
    """A even more derived class"""

    def cmethod(self, a, b):
        """And yet another fine method"""
        pass

