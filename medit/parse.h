#include <errno.h>

static gboolean
parse_filename (const char     *filename,
                MooAppFileInfo *file)
{
    const char *colon;
    char *freeme = NULL;
    char *norm_filename;
    char *uri;

    norm_filename = _moo_normalize_file_path (filename);
    filename = norm_filename;

    if (g_str_has_suffix (filename, "/") ||
#ifdef G_OS_WIN32
        g_str_has_suffix (filename, "\\") ||
#endif
        g_file_test (filename, G_FILE_TEST_IS_DIR))
    {
        g_free (norm_filename);
        return FALSE;
    }

    if ((colon = strrchr (filename, ':')) &&
        colon != filename &&
        strspn (colon + 1, "0123456789") == strlen (colon + 1) &&
        !g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        if (colon[1])
        {
            errno = 0;
            file->line = strtol (colon + 1, NULL, 10);
            if (errno)
                file->line = 0;
        }

        freeme = g_strndup (filename, colon - filename);
        filename = freeme;
    }

    if (!(uri = g_filename_to_uri (filename, NULL, NULL)))
    {
        g_critical ("could not convert filename to URI");
        g_free (freeme);
        g_free (norm_filename);
        return FALSE;
    }

    g_free (file->uri);
    file->uri = uri;

    g_free (freeme);
    g_free (norm_filename);
    return TRUE;
}

static void
parse_options_from_uri (const char     *optstring,
                        MooAppFileInfo *file)
{
    char **p, **comps;

    comps = g_strsplit (optstring, ";", 0);

    for (p = comps; p && *p; ++p)
    {
        if (!strncmp (*p, "line=", strlen ("line=")))
        {
            /* doesn't matter if there is an error */
            file->line = strtoul (*p + strlen ("line="), NULL, 10);
        }
        else if (!strncmp (*p, "options=", strlen ("options=")))
        {
            char **opts, **op;
            opts = g_strsplit (*p + strlen ("options="), ",", 0);
            for (op = opts; op && *op; ++op)
            {
                if (!strcmp (*op, "new-window"))
                    file->options |= MOO_EDIT_OPEN_NEW_WINDOW;
                else if (!strcmp (*op, "new-tab"))
                    file->options |= MOO_EDIT_OPEN_NEW_TAB;
            }
            g_strfreev (opts);
        }
    }

    g_strfreev (comps);
}

static gboolean
parse_uri (const char     *scheme,
           const char     *uri,
           MooAppFileInfo *file)
{
    const char *question_mark;
    const char *optstring = NULL;

    if (strcmp (scheme, "file") != 0)
    {
        file->uri = g_strdup (uri);
        return TRUE;
    }

    question_mark = strchr (uri, '?');

    if (question_mark && question_mark > uri)
    {
        file->uri = g_strndup (uri, question_mark - uri);
        optstring = question_mark + 1;
    }
    else
    {
        file->uri = g_strdup (uri);
    }

    if (optstring)
        parse_options_from_uri (optstring, file);

    return TRUE;
}

static char *
parse_uri_scheme (const char *string)
{
    const char *p;

    for (p = string; *p; ++p)
    {
        if (*p == ':')
        {
            if (p != string)
                return g_strndup (string, p - string);

            break;
        }

        if (!(p != string && g_ascii_isalnum (*p)) &&
            !(p == string && g_ascii_isalpha (*p)))
                break;
    }

    return NULL;
}

static gboolean
parse_file (const char      *string,
            MooAppFileInfo  *file,
            char           **current_dir)
{
    char *uri_scheme;
    char *filename;
    gboolean ret;

    if (g_path_is_absolute (string))
        return parse_filename (string, file);

    if ((uri_scheme = parse_uri_scheme (string)))
    {
        ret = parse_uri (uri_scheme, string, file);
        g_free (uri_scheme);
        return ret;
    }

    if (!*current_dir)
        *current_dir = g_get_current_dir ();

    filename = g_build_filename (*current_dir, string, NULL);
    ret = parse_filename (filename, file);

    g_free (filename);
    return ret;
}

static void
parse_files (MooAppFileInfo **files,
             int             *n_files)
{
    int i;
    int count;
    char *current_dir = NULL;

    *files = NULL;
    *n_files = 0;

    if (!medit_opts.files || !(*n_files = g_strv_length (medit_opts.files)))
        return;

    *files = g_new0 (MooAppFileInfo, *n_files);

    for (i = 0, count = 0; i < *n_files; ++i)
    {
        if (i == 0)
        {
            if (medit_opts.new_window)
                (*files)[i].options |= MOO_EDIT_OPEN_NEW_WINDOW;
            if (medit_opts.new_tab)
                (*files)[i].options |= MOO_EDIT_OPEN_NEW_TAB;
            if (medit_opts.reload)
                (*files)[i].options |= MOO_EDIT_OPEN_RELOAD;
            (*files)[i].line = medit_opts.line;
            if (medit_opts.encoding && medit_opts.encoding[0])
                (*files)[i].encoding = g_strdup (medit_opts.encoding);
        }

        if (parse_file (medit_opts.files[i], *files + count, &current_dir))
            count++;
    }

    *n_files = count;

    g_free (current_dir);
}

static void
free_files (MooAppFileInfo *files,
            int             n_files)
{
    int i;

    for (i = 0; i < n_files; ++i)
    {
        g_free (files[i].uri);
        g_free (files[i].encoding);
    }

    g_free (files);
}
