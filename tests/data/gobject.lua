require("mooedittest")
require("munit")

t = mooedittest.new_text_view()
tassert(t ~= nil, "mooedittest.new_text_view() != nil")

t:set_property('visible', false)
tassert(not t:get_property('visible'), "not t:get_property('visible') after t:set_property('visible', false)")
t:set_property('visible', true)
tassert(t:get_property('visible'), "t:get_property('visible') after t:set_property('visible', true)")

-- mooedittest.present(t)
tassert(true)
