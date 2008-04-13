/*
 *   moocompat.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

/*
 *  Some functions here are taken from GTK+ and GLIB (http://gtk.org/)
 *
 *  g_mkdir_with_parents and g_file_set_contents are taken from glib/gfileutils.c;
 *  it's Copyright 2000 Red Hat, Inc.
 *
 *  g_utf8_collate_key_for_filename is from glib/gunicollate.c:
 *  it's Copyright 2001,2005 Red Hat, Inc.
 *
 *  g_listenv function is taken from glib/gutils.c;
 *  it's Copyright (C) 1995-1998  Peter Mattis, Spencer Kimball and Josh MacDonald
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "mooutils/moocompat.h"
#include "mooutils/moo-environ.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <windows.h>
#include <io.h>
#endif /* G_OS_WIN32 */

#ifndef S_ISLNK
#define S_ISLNK(x) 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif


#if !GLIB_CHECK_VERSION(2,8,0)

int
g_mkdir_with_parents (const gchar *pathname,
		      int          mode)
{
  gchar *fn, *p;

  if (pathname == NULL || *pathname == '\0')
    {
      errno = EINVAL;
      return -1;
    }

  fn = g_strdup (pathname);

  if (g_path_is_absolute (fn))
    p = (gchar *) g_path_skip_root (fn);
  else
    p = fn;

  do
    {
      while (*p && !G_IS_DIR_SEPARATOR (*p))
	p++;

      if (!*p)
	p = NULL;
      else
	*p = '\0';

      if (!g_file_test (fn, G_FILE_TEST_EXISTS))
	{
	  if (g_mkdir (fn, mode) == -1)
	    {
	      int errno_save = errno;
	      g_free (fn);
	      errno = errno_save;
	      return -1;
	    }
	}
      else if (!g_file_test (fn, G_FILE_TEST_IS_DIR))
	{
	  g_free (fn);
	  errno = ENOTDIR;
	  return -1;
	}
      if (p)
	{
	  *p++ = G_DIR_SEPARATOR;
	  while (*p && G_IS_DIR_SEPARATOR (*p))
	    p++;
	}
    }
  while (p);

  g_free (fn);

  return 0;
}


/*
 * create_temp_file based on the mkstemp implementation from the GNU C library.
 * Copyright (C) 1991,92,93,94,95,96,97,98,99 Free Software Foundation, Inc.
 */
