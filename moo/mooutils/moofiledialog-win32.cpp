/*
 *   moofiledialog-win32.cpp
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <mooutils/moofiledialog-win32.h>
#include <mooutils/mooutils-dialog-win32.h>
#include <mooutils/mooutils-macros.h>
#include <moocpp/gutil.h>
#include <gdk/gdkwin32.h>
#include <gtk/gtk.h>

#include <atomic>

#include <windows.h>
#include <shobjidl.h> 
#include <shlwapi.h>

using namespace moo;

#undef IGNORE
#define IGNORE(op) MOO_STMT_START { op; } MOO_STMT_END
#define CHECK(op) MOO_STMT_START { if (FAILED(hr = op)) goto err; } MOO_STMT_END

template<typename T>
void ReleaseAndNull(T*& p)
{
    if (p)
    {
        p->Release();
        p = nullptr;
    }
}

namespace ControlId {

enum
{
    combo = 1,
    combo_item1,
    combo_item2,
    combo_item3,
};

} // namespace ControlId

namespace {

class FileDialogCallback : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
    FileDialogCallback()
        : m_ref_count(1)
    {
    }

    MOO_DISABLE_COPY_OPS(FileDialogCallback);

private:
    ~FileDialogCallback()
    {
    }

public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(FileDialogCallback, IFileDialogEvents),
            QITABENT(FileDialogCallback, IFileDialogControlEvents),
            QITABENTMULTI(FileDialogCallback, IUnknown, IFileDialogEvents),
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++m_ref_count;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        int ret = --m_ref_count;
        if (ret == 0)
            delete this;
        return ret;
    }

    // IFileDialogEvents
    IFACEMETHODIMP OnFileOk(IFileDialog *pfd)
    {
        return S_OK; // S_FALSE keeps the dialog up, return S_OK to allows it to dismiss
    }

    IFACEMETHODIMP OnFolderChanging(IFileDialog *pDlg, IShellItem * /* psi */)
    {
        HookupHelper(pDlg);
        return S_OK;
    }

    IFACEMETHODIMP OnFolderChange(IFileDialog * /* pfd */)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnShareViolation(IFileDialog * /* pfd */, IShellItem * /* psi */, FDE_SHAREVIOLATION_RESPONSE * /* pResponse */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnTypeChange(IFileDialog * /* pfd */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnOverwrite(IFileDialog * /* pfd */, IShellItem * /* psi */, FDE_OVERWRITE_RESPONSE *pResponse)
    {
        *pResponse = FDEOR_DEFAULT;
        return S_OK;
    }

    // IFileDialogControlEvents
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, DWORD /* dwIDItem */)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *pfdc, DWORD dwIDCtl)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, BOOL /* bChecked */)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */)
    {
        return S_OK;
    }

private:
    void HookupHelper(IFileDialog* pDlg)
    {
        HRESULT hr = NOERROR;
        IOleWindow *pWindow = nullptr;

        CHECK(pDlg->QueryInterface(IID_PPV_ARGS(&pWindow)));
        HWND hwnd_dialog = nullptr;
        CHECK(pWindow->GetWindow(&hwnd_dialog));

        if (hwnd_dialog)
            m_helper.hookup(hwnd_dialog);

    err:
        ReleaseAndNull(pWindow);
    }

private:
    MooWinDialogHelper m_helper;
    std::atomic<int> m_ref_count;
};

class FileDialog
{
protected:
    FileDialog(HWND hwnd_parent, const gstr& start_folder, const gstr& start_name = gstr())
        : m_hwnd_parent(hwnd_parent)
        , m_start_folder(start_folder)
        , m_start_name(start_name)
    {
    }

    ~FileDialog()
    {
    }

    MOO_DISABLE_COPY_OPS(FileDialog);

    template<typename IRealDialog>
    HRESULT show_dialog(const CLSID& clsid, IRealDialog** ppDlg)
    {
        HRESULT hr = NOERROR;
        IRealDialog *pDlg = nullptr;
        IFileDialogEvents *pfde = nullptr;
        DWORD cookie = 0;

        CHECK(CoCreateInstance(clsid, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pDlg)));

        FILEOPENDIALOGOPTIONS fos = 0;
        CHECK(pDlg->GetOptions(&fos));
        fos |= get_additional_dialog_options();
        CHECK(pDlg->SetOptions(fos));

        IGNORE(set_start_folder(pDlg));
        IGNORE(set_start_basename(pDlg));

        CHECK(customize_dialog(pDlg));

        pfde = new FileDialogCallback();
        CHECK(pDlg->Advise(pfde, &cookie));

        CHECK(pDlg->Show(m_hwnd_parent));

        goto out;

err:
        ReleaseAndNull(pDlg);

out:
        if (cookie && pDlg)
            pDlg->Unadvise(cookie);

        ReleaseAndNull(pfde);

        *ppDlg = pDlg;
        return hr;
    }

    virtual HRESULT customize_dialog(IFileDialog* pDlg) const { return S_OK; }
    virtual FILEOPENDIALOGOPTIONS get_additional_dialog_options() const = 0;

