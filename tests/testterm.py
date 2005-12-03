import gtk
from pyconsole import Console

def create_terminal():
    import vttest

    win = gtk.Window()
    win.set_default_size(700, 500)
    swin = gtk.ScrolledWindow()
    win.add(swin)
    swin.set_policy(gtk.POLICY_NEVER, gtk.POLICY_ALWAYS)

    term = vttest.VtTest()
    term.set_property("font-name", "Courier New 11")

    swin.add(term)

    win.show_all()
    win.set_resize_mode(gtk.RESIZE_IMMEDIATE)
    swin.set_resize_mode(gtk.RESIZE_IMMEDIATE)

    def set_width(term, width, window):
        height = term.char_height() * 25
        width *= term.char_width()
        term.set_size_request(width, height)
        window.resize(10, 10)
        window.check_resize()
        window.window.process_updates(True)

    term.connect("set-width", set_width, win)
    term.connect("set-window-title", lambda term,title,win: win.set_title(title), win)
    term.connect("bell", lambda *whatever: gtk.gdk.beep())

    return term


if __name__ == '__main__':
    window = gtk.Window()
    swin = gtk.ScrolledWindow()
    swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
    window.add(swin)
    swin.add(Console(use_rlcompleter=False,
                     locals={'create_terminal' : create_terminal},
                     start_script="import gtk\n" + \
                                  "import moo\n" + \
                                  "import vttest\n" + \
                                  "term = create_terminal()\n"))
    window.set_default_size(400, 300)
    window.show_all()

    if not gtk.main_level():
        window.connect("destroy", gtk.main_quit)
        gtk.main()
