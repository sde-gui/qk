# -*- makefile -*-

# $(pixmaps_files) should be a list of icons for pixmaps/ dir
# $(theme_files) should be a list of icons for hicolor/48x48/apps/ dir
# call MOO_AM_MIME_MK autoconf macro

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = $(pixmaps_files)

iconthemedir = $(datadir)/icons/hicolor/48x48/apps
icontheme_DATA = $(theme_files)

update_icon_cache = gtk-update-icon-cache -f -t $(DESTDIR)$(datadir)/icons/hicolor

if MOO_ENABLE_GENERATED_FILES
install_data_hook_dummy_var =
uninstall_hook_dummy_var =
install-data-hook:
	if echo "Updating icon cache" && $(update_icon_cache); then		\
		echo "Done.";							\
	else									\
		echo "*** GTK icon cache not updated. After install, run this:";\
		echo $(update_icon_cache);					\
	fi
uninstall-hook:
	if echo "Updating icon cache" && $(update_icon_cache); then echo "Done."; else echo "Failed."; fi
endif