private:
    HRESULT set_start_folder(IFileDialog* pDlg)
    {
        HRESULT hr = NOERROR;
        IShellItem* pItem = nullptr;
        WCHAR* pszPath = nullptr;
        gerrp err;

        if (m_start_folder.empty())
            goto out;

        pszPath = reinterpret_cast<WCHAR*>(g_utf8_to_utf16(m_start_folder, -1, nullptr, nullptr, &err));

        CHECK(SHCreateItemFromParsingName(pszPath, nullptr, IID_PPV_ARGS(&pItem)));

        CHECK(pDlg->SetFolder(pItem));

err:
out:
        ReleaseAndNull(pItem);
        g_free(pszPath);
        return hr;
    }

    HRESULT set_start_basename(IFileDialog* pDlg)
    {
        HRESULT hr = NOERROR;
        WCHAR* pszPath = nullptr;
        gerrp err;

        if (m_start_name.empty())
            goto out;

        pszPath = reinterpret_cast<WCHAR*>(g_utf8_to_utf16(m_start_name, -1, nullptr, nullptr, &err));
        CHECK(pDlg->SetFileName(pszPath));

err:
out:
        g_free(pszPath);
        return hr;
    }

private:
    HWND m_hwnd_parent;
    moo::gstr m_start_folder;
    moo::gstr m_start_name;
};

class OpenFilesDialog : public FileDialog
{
public:
    OpenFilesDialog(HWND hwnd_parent, const gstr& start_folder)
        : FileDialog(hwnd_parent, start_folder)
    {
    }

    ~OpenFilesDialog()
    {
    }

    gstrvec run()
    {
        HRESULT hr = NOERROR;
        IFileOpenDialog *pDlg = nullptr;
        IShellItemArray *pResults = nullptr;
        IShellItem *psi = nullptr;
        LPWSTR pszPath = nullptr;
        std::vector<gstr> filenames;

        CHECK(show_dialog(CLSID_FileOpenDialog, &pDlg));

        CHECK(pDlg->GetResults(&pResults));

        DWORD itemCount = 0;
        CHECK(pResults->GetCount(&itemCount));

        for (DWORD i = 0; i < itemCount; ++i)
        {
            CHECK(pResults->GetItemAt(i, &psi));
            CHECK(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath));

            gstr utf8_path = g::utf16_to_utf8(pszPath);
            filenames.emplace_back(std::move(utf8_path));

            CoTaskMemFree(pszPath);
            pszPath = nullptr;

            ReleaseAndNull(psi);
        }

        goto out;

err:
        filenames.clear();

out:
        CoTaskMemFree(pszPath);

        ReleaseAndNull(psi);
        ReleaseAndNull(pResults);
        ReleaseAndNull(pDlg);

        return filenames;
    }

private:
    FILEOPENDIALOGOPTIONS get_additional_dialog_options() const override
    {
        return FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT;
    }

    HRESULT customize_dialog(IFileOpenDialog* pDlg)
    {
        HRESULT hr = NOERROR;
        //IFileDialogCustomize* pCustomize = nullptr;
        //CHECK(pDlg->QueryInterface(IID_PPV_ARGS(&pCustomize)));
        //CHECK(pCustomize->EnableOpenDropDown(ControlId::combo));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item1, L"Open"));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item2, L"UTF-8"));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item3, L"More..."));
        //ReleaseAndNull(pCustomize);
        return hr;
    }
};

class SaveFileDialog : public FileDialog
{
public:
    SaveFileDialog(HWND hwnd_parent, const moo::gstr& start_folder, const moo::gstr& basename)
        : FileDialog(hwnd_parent, start_folder, basename)
    {
    }

    ~SaveFileDialog()
    {
    }

    gstr run()
    {
        HRESULT hr = NOERROR;
        IFileSaveDialog* pDlg = nullptr;
        LPWSTR pszPath = nullptr;
        IShellItem* psi = nullptr;
        gstr path;

        CHECK(show_dialog(CLSID_FileSaveDialog, &pDlg));

        CHECK(pDlg->GetResult(&psi));

        CHECK(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath));

        path = g::utf16_to_utf8(pszPath);

        goto out;

err:
        path.clear();

out:
        CoTaskMemFree(pszPath);
        ReleaseAndNull(psi);
        ReleaseAndNull(pDlg);
        return path;
    }

private:
    FILEOPENDIALOGOPTIONS get_additional_dialog_options() const override
    {
        return FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM;
    }

    HRESULT customize_dialog(IFileSaveDialog* pDlg)
    {
        HRESULT hr = NOERROR;
        //IFileDialogCustomize* pCustomize = nullptr;
        //CHECK(pDlg->QueryInterface(IID_PPV_ARGS(&pCustomize)));
        //CHECK(pCustomize->EnableOpenDropDown(ControlId::combo));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item1, L"Open"));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item2, L"UTF-8"));
        //CHECK(pCustomize->AddControlItem(ControlId::combo, ControlId::combo_item3, L"More..."));
        //ReleaseAndNull(pCustomize);
        return hr;
    }
};

} // anonymous namespace

std::vector<gstr> moo_show_win32_file_open_dialog(HWND hwnd_parent, const gstr& start_folder)
{
    OpenFilesDialog dlg(hwnd_parent, start_folder);
    return dlg.run();
}

moo::gstr moo_show_win32_file_save_as_dialog(HWND hwnd_parent, const moo::gstr& start_folder, const moo::gstr& basename)
{
    SaveFileDialog dlg(hwnd_parent, start_folder, basename);
    return dlg.run();
}
