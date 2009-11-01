#include "mooscript.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>

void
mom_value_init (MomValue *mv)
{
    g_return_if_fail (mv != NULL);
    memset (mv, 0, sizeof *mv);
}

void
mom_value_free (MomValue *mv)
{
    g_return_if_fail (MOM_IS_VALUE(mv));

    switch (mv->type)
    {
        case MOM_VALUE_NONE:
        case MOM_VALUE_HANDLE:
        case MOM_VALUE_BOOL:
        case MOM_VALUE_INT:
        case MOM_VALUE_UINT:
        case MOM_VALUE_LONG:
        case MOM_VALUE_ULONG:
        case MOM_VALUE_INT64:
        case MOM_VALUE_UINT64:
            break;

        case MOM_VALUE_STRING:
            mom_string_free (&mv->u.s);
            break;

        case MOM_VALUE_BYTE_ARRAY:
            mom_byte_array_free (&mv->u.ba);
            break;

        case MOM_VALUE_INVALID:
            g_assert_not_reached();
            break;
    }

    mv->type = 0;
}

MomResult
mom_value_copy (const MomValue *mv_src,
                MomValue       *mv_dst)
{
    MomResult mr = MOM_SUCCESS;
    MomValue mv = {0};

    g_return_val_if_fail (MOM_IS_VALUE(mv_src), MOM_UNEXPECTED);
    g_return_val_if_fail (MOM_IS_VALUE(mv_dst), MOM_UNEXPECTED);

    switch (mv_src->type)
    {
        case MOM_VALUE_NONE:
        case MOM_VALUE_HANDLE:
        case MOM_VALUE_BOOL:
        case MOM_VALUE_INT:
        case MOM_VALUE_UINT:
        case MOM_VALUE_LONG:
        case MOM_VALUE_ULONG:
        case MOM_VALUE_INT64:
        case MOM_VALUE_UINT64:
            mv = *mv_src;
            break;

        case MOM_VALUE_STRING:
            if (MOM_SUCCEEDED (mr = mom_string_copy (&mv_src->u.s, &mv.u.s)))
                mv.type = MOM_VALUE_STRING;
            break;

        case MOM_VALUE_BYTE_ARRAY:
            if (MOM_SUCCEEDED (mr = mom_byte_array_copy (&mv_src->u.ba, &mv.u.ba)))
                mv.type = MOM_VALUE_BYTE_ARRAY;
            break;

        case MOM_VALUE_INVALID:
            g_assert_not_reached();
            break;
    }

    if (MOM_SUCCEEDED (mr))
    {
        mom_value_free (mv_dst);
        *mv_dst = mv;
    }
    else
    {
        mom_value_free (&mv);
    }

    return mr;
}

MomResult
mom_value_init_copy (const MomValue *mv_src,
                     MomValue       *mv_dst)
{
    g_return_val_if_fail (MOM_IS_VALUE(mv_src), MOM_UNEXPECTED);
    g_return_val_if_fail (mv_dst != NULL, MOM_UNEXPECTED);
    mom_value_init (mv_dst);
    return mom_value_copy (mv_src, mv_dst);
}


struct MomStringImpl
{
    char *str;
    guint ref_count;
};

MomResult
mom_string_init (MomString  *ms,
                 const char *src)
{
    g_return_val_if_fail (ms != NULL, MOM_UNEXPECTED);
    ms->p = NULL;

    g_return_val_if_fail (src != NULL, MOM_UNEXPECTED);

    ms->p = moo_new (MomStringImpl);
    ms->p->str = g_strdup (src);
    ms->p->ref_count = 1;

    return MOM_SUCCESS;
}

void
mom_string_free (MomString *ms)
{
    g_return_if_fail (ms != NULL);
    g_return_if_fail (ms->p != NULL);

    if (--ms->p->ref_count == 0)
    {
        g_free (ms->p->str);
        moo_free (MomStringImpl, ms->p);
        ms->p = NULL;
    }
}

