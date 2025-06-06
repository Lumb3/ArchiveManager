
#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/thread.h>
#include <vector>
#include <string>
#include <memory>
#include "PathOptimizer.h"

class EnhancedZipPanel : public wxPanel {
public:
    explicit EnhancedZipPanel(wxWindow* parent);
    ~EnhancedZipPanel() override;

    // Disable copy operations
    EnhancedZipPanel(const EnhancedZipPanel&) = delete;
    EnhancedZipPanel& operator=(const EnhancedZipPanel&) = delete;

private:
    // Event handlers
    void OnBrowseFiles(wxCommandEvent& event);
    void OnBrowseFolder(wxCommandEvent& event);
    void OnRemoveSelected(wxCommandEvent& event);
    void OnClearAll(wxCommandEvent& event);
    void OnCreateArchive(wxCommandEvent& event);
    void OnBrowseOutput(wxCommandEvent& event);
    void OnCompressionChange(wxCommandEvent& event);
    void OnOptimizeOrder(wxCommandEvent& event);

    // UI Components
    wxStaticText* m_titleLabel{nullptr};
    wxListCtrl* m_fileList{nullptr};
    wxButton* m_browseFilesBtn{nullptr};
    wxButton* m_browseFolderBtn{nullptr};
    wxButton* m_removeBtn{nullptr};
    wxButton* m_clearBtn{nullptr};
    wxButton* m_optimizeBtn{nullptr};

    wxStaticText* m_outputLabel{nullptr};
    wxTextCtrl* m_outputPath{nullptr};
    wxButton* m_browseOutputBtn{nullptr};

    wxStaticText* m_compressionLabel{nullptr};
    wxChoice* m_compressionLevel{nullptr};

    wxButton* m_createBtn{nullptr};
    wxGauge* m_progressBar{nullptr};
    wxStaticText* m_statusText{nullptr};

    // Data members
    std::vector<std::string> m_selectedFiles;
    std::unique_ptr<PathOptimizer> m_pathOptimizer;
    wxMutex m_mutex; // For thread safety

    // Helper methods
    void setupUI();
    void updateFileList();
    void addFilesToList(const std::vector<std::string>& files);
    bool createZipArchive(const std::string& outputPath,
                         const std::vector<std::string>& files,
                         int compressionLevel);
    void updateProgress(int percent, const std::string& status);
    std::string getDefaultOutputPath() const;

    enum {
        ID_BROWSE_FILES = wxID_HIGHEST + 1,
        ID_BROWSE_FOLDER,
        ID_REMOVE_SELECTED,
        ID_CLEAR_ALL,
        ID_CREATE_ARCHIVE,
        ID_BROWSE_OUTPUT,
        ID_COMPRESSION_CHANGE,
        ID_OPTIMIZE_ORDER
    };

    wxDECLARE_EVENT_TABLE();
};