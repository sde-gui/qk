if [ ! -z $1 ]; then
    if [ ! -e $1 ]; then
        echo $0: error, file $1 doesn\'t exist
        exit 1
    fi
fi

replace () {
    sed -e 's/#include <sys\/types.h>//' \
        -e 's/#include <sys\/stat.h>//' \
        -e 's/#include <string.h>//' \
        -e 's/#include <stdio.h>//' \
        -e 's/#include <unistd.h>//' \
        -e 's/#include "about.h"//' \
        -e 's/#include "callbacks.h"/#include "mooutils\/compat.h"/' \
        -e 's/#include "support.h"/#include "gui\/glade\/support.h"/' \
        -e 's/create_about_dialog (void)/_ggap_create_about_dialog (const char *comment, const char *copyright, const char *name);\nGtkWidget *_ggap_create_about_dialog (const char *comment, const char *copyright, const char *name)/' \
        -e 's/name_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/name_label = gtk_label_new_with_markup (_(name));/' \
        -e 's/comment_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/comment_label = gtk_label_new_with_markup (_(comment));/' \
        -e 's/copyright_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/copyright_label = gtk_label_new_with_markup (_(copyright));/'
}

echo '#include <gtk/gtk.h>'
echo '#if GTK_MINOR_VERSION <= 4'
cat $1 | replace
echo '#endif /* GTK_MINOR_VERSION <= 4 */'