MomResult
mom_string_copy (const MomString *ms_src,
                 MomString       *ms_dst)
{
    g_return_val_if_fail (ms_src != NULL, MOM_UNEXPECTED);
    g_return_val_if_fail (ms_dst != NULL, MOM_UNEXPECTED);

    if (ms_src->p != ms_dst->p)
    {
        mom_string_free (ms_dst);
        ms_dst->p = ms_src->p;
        ms_dst->p->ref_count++;
    }

    return MOM_SUCCESS;
}

MomResult
mom_string_set (MomString  *ms,
                const char *str)
{
    MomResult mr;

    g_return_val_if_fail (ms != NULL, MOM_UNEXPECTED);
    g_return_val_if_fail (str != NULL, MOM_UNEXPECTED);

    if (ms->p->ref_count == 1)
    {
        char *tmp = ms->p->str;
        ms->p->str = g_strdup (str);
        g_free (tmp);
        mr = MOM_SUCCESS;
    }
    else
    {
        MomString ms_tmp;
        if (MOM_SUCCEEDED(mr = mom_string_init (&ms_tmp, str)))
        {
            mom_string_free (ms);
            *ms = ms_tmp;
        }
    }

    return mr;
}

const char *
mom_string_get (MomString *ms)
{
    g_return_val_if_fail (ms != NULL, NULL);
    g_return_val_if_fail (ms->p != NULL, NULL);
    return ms->p->str;
}


struct MomByteArrayImpl
{
    guchar *data;
    guint len;
    guint ref_count;
};

MomResult
mom_byte_array_init (MomByteArray *mba,
                     const guchar *src,
                     guint         len)
{
    g_return_val_if_fail (mba != NULL, MOM_UNEXPECTED);
    mba->p = NULL;

    g_return_val_if_fail (src != NULL || len == 0, MOM_UNEXPECTED);

    mba->p = moo_new (MomByteArrayImpl);
    mba->p->data = len == 0 ? NULL : g_memdup (src, len);
    mba->p->ref_count = 1;

    return MOM_SUCCESS;
}

void
mom_byte_array_free (MomByteArray *mba)
{
    g_return_if_fail (mba != NULL);
    g_return_if_fail (mba->p != NULL);

    if (--mba->p->ref_count == 0)
    {
        g_free (mba->p->data);
        moo_free (MomByteArrayImpl, mba->p);
        mba->p = NULL;
    }
}

MomResult
mom_byte_array_copy (const MomByteArray *mba_src,
                     MomByteArray       *mba_dst)
{
    g_return_val_if_fail (mba_src != NULL, MOM_UNEXPECTED);
    g_return_val_if_fail (mba_dst != NULL, MOM_UNEXPECTED);

    if (mba_src->p != mba_dst->p)
    {
        mom_byte_array_free (mba_dst);
        mba_dst->p = mba_src->p;
        mba_dst->p->ref_count++;
    }

    return MOM_SUCCESS;
}

MomResult
mom_byte_array_set (MomByteArray *mba,
                    const guchar *src,
                    guint         len)
{
    MomResult mr;

    g_return_val_if_fail (mba != NULL, MOM_UNEXPECTED);
    g_return_val_if_fail (src != NULL || len == 0, MOM_UNEXPECTED);

    if (mba->p->ref_count == 1)
    {
        guchar *tmp = mba->p->data;
        mba->p->data = len == 0 ? NULL : g_memdup (src, len);
        g_free (tmp);
        mr = MOM_SUCCESS;
    }
    else
    {
        MomByteArray mba_tmp;
        if (MOM_SUCCEEDED(mr = mom_byte_array_init (&mba_tmp, src, len)))
        {
            mom_byte_array_free (mba);
            *mba = mba_tmp;
        }
    }

    return mr;
}

const guchar *
mom_byte_array_get (MomByteArray *mba,
                    guint        *len)
{
    g_return_val_if_fail (mba != NULL, NULL);
    g_return_val_if_fail (mba->p != NULL, NULL);
    if (len != NULL)
        *len = mba->p->len;
    return mba->p->data ? mba->p->data : (const guchar*) "";
}


/* -%- strip:true -%- */
