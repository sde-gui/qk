import unittest
from mprj.config import *
from mprj.config._item import create_instance as item_create_instance
from mprj.config._xml import XMLItem, XMLGroup, XML


class TestItem(unittest.TestCase):
    def testdescription(self):
        type, dummy = Item()
        self.assert_(type is Item)
        self.assert_(not dummy)

    def testlongdescription(self):
        type, dct = Item(default=2, value=3)
        self.assert_(type is Item)
        self.assert_(len(dct) == 2)
        self.assert_(dct['default'] == 2)
        self.assert_(dct['value'] == 3)

    def testcreateinstance(self):
        i = Item.create_instance('id', name='blah', description='an item')
        self.assert_(i.get_id() == 'id')
        self.assert_(i.get_name() == 'blah')
        self.assert_(i.get_description() == 'an item')
        self.assert_(i.get_visible())

    def testcreateinstance2(self):
        i = Item.create_instance('id', visible=False)
        self.assert_(i.get_id() == 'id')
        self.assert_(i.get_name() == 'id')
        self.assert_(i.get_description() == 'id')
        self.assert_(not i.get_visible())

    def testjunk(self):
        # id must be specified
        self.assertRaises(Exception, Item.create_instance)
        # 'something' is invalid
        self.assertRaises(Exception, Item.create_instance, 'blah', something=8)


class TestCreateInstance(unittest.TestCase):
    def testnormal(self):
        item_create_instance(Item(name='wefwef'), 'id')
        item_create_instance(Item(name='wefwef', description='ewfwef'), 'id')
        item_create_instance(Item, 'id')
        item_create_instance([Item, {'name' : 'ffff'}], 'id')

    def testjunk(self):
        self.assertRaises(TypeError, item_create_instance, Item(name='wefwef'))
        self.assertRaises(TypeError, item_create_instance, Item(blah='wefwef'), 'fwef')
        self.assertRaises(TypeError, item_create_instance)
        self.assertRaises(TypeError, item_create_instance, 'wefef')
        self.assertRaises(TypeError, item_create_instance, 1)
        self.assertRaises(TypeError, item_create_instance, Item, 1)
        self.assertRaises(TypeError, item_create_instance, [Item], 1)
        self.assertRaises(TypeError, item_create_instance, [Item], 'dd', name='ededed')
        self.assertRaises(TypeError, item_create_instance, [Item, {'name' : 'fff'}], 'dd', name='ededed')


