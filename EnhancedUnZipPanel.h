// Author: Erkhembileg Ariunbold
// Project: ArchiveManager
// Date: 2025.06.06

#ifndef ENHANCED_UNZIP_PANEL_H
#define ENHANCED_UNZIP_PANEL_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/progdlg.h>
#include <wx/dir.h>
#include <memory>

// Control IDs
constexpr int ID_LOAD_ZIP = 1001;
constexpr int ID_EXTRACT_ALL = 1002;
constexpr int ID_EXTRACT_SELECTED = 1003;
constexpr int ID_FILE_LIST = 1004;

class EnhancedUnZipPanel : public wxPanel
{
public:
    EnhancedUnZipPanel(wxWindow* parent);

private:
    // UI setup helpers
    void SetupUI();
    void SetupFileList();
    void SetupButtons();
    void SetupProgressBar();

    // Functional logic
    bool LoadArchiveEntries();
    void ExtractAll(const wxString& destPath);
    void ExtractSelected(const wxString& destPath);
    void EnableControls(bool enable);

    // Event handlers
    void OnLoadZip(wxCommandEvent& event);
    void OnExtractAll(wxCommandEvent& event);
    void OnExtractSelected(wxCommandEvent& event);
    void OnItemSelect(wxListEvent& event);

    // UI components
    std::unique_ptr<wxListCtrl> m_fileList;
    std::unique_ptr<wxButton> m_loadZipButton;
    std::unique_ptr<wxButton> m_extractButton;
    std::unique_ptr<wxButton> m_extractAllButton;
    std::unique_ptr<wxGauge> m_progressBar;
    std::unique_ptr<wxStaticText> m_statusText;

    // Internal state
    wxString m_archivePath;

    wxDECLARE_EVENT_TABLE();
};

#endif // ENHANCED_UNZIP_PANEL_H
