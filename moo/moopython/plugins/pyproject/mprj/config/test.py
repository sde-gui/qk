#
#  mprj/config/_test.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

import unittest
from mprj.config import *
from mprj.config._item import create_instance as item_create_instance
from mprj.config._xml import XMLItem, XMLGroup, XML


class TestItem(unittest.TestCase):
    def testdescription(self):
        type = Item()
        self.assert_(type is Item)

    def testlongdescription(self):
        type = Item(default=2, value=3)
        self.assert_(type is not Item)
        self.assert_(issubclass(type, Item))
        dct = type.__item_attributes__
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
        self.assertRaises(TypeError, item_create_instance, [Item, {'name' : 'ffff'}], 'id')

    def testjunk(self):
        self.assertRaises(TypeError, item_create_instance, Item(name='wefwef'))
        item_create_instance(Item(blah='wefwef'), 'fwef')
        self.assertRaises(TypeError, item_create_instance)
        self.assertRaises(TypeError, item_create_instance, 'wefef')
        self.assertRaises(TypeError, item_create_instance, 1)
        self.assertRaises(TypeError, item_create_instance, Item, 1)
        self.assertRaises(TypeError, item_create_instance, [Item], 1)
        self.assertRaises(TypeError, item_create_instance, [Item], 'dd', name='ededed')
        self.assertRaises(TypeError, item_create_instance, [Item, {'name' : 'fff'}], 'dd', name='ededed')


class TestSetting(unittest.TestCase):
    def testvalue2(self):
        s = item_create_instance(Setting(), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        s.reset()
        self.assert_(s.get_value() is None)
        s.copy()

    def testdatatype(self):
        s = item_create_instance(Setting(data_type=str), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        self.assertRaises(TypeError, s.set_value, 8)
        s.reset()
        self.assert_(s.get_value() is None)
        s.set_string('5')
        self.assert_(s.get_value() == '5')
        s.copy()

    def testdatatype2(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        self.assert_(s.get_default() is None)
        self.assert_(s.get_value() is None)
        s.set_value(8)
        self.assert_(s.get_value() == 8)
        self.assertRaises(TypeError, s.set_value, '8')
        s.reset()
        self.assert_(s.get_value() is None)
        s.set_string('5')
        self.assert_(s.get_value() == 5)
        self.assertRaises(Exception, s.set_string, 'ewfwef')
        self.assertRaises(TypeError, s.set_value, 'ewfwef')
        s.copy()

    def testnormal(self):
        s = item_create_instance(Setting(name='name', data_type=int), '1', value=2)
        self.assert_(s.get_default() is None)
        self.assert_(not s.is_default())
        s = item_create_instance(Setting(name='name', default=8, data_type=int), '2')
        self.assert_(s.get_default() == 8)
        self.assert_(s.is_default())
        s = item_create_instance(Setting(name='name', default=8, data_type=int), '3', value=2)
        self.assert_(s.get_default() == 8)
        self.assert_(s.get_value() == 2)
        self.assert_(not s.is_default())
        s.copy()

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
        s1.copy()
        s2.copy()

    def testsave(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        self.assert_(s.save() == [])
        s.set_value(5)
        self.assert_(s.save() == [XMLItem('id', '5')])
        s.reset()
        self.assert_(s.save() == [])
        s.copy()

    def testsave2(self):
        s = item_create_instance(Setting(data_type=str), 'id')
        self.assert_(s.save() == [])
        s.set_value('')
        self.assert_(s.save() == [XMLItem('id', '')])
        s.reset()
        self.assert_(s.save() == [])
        s.copy()

    def testsave3(self):
        s = item_create_instance(Setting(default='444', data_type=str), 'id')
        self.assert_(s.save() == [])
        s.set_value('fff')
        self.assert_(s.save() == [XMLItem('id', 'fff')])
        s.set_value('')
        self.assert_(s.save() == [XMLItem('id', '')])
        s.copy()
        s = item_create_instance(Setting(default='444', data_type=str, null_ok=True), 'id')
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
        s.copy()

    def testload2(self):
        s = item_create_instance(Setting(data_type=int), 'id')
        node = XMLItem('id', 'fff')
        self.assertRaises(ValueError, s.load, node)
        node = XMLItem('id', '5')
        s.load(node)
        self.assert_(s.get_value() == 5)
        s.copy()


class TestDict(unittest.TestCase):
    def assign(self, s, key, val):
        s[key] = val

    def testdict(self):
        s = item_create_instance(Dict(str), 'id')
        self.assert_(s == s.copy())
        s['blah'] = 'fwef'
        self.assert_(s['blah'] == 'fwef')
        s['foo'] = 'ddd'
        self.assert_(s['foo'] == 'ddd')
        s['foo'] = 'blah'
        self.assert_(s['foo'] == 'blah')
        self.assert_(s == s.copy())
        self.assertRaises(Exception, self.assign, s, 'fff', 3)

    def testdict2(self):
        S = Setting(data_type=int, default=5)
        s = item_create_instance(Dict(S), 'id')
        self.assert_(s == s.copy())
        s['blah'] = S.create_instance('dd')
        self.assert_(s['blah'] == 5)
        self.assertRaises(Exception, self.assign, s, 'fff', 5)
        self.assertRaises(Exception, self.assign, s, 'fff', '44')
        self.assertRaises(Exception, self.assign, s, 'fff', None)
        self.assert_(s == s.copy())

    def testnormal(self):
        from mprj.config._dict import DictBase
        self.assert_(issubclass(Dict(str), DictBase))


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
        self.assert_(g1.copy() == g1)
        self.assert_(g2.copy() == g2)

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
        self.assert_(g.copy() == g)


class TestConfig(unittest.TestCase):
    def testconfig(self):
        class C(Config):
            __items__ = {
                'variables' : Dict(str),
                'variables2' : Dict(str, xml_elm_name='item', xml_attr_name='id'),
                'project_dir' : Setting(data_type=str),
                'stuff' : Dict(str)
            }

        f = File("""<medit-project version="2.0" name="Foo" type="Simple">
                      <variables>
                        <foo>bar</foo>
                      </variables>
                      <variables2>
                        <item id="foo">bar</item>
                      </variables2>
                      <project_dir>.</project_dir>
                      <stuff>
                        <kff>ddd</kff>
                      </stuff>
                    </medit-project>""")
        c = C(f)
        self.assert_(len(c.items()) == 4)
        self.assert_(c.project_dir == '.')
        self.assert_(len(c.stuff) == 1)
        self.assert_(c.stuff['kff'] == 'ddd')
        self.assert_(c.variables['foo'] == 'bar')
        self.assert_(c.variables2['foo'] == 'bar')


if __name__ == '__main__':
    unittest.main()