class TestSetting(unittest.TestCase):
    def testvalue(self):
        s = item_create_instance(Setting(name='name', default=5), 'id')
        self.assert_(s.get_default() == 5)
        self.assert_(s.get_value() == 5)
        s.set_value(8)
        self.assert_(s.get_value() == 8)
        s.reset()
        self.assert_(s.get_value() == 5)

    def testvalue2(self):
        s = item_create_instance(Setting(), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        s.set_value(8)
        self.assert_(s.get_value() == 8)
        s.reset()
        self.assert_(s.get_value() is None)

    def testdatatype(self):
        s = item_create_instance(Setting(data_type=str), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        self.assertRaises(ValueError, s.set_value, 8)
        s.reset()
        self.assert_(s.get_value() is None)
        s.set_string('5')
        self.assert_(s.get_value() == '5')

    def testdatatype2(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        s.set_value(8)
        self.assert_(s.get_value() == 8)
        self.assertRaises(ValueError, s.set_value, '8')
        s.reset()
        self.assert_(s.get_value() is None)
        s.set_string('5')
        self.assert_(s.get_value() == 5)
        self.assertRaises(ValueError, s.set_string, 'ewfwef')
        self.assertRaises(ValueError, s.set_value, 'ewfwef')

    def testnormal(self):
        s = item_create_instance(Setting(name='name', value=2), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(not s.is_default())
        s = item_create_instance(Setting(name='name', default=8), 'id')
        self.assert_(s.get_default() == 8)
        self.assert_(s.is_default())
        s = item_create_instance(Setting(name='name', value=2, default=8), 'id')
        self.assert_(s.get_default() == 8)
        self.assert_(s.get_value() == 2)
        self.assert_(not s.is_default())

    def testops(self):
        s1 = item_create_instance(Setting(data_type=int), 'id')
        s2 = item_create_instance(Setting(data_type=int), 'id')
        self.assert_(s1 == s2)
        s1.set_value(5)
        self.assert_(s1 != s2)
        s2.set_value(5)
        self.assert_(s1 == s2)
        s2 = s1.copy()
        self.assert_(s1 == s2)
        self.assert_(s1.equal(5))
        self.assert_(not s1.equal(s2))
        self.assert_(not s1.equal(3))

    def testsave(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        self.assert_(s.save() == [])
        s.set_value(5)
        self.assert_(s.save() == [XMLItem('id', '5')])
        s.reset()
        self.assert_(s.save() == [])

    def testsave2(self):
        s = item_create_instance(Setting(data_type=str), 'id')
        self.assert_(s.save() == [])
        s.set_value('')
        self.assert_(s.save() == [XMLItem('id', '')])
        s.reset()
        self.assert_(s.save() == [])

    def testsave3(self):
        s = item_create_instance(Setting(default='444', data_type=str), 'id')
        self.assert_(s.save() == [])
        s.set_value('fff')
        self.assert_(s.save() == [XMLItem('id', 'fff')])
        s.set_value(None)
        self.assert_(s.save() == [XMLItem('id', None)])

    def testload(self):
        s = item_create_instance(Setting(data_type=str), 'id')
        node = XMLItem('id', 'fff')
        s.load(node)
        self.assert_(s.get_value() == 'fff')
        node = XMLItem('id', None)
        s.load(node)
        self.assert_(s.get_value() is None)

    def testload2(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        node = XMLItem('id', 'fff')
        self.assertRaises(ValueError, s.load, node)
        node = XMLItem('id', '5')
        s.load(node)
        self.assert_(s.get_value() == 5)


class TestGroup(unittest.TestCase):
    def testgroup(self):
        class G(Group):
            __items__ = {
                'foo' : Setting(data_type=int, default=8),
                'bar' : Setting(data_type=int),
                'baz' : Setting(data_type=str),
            }
        g = G.create_instance('g')
        self.assert_(g == g.copy())
        self.assert_(g.foo == 8)
        self.assert_(g.bar is None)
        self.assert_(g.baz is None)
        g.foo = 2
        self.assert_(g.foo == 2)
        self.assert_(isinstance(g['foo'], Setting))
        g.baz = '444'
        self.assert_(g.baz == '444')
        saved = g.save()
        self.assert_(saved == [XML('<g><baz>444</baz><foo>2</foo></g>').root])
        g2 = G.create_instance('g')
        self.assert_(g2 != g)
        g2.load(saved[0])
        self.assert_(g2 == g)

    def testgroup2(self):
        class G1(Group):
            __items__ = {
                'foo' : Setting(data_type=int, default=3),
                'bar' : Setting(data_type=str, default='fff'),
                'baz' : Setting(data_type=str),
            }
        class G2(G1):
            __items__ = {
                'blah' : Setting(data_type=bool, default=True),
            }
        g1 = G1.create_instance('g')
        g2 = G2.create_instance('g')
        self.assert_(g1 != g2)
        self.assert_(g1.foo == g2.foo)
        self.assert_(g1.bar == g2.bar)
        self.assert_(g1.baz == g2.baz)
        self.assert_(g2.blah is True)

    def testgroup3(self):
        class G1(Group):
            __items__ = {
                'foo' : Setting(data_type=int, default=3),
                'bar' : Setting(data_type=str, default='fff'),
                'baz' : Setting(data_type=str),
            }
        class G2(G1):
            __items__ = {
                'blah' : Setting(data_type=bool, default=True),
            }
        class G3(Group):
            __items__ = {
                'foo' : G1,
                'bar' : G1,
                'baz' : G2,
            }
        g = G3.create_instance('g')
        self.assert_(g.foo == g.bar)


class TestList(unittest.TestCase):
    def testlist(self):
        typ = List(str)
        l = typ.create_instance('ddd')
        l.append('1')
        l.append('2')
        self.assert_(len(l) == 2)
        l2 = typ.create_instance('ddd')
        l2.append('1')
        l2.append('2')
        self.assert_(l == l2)

    def testlist2(self):
        typ = List(Setting())
        l = typ.create_instance('ddd')
        l.append(Setting.create_instance('a'))
        l.append(Setting.create_instance('b'))
        self.assert_(len(l) == 2)
        l2 = typ.create_instance('ddd')
        l2.append(Setting.create_instance('a'))
        l2.append(Setting.create_instance('b'))
        self.assert_(l == l2)
        self.assert_(l == l.copy())


class TestConfig(unittest.TestCase):
    def testconfig(self):
        class C(Config):
            __items__ = {
                'variables' : Dict(str),
                'project_dir' : Setting(data_type=str),
                'stuff' : List(str)
            }

        f = File("""<medit-project version="1.0" name="Foo" type="Simple">
                      <variables>
                        <foo>bar</foo>
                      </variables>
                      <project_dir>.</project_dir>
                      <stuff>
                        <kff>ddd</kff>
                      </stuff>
                    </medit-project>""")
        c = C(f)
        self.assert_(len(c.get_items()) == 3)
        self.assert_(c.project_dir == '.')
        self.assert_(len(c.stuff) == 1)
        self.assert_(c.stuff[0] == 'ddd')
        self.assert_(c.variables['foo'] == 'bar')


if __name__ == '__main__':
    unittest.main()
