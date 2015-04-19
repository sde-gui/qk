SET(plugins_sources
    plugins/moofileselector-prefs.c
    plugins/moofileselector.c
    plugins/moofileselector.h
    plugins/mooplugin-builtin.h
    plugins/mooplugin-builtin.c
    plugins/moofilelist.c
    plugins/moofind.c
)

foreach(input_file
	plugins/glade/moofileselector-prefs.glade
	plugins/glade/moofileselector.glade
	plugins/glade/moofind.glade
	plugins/glade/moogrep.glade
)
    ADD_GXML(${input_file})
endforeach(input_file)

# zzz include plugins/ctags/Makefile.incl
include(${CMAKE_CURRENT_SOURCE_DIR}/plugins/usertools/usertools.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/plugins/support/support.cmake)
