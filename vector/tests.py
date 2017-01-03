import math, random
from unittest import TestCase
from vec import *

class TestVec(TestCase):
    def test_assignment(self):
        self.assertEqual("vec()", str(vec()), )
        self.assertEqual("vec(1)", str(vec(1)))
        self.assertEqual("vec(5, 7)", str(vec(5,7)))
        self.assertEqual("vec(1, 2, 3)", str(vec(1,2,3)))
        self.assertEqual("vec(1, 2, 3, 4)", str(vec(1,2,3,4)))
        self.assertEqual("vec(1, 2, 3)", str(vec((1,2,3))))
        self.assertEqual("vec(1, 2, 3)", str(vec((1,2),3)))
        self.assertEqual("vec(1, 2, 3)", str(vec(1,(2,3))))
        self.assertEqual("vec(1, 2, 3)", str(vec(1,(2,),3)))
        self.assertEqual("vec(1, 2, 3, 4, 5)", str(vec([1,[2,[3,[4,]],5]] )))
        with self.assertRaises(TypeError):
            vec3("blab")
        with self.assertRaises(TypeError):
            vec3({"x":23})

    def test_equal(self):
        self.assertTrue(  vec3(1) == vec3(1) )
        self.assertFalse( vec3(1) == vec3(2) )
        self.assertTrue(  vec3(1) == (1,1,1) )
        self.assertFalse( vec3(1) == (1, 1) )

    def test_attributes(self):
        self.assertEqual(1, vec(1,2,3,4).x)
        self.assertEqual(2, vec(1,2,3,4).y)
        self.assertEqual(3, vec(1,2,3,4).z)
        self.assertEqual(4, vec(1,2,3,4).w)
        self.assertEqual(1, vec(1,2,3,4).r)
        self.assertEqual(2, vec(1,2,3,4).g)
        self.assertEqual(3, vec(1,2,3,4).b)
        self.assertEqual(4, vec(1,2,3,4).a)
        self.assertEqual([1, 2], vec(1, 2, 3).xy)
        self.assertEqual([3, 2, 1], vec(1, 2, 3).zyx)
        self.assertEqual([3, 2, 1], vec(1, 2, 3).bgr)
        self.assertEqual([4, 3, 2, 1], vec(1, 2, 3, 4).abgr)
        self.assertEqual([4, 3, 2, 1], vec(1, 2, 3, 4).wzyx)
        self.assertEqual([1, 2, 3, 4], vec(1, 2, 3, 4).stpq)
        self.assertEqual([1,2,3,1,2,3,1,2,3], vec(1, 2, 3).xyzxyzxyz)
        with self.assertRaises(AttributeError):
            a = vec(1,2,3).k

    def test_attributes_set(self):
        a = vec3()
        a.x = 5
        self.assertEqual((5,0,0), a)
        a.y = 6
        self.assertEqual((5, 6, 0), a)
        a.z = 7
        self.assertEqual((5, 6, 7), a)
        a.xy = [8,9]
        self.assertEqual((8, 9, 7), a)
        a.zyx = [10,11,12]
        self.assertEqual((12,11,10), a)
        with self.assertRaises(AttributeError):
            a.w = 1.
        with self.assertRaises(AttributeError):
            a.xx = [1,2]
        with self.assertRaises(TypeError):
            a.xy = [1]
        with self.assertRaises(TypeError):
            a.xyz = [1,2,3,4]
        a = vec(0,0,0,0)
        a.rgba = [1,2,3,4]
        self.assertEqual((1,2,3,4), a)
        a.stpq = [5,6,7,8]
        self.assertEqual((5,6,7,8), a)

    def test_getitem(self):
        a = vec3(1,2,3)
        self.assertEqual(1, a[0])
        self.assertEqual(2, a[1])
        self.assertEqual(3, a[2])
        self.assertEqual(3, a[-1])
        self.assertEqual(2, a[-2])
        self.assertEqual(1, a[-3])
        with self.assertRaises(IndexError):
            a[3]
        with self.assertRaises(IndexError):
            a[-4]

    def test_setitem(self):
        a = vec(0,0,0)
        a[0] = 1
        self.assertEqual(vec(1,0,0), a)
        a[1] = 2
        self.assertEqual(vec(1,2,0), a)
        a[2] = 3
        self.assertEqual(vec(1,2,3), a)
        a[-1] = 4
        self.assertEqual(vec(1,2,4), a)
        a[-2] = 5
        self.assertEqual(vec(1,5,4), a)
        a[-3] = 6
        self.assertEqual(vec(6,5,4), a)
        with self.assertRaises(IndexError):
            a[3] = 1
        with self.assertRaises(IndexError):
            a[-4] = 1
        with self.assertRaises(TypeError):
            a[0] = "a"
        with self.assertRaises(TypeError):
            a[0] = {1}
        with self.assertRaises(TypeError):
            a[0] = {"x":1}

    def test_contains(self):
        self.assertEqual(True,  1 in vec(1,2,3))
        self.assertEqual(True,  2 in vec(1,2,3))
        self.assertEqual(True,  3 in vec(1,2,3))
        self.assertEqual(False, 4 in vec(1,2,3))
        self.assertEqual(False, 0 in vec())
        self.assertEqual(True,  [1,2] in vec(1,2,3))
        self.assertEqual(True,  [2,3] in vec(1,2,3))
        self.assertEqual(True,  [1,2,3] in vec(1,2,3))
        self.assertEqual(False, [1,3] in vec(1,2,3))
        with self.assertRaises(TypeError):
            "a" in vec(1,2,3)
        with self.assertRaises(TypeError):
            {"a":1} in vec(1,2,3)

    def test_len(self):
        self.assertEqual(0, len(vec()))
        self.assertEqual(1, len(vec(1)))
        self.assertEqual(4, len(vec(1,2,3,4)))

    def test_iter(self):
        self.assertEqual([1,2,3], [x for x in vec(1,2,3)])
        self.assertEqual([3,2,1], [x for x in reversed(vec(1, 2, 3))])

    def test_split(self):
        self.assertEqual([vec(1,2),vec(3,4)],   vec(1,2,3,4).split(2))
        self.assertEqual([vec3(1,2,3),vec(4)],  vec(1,2,3,4).split(3))
        self.assertEqual([vec(1,2,3,4)],        vec(1,2,3,4).split(4))


    def test_abs(self):
        self.assertEqual(vec3(1,2,3), abs(vec3(-1,-2,-3)))
        self.assertEqual(vec3(1,2,3), abs(vec3( 1,-2, 3)))

    def test_floor(self):
        self.assertEqual(vec3(1,2,3),       vec3(1.4,2.5,3.6).floor())
        self.assertEqual(vec3(-2,-3,-4),    vec3(-1.4,-2.5,-3.6).floor())
        self.assertEqual(vec3(1,2,3),       vec3(1.4,2.5,3.6).floored())
        self.assertEqual(vec3(-2,-3,-4),    vec3(-1.4,-2.5,-3.6).floored())

    def test_round(self):
        self.assertEqual((0, 0, 1),         vec3(0.49, 0.5, 0.51).round())
        self.assertEqual((0, 0, -1),        vec3(-0.49, -0.5, -0.51).round())
        self.assertEqual((0.5, 0.5, 0.5),   vec3(0.49, 0.5, 0.51).round(1))
        self.assertEqual((-0.5, -0.5, -0.5),vec3(-0.49, -0.5, -0.51).round(1))
        self.assertEqual((0.12346, 0.12346, 0.12341), vec3(0.123456, 0.123456789, 0.1234123456789).round(5))
        self.assertEqual((0, 0, 1),         vec3(0.49, 0.5, 0.51).rounded())
        self.assertEqual((0, 0, -1),        vec3(-0.49, -0.5, -0.51).rounded())

    def test_add(self):
        self.assertEqual(vec3(3),       vec3(1) + 2)
        self.assertEqual(vec3(3),       vec3(1) + vec3(2))
        self.assertEqual(vec3(3,5,7),   vec3(1,2,3) + vec3(2,3,4))
        self.assertEqual(vec3(3),       vec3(1) + vec3(2))
        self.assertEqual(vec3(2,3,4),   vec3(1) + [1,2,3])

        self.assertEqual(vec3(3),       2 + vec3(1))
        self.assertEqual(vec3(2,3,4),   [1,2,3] + vec3(1))

        with self.assertRaises(TypeError):
            vec3() + [1,2]
        with self.assertRaises(TypeError):
            vec3() + [1, 2, 3, 4]
        with self.assertRaises(TypeError):
            vec3() + "abc"
        with self.assertRaises(TypeError):
            vec3() + {1:2.}

    def test_add_inplace(self):
        a = vec3(1)
        a += 1
        self.assertEqual(vec3(2), a)
        a += [1,2,3]
        self.assertEqual(vec3(3,4,5), a)

    def test_sub(self):
        self.assertEqual(vec3(1), vec3(3) - vec3(2))
        self.assertEqual(vec3(1), vec3(3) - 2)
        self.assertEqual(vec3(2,1,0), vec3(3) - (1,2,3))
        self.assertEqual(vec3(1), 3 - vec3(2))
        self.assertEqual(vec3(-1,0,1), (1,2,3) - vec3(2))

    def test_sub_inplace(self):
        a = vec3(1)
        a -= 2
        self.assertEqual(vec3(-1), a)
        a -= [1,2,3]
        self.assertEqual(vec3((-2,-3,-4)), a)

    def test_mul(self):
        self.assertEqual(vec3(6), vec3(2) * vec3(3))
        self.assertEqual(vec3(6), vec3(2) * 3)
        self.assertEqual(vec3(2,4,6), vec3(2) * (1,2,3))
        self.assertEqual(vec3(2,4,6), vec3(2) * vec3(1,2,3))
        self.assertEqual(vec3(6), 2 * vec3(3))
        self.assertEqual(vec3(3,6,9), (1,2,3) * vec3(3))

        a = vec3(1)
        a *= 2
        self.assertEqual(vec3(2), a)
        a *= [1,2,3]
        self.assertEqual(vec3(2,4,6), a)

    def test_div(self):
        self.assertEqual(vec3(1.5), vec3(3) / vec3(2))
        self.assertEqual(vec3(1.5), vec3(3) / 2)
        self.assertEqual(vec3(2,1,.5), vec3(2) / (1,2,4))
        self.assertEqual(vec3(2,1,.5), vec3(2) / vec3((1,2,4)))
        self.assertEqual(vec3(1.5), 3 / vec3(2))
        self.assertEqual(vec3(.5,1,1.5), (1,2,3) / vec3(2))

        a = vec3(8)
        a /= 2
        self.assertEqual(vec3(4), a)
        a /= [1,2,4]
        self.assertEqual(vec3((4,2,1)), a)

    def test_mod(self):
        self.assertEqual(vec3(1,0,1), vec3(1,2,3) % 2)
        self.assertEqual(vec3(1,2,.5), vec3(1,2,3) % 2.5)

    def test_dot(self):
        self.assertEqual(32, vec3(1,2,3).dot((4,5,6)))
        self.assertEqual(32, vec3(1,2,3).dot(4,5,6))
        with self.assertRaises(TypeError):
            vec3().dot((1,2))

    def _test_cross(self):
        self.assertEqual((0,0,1), vec3(1,0,0).cross((0,1,0)))
        self.assertEqual((0,-1,0), vec3(1,0,0).cross((0,0,1)))
        self.assertEqual((1,0,0), vec3(0,1,0).cross((0,0,1)))
        self.assertEqual((0,0,1), vec3(1,0,0).crossed((0,1,0)))
        self.assertEqual((0,-1,0), vec3(1,0,0).crossed((0,0,1)))
        self.assertEqual((1,0,0), vec3(0,1,0).crossed((0,0,1)))

    # TODO
    def _test_rotate(self):
        self.assertEqual(vec3(1,-3,2), vec3(1,2,3).rotate_x(90).round())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotate_y(90).round())
        self.assertEqual(vec3(-2,1,3), vec3(1,2,3).rotate_z(90).round())
        self.assertEqual(vec3(3,2,-1), vec3(2,-3,-1).rotate_z(90).round())

        self.assertEqual(vec3(1,-3,2), vec3(1,2,3).rotate_axis((1,0,0), 90).round())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotate_axis((0,1,0), 90).round())
        self.assertEqual(vec3(-2,1,3), vec3(1,2,3).rotate_axis((0,0,1), 90).round())

        self.assertEqual(vec3(2,-3,-1), vec3(1,2,3).rotate_x(90).rotate_y(90).round())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotate_x(90).rotate_y(90).rotate_z(90).round())

    def _test_rotated(self):
        self.assertEqual(vec3(1,-3,2), vec3(1,2,3).rotated_x(90).rounded())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotated_y(90).rounded())
        self.assertEqual(vec3(-2,1,3), vec3(1,2,3).rotated_z(90).rounded())
        self.assertEqual(vec3(3,2,-1), vec3(2,-3,-1).rotated_z(90).rounded())

        self.assertEqual(vec3(1,-3,2), vec3(1,2,3).rotated_axis((1,0,0), 90).rounded())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotated_axis((0,1,0), 90).rounded())
        self.assertEqual(vec3(-2,1,3), vec3(1,2,3).rotated_axis((0,0,1), 90).rounded())

        self.assertEqual(vec3(2,-3,-1), vec3(1,2,3).rotated_x(90).rotated_y(90).rounded())
        self.assertEqual(vec3(3,2,-1), vec3(1,2,3).rotated_x(90).rotated_y(90).rotated_z(90).rounded())

        self.assertEqual(vec3(3,2,-1),
                vec3(1,2,3).rotated_axis((1,0,0), 90).rotated_axis((0,1,0), 90).rotated_axis((0,0,1), 90).rounded())


    def test_op_speed(self):
        for i in range(1000000):
            vec3(1) + vec3(2)