static gint
create_temp_file (gchar *tmpl,
		  int    permissions)
{
  char *XXXXXX;
  int count, fd;
  static const char letters[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static const int NLETTERS = sizeof (letters) - 1;
  glong value;
  GTimeVal tv;
  static int counter = 0;

  /* find the last occurrence of "XXXXXX" */
  XXXXXX = g_strrstr (tmpl, "XXXXXX");

  if (!XXXXXX || strncmp (XXXXXX, "XXXXXX", 6))
    {
      errno = EINVAL;
      return -1;
    }

  /* Get some more or less random data.  */
  g_get_current_time (&tv);
  value = (tv.tv_usec ^ tv.tv_sec) + counter++;

  for (count = 0; count < 100; value += 7777, ++count)
    {
      glong v = value;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[1] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[2] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[3] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[4] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[5] = letters[v % NLETTERS];

      /* tmpl is in UTF-8 on Windows, thus use g_open() */
      fd = g_open (tmpl, O_RDWR | O_CREAT | O_EXCL | O_BINARY, permissions);

      if (fd >= 0)
	return fd;
      else if (errno != EEXIST)
	/* Any other error will apply also to other names we might
	 *  try, and there are 2^32 or so of them, so give up now.
	 */
	return -1;
    }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return -1;
}

static gboolean
rename_file (const char *old_name,
	     const char *new_name,
	     GError **err)
{
  errno = 0;
  if (g_rename (old_name, new_name) == -1)
    {
      int save_errno = errno;
      gchar *display_old_name = g_filename_display_name (old_name);
      gchar *display_new_name = g_filename_display_name (new_name);

      g_set_error (err,
		   G_FILE_ERROR,
		   g_file_error_from_errno (save_errno),
		   "Failed to rename file '%s' to '%s': g_rename() failed: %s",
		   display_old_name,
		   display_new_name,
		   g_strerror (save_errno));

      g_free (display_old_name);
      g_free (display_new_name);

      return FALSE;
    }

  return TRUE;
}

static gchar *
write_to_temp_file (const gchar *contents,
		    gssize length,
		    const gchar *template,
		    GError **err)
{
  gchar *tmp_name;
  gchar *display_name;
  gchar *retval;
  FILE *file;
  gint fd;
  int save_errno;

  retval = NULL;

  tmp_name = g_strdup_printf ("%s.XXXXXX", template);

  errno = 0;
  fd = create_temp_file (tmp_name, 0666);
  display_name = g_filename_display_name (tmp_name);

  if (fd == -1)
    {
      save_errno = errno;
      g_set_error (err,
		   G_FILE_ERROR,
		   g_file_error_from_errno (save_errno),
		   "Failed to create file '%s': %s",
		   display_name, g_strerror (save_errno));

      goto out;
    }

  errno = 0;
  file = fdopen (fd, "wb");
  if (!file)
    {
      save_errno = errno;
      g_set_error (err,
		   G_FILE_ERROR,
		   g_file_error_from_errno (save_errno),
		   "Failed to open file '%s' for writing: fdopen() failed: %s",
		   display_name,
		   g_strerror (save_errno));

      close (fd);
      g_unlink (tmp_name);

      goto out;
    }

  if (length > 0)
    {
      gsize n_written;

      errno = 0;

      n_written = fwrite (contents, 1, length, file);

      if (n_written < (gsize) length)
	{
	  save_errno = errno;

 	  g_set_error (err,
		       G_FILE_ERROR,
		       g_file_error_from_errno (save_errno),
		       "Failed to write file '%s': fwrite() failed: %s",
		       display_name,
		       g_strerror (save_errno));

	  fclose (file);
	  g_unlink (tmp_name);

	  goto out;
	}
    }

  errno = 0;
  if (fclose (file) == EOF)
    {
      save_errno = 0;

      g_set_error (err,
		   G_FILE_ERROR,
		   g_file_error_from_errno (save_errno),
		   "Failed to close file '%s': fclose() failed: %s",
		   display_name,
		   g_strerror (save_errno));

      g_unlink (tmp_name);

      goto out;
    }

  retval = g_strdup (tmp_name);

 out:
  g_free (tmp_name);
  g_free (display_name);

  return retval;
}

gboolean
g_file_set_contents (const gchar *filename,
		     const gchar *contents,
		     gssize	     length,
		     GError	   **error)
{
  gchar *tmp_filename;
  gboolean retval;
  GError *rename_error = NULL;

  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (contents != NULL || length == 0, FALSE);
  g_return_val_if_fail (length >= -1, FALSE);

  if (length == -1)
    length = strlen (contents);

  tmp_filename = write_to_temp_file (contents, length, filename, error);

  if (!tmp_filename)
    {
      retval = FALSE;
      goto out;
    }

  if (!rename_file (tmp_filename, filename, &rename_error))
    {
#ifndef G_OS_WIN32

      g_unlink (tmp_filename);
      g_propagate_error (error, rename_error);
      retval = FALSE;
      goto out;

#else /* G_OS_WIN32 */

      /* Renaming failed, but on Windows this may just mean
       * the file already exists. So if the target file
       * exists, try deleting it and do the rename again.
       */
      if (!g_file_test (filename, G_FILE_TEST_EXISTS))
	{
	  g_unlink (tmp_filename);
	  g_propagate_error (error, rename_error);
	  retval = FALSE;
	  goto out;
	}

      g_error_free (rename_error);

      if (g_unlink (filename) == -1)
	{
          gchar *display_filename = g_filename_display_name (filename);

	  int save_errno = errno;

	  g_set_error (error,
		       G_FILE_ERROR,
		       g_file_error_from_errno (save_errno),
		       _("Existing file '%s' could not be removed: g_unlink() failed: %s"),
		       display_filename,
		       g_strerror (save_errno));

	  g_free (display_filename);
	  g_unlink (tmp_filename);
	  retval = FALSE;
	  goto out;
	}

      if (!rename_file (tmp_filename, filename, error))
	{
	  g_unlink (tmp_filename);
	  retval = FALSE;
	  goto out;
	}

#endif
    }

  retval = TRUE;

 out:
  g_free (tmp_filename);
  return retval;
}


/* This is a collation key that is very very likely to sort before any
   collation key that libc strxfrm generates. We use this before any
   special case (dot or number) to make sure that its sorted before
   anything else.
 */
#define COLLATION_SENTINEL "\1\1\1"

gchar*
g_utf8_collate_key_for_filename (const gchar *str,
				 gssize       len)
{
  GString *result;
  GString *append;
  const gchar *p;
  const gchar *prev;
  const gchar *end;
  gchar *collate_key;
  gint digits;
  gint leading_zeros;

  /*
   * How it works:
   *
   * Split the filename into collatable substrings which do
   * not contain [.0-9] and special-cased substrings. The collatable
   * substrings are run through the normal g_utf8_collate_key() and the
   * resulting keys are concatenated with keys generated from the
   * special-cased substrings.
   *
   * Special cases: Dots are handled by replacing them with '\1' which
   * implies that short dot-delimited substrings are before long ones,
   * e.g.
   *
   *   a\1a   (a.a)
   *   a-\1a  (a-.a)
   *   aa\1a  (aa.a)
   *
   * Numbers are handled by prepending to each number d-1 superdigits
   * where d = number of digits in the number and SUPERDIGIT is a
   * character with an integer value higher than any digit (for instance
   * ':'). This ensures that single-digit numbers are sorted before
   * double-digit numbers which in turn are sorted separately from
   * triple-digit numbers, etc. To avoid strange side-effects when
   * sorting strings that already contain SUPERDIGITs, a '\2'
   * is also prepended, like this
   *
   *   file\21      (file1)
   *   file\25      (file5)
   *   file\2:10    (file10)
   *   file\2:26    (file26)
   *   file\2::100  (file100)
   *   file:foo     (file:foo)
   *
   * This has the side-effect of sorting numbers before everything else (except
   * dots), but this is probably OK.
   *
   * Leading digits are ignored when doing the above. To discriminate
   * numbers which differ only in the number of leading digits, we append
   * the number of leading digits as a byte at the very end of the collation
   * key.
   *
   * To try avoid conflict with any collation key sequence generated by libc we
   * start each switch to a special cased part with a sentinel that hopefully
   * will sort before anything libc will generate.
   */

  if (len < 0)
    len = strlen (str);

  result = g_string_sized_new (len * 2);
  append = g_string_sized_new (0);

  end = str + len;

  /* No need to use utf8 functions, since we're only looking for ascii chars */
  for (prev = p = str; p < end; p++)
    {
      switch (*p)
	{
	case '.':
	  if (prev != p)
	    {
	      collate_key = g_utf8_collate_key (prev, p - prev);
	      g_string_append (result, collate_key);
	      g_free (collate_key);
	    }

	  g_string_append (result, COLLATION_SENTINEL "\1");

	  /* skip the dot */
	  prev = p + 1;
	  break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  if (prev != p)
	    {
	      collate_key = g_utf8_collate_key (prev, p - prev);
	      g_string_append (result, collate_key);
	      g_free (collate_key);
	    }

	  g_string_append (result, COLLATION_SENTINEL "\2");

	  prev = p;

	  /* write d-1 colons */
	  if (*p == '0')
	    {
	      leading_zeros = 1;
	      digits = 0;
	    }
	  else
	    {
	      leading_zeros = 0;
	      digits = 1;
	    }

	  while (++p < end)
	    {
	      if (*p == '0' && !digits)
		++leading_zeros;
	      else if (g_ascii_isdigit(*p))
		++digits;
	      else
                {
 		  /* count an all-zero sequence as
                   * one digit plus leading zeros
                   */
          	  if (!digits)
                    {
                      ++digits;
                      --leading_zeros;
                    }
		  break;
                }
	    }

	  while (digits > 1)
	    {
	      g_string_append_c (result, ':');
	      --digits;
	    }

	  if (leading_zeros > 0)
	    {
	      g_string_append_c (append, (char)leading_zeros);
	      prev += leading_zeros;
	    }

	  /* write the number itself */
	  g_string_append_len (result, prev, p - prev);

	  prev = p;
	  --p;	  /* go one step back to avoid disturbing outer loop */
	  break;

	default:
	  /* other characters just accumulate */
	  break;
	}
    }

  if (prev != p)
    {
      collate_key = g_utf8_collate_key (prev, p - prev);
      g_string_append (result, collate_key);
      g_free (collate_key);
    }

  g_string_append (result, append->str);
  g_string_free (append, TRUE);

  return g_string_free (result, FALSE);
}


gchar **
g_listenv (void)
{
#ifndef G_OS_WIN32
  gchar **result, *eq;
  gint len, i, j;

  len = g_strv_length (environ);
  result = g_new0 (gchar *, len + 1);

  j = 0;
  for (i = 0; i < len; i++)
    {
      eq = strchr (environ[i], '=');
      if (eq)
	result[j++] = g_strndup (environ[i], eq - environ[i]);
    }

  result[j] = NULL;

  return result;
#else
  gchar **result, *eq;
  gint len = 0, i, j;

  if (G_WIN32_HAVE_WIDECHAR_API ())
    {
      wchar_t *p, *q;

      p = (wchar_t *) GetEnvironmentStringsW ();
      if (p != NULL)
	{
	  q = p;
	  while (*q)
	    {
	      q += wcslen (q) + 1;
	      len++;
	    }
	}
      result = g_new0 (gchar *, len + 1);

      j = 0;
      q = p;
      while (*q)
	{
	  result[j] = g_utf16_to_utf8 (q, -1, NULL, NULL, NULL);
	  if (result[j] != NULL)
	    {
	      eq = strchr (result[j], '=');
	      if (eq && eq > result[j])
		{
		  *eq = '\0';
		  j++;
		}
	      else
		g_free (result[j]);
	    }
	  q += wcslen (q) + 1;
	}
      result[j] = NULL;
      FreeEnvironmentStringsW (p);
    }
  else
    {
      len = g_strv_length (environ);
      result = g_new0 (gchar *, len + 1);

      j = 0;
      for (i = 0; i < len; i++)
	{
	  result[j] = g_locale_to_utf8 (environ[i], -1, NULL, NULL, NULL);
	  if (result[j] != NULL)
	    {
	      eq = strchr (result[j], '=');
	      if (eq && eq > result[j])
		{
		  *eq = '\0';
		  j++;
		}
	      else
		g_free (result[j]);
	    }
	}
      result[j] = NULL;
    }

  return result;
#endif
}

#endif /* !GLIB_CHECK_VERSION(2,8,0) */


#if !GTK_CHECK_VERSION(2,10,0)

GdkAtom
gdk_atom_intern_static_string (const char *atom_name)
{
    return gdk_atom_intern (atom_name, FALSE);
}

GType
gtk_unit_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
    {
        static const GEnumValue values[] = {
            { GTK_UNIT_PIXEL, (char*) "GTK_UNIT_PIXEL", (char*) "pixel" },
            { GTK_UNIT_POINTS, (char*) "GTK_UNIT_POINTS", (char*) "points" },
            { GTK_UNIT_INCH, (char*) "GTK_UNIT_INCH", (char*) "inch" },
            { GTK_UNIT_MM, (char*) "GTK_UNIT_MM", (char*) "mm" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("GtkUnit", values);
    }

    return type;
}

#endif /* !GTK_CHECK_VERSION(2,10,0) */


#if !GLIB_CHECK_VERSION(2,14,0)

void
g_string_append_vprintf (GString    *string,
                         const char *format,
                         va_list     args)
{
    char *buf;
    int len;

    g_return_if_fail (string != NULL);
    g_return_if_fail (format != NULL);

    len = g_vasprintf (&buf, format, args);

    if (len > 0)
        g_string_append_len (string, buf, len);

    g_free (buf);
}

#endif /* !GLIB_CHECK_VERSION(2,14,0) */
