// Author: Erkhembileg Ariunbold
// Project: ArchiveManager
// Date: 2025.06.06

#include "EnhancedZipPanel.h"
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/gauge.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/listctrl.h>

#include "zip.h"
#include "zlib.h"

wxBEGIN_EVENT_TABLE(EnhancedZipPanel, wxPanel)
    EVT_BUTTON(ID_BROWSE_FILES, EnhancedZipPanel::OnBrowseFiles)
    EVT_BUTTON(ID_BROWSE_FOLDER, EnhancedZipPanel::OnBrowseFolder)
    EVT_BUTTON(ID_REMOVE_SELECTED, EnhancedZipPanel::OnRemoveSelected)
    EVT_BUTTON(ID_CLEAR_ALL, EnhancedZipPanel::OnClearAll)
    EVT_BUTTON(ID_CREATE_ARCHIVE, EnhancedZipPanel::OnCreateArchive)
    EVT_BUTTON(ID_BROWSE_OUTPUT, EnhancedZipPanel::OnBrowseOutput)
    EVT_CHOICE(ID_COMPRESSION_CHANGE, EnhancedZipPanel::OnCompressionChange)
    EVT_BUTTON(ID_OPTIMIZE_ORDER, EnhancedZipPanel::OnOptimizeOrder)
wxEND_EVENT_TABLE()

EnhancedZipPanel::EnhancedZipPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_pathOptimizer(std::make_unique<PathOptimizer>())
{
    setupUI();
}

EnhancedZipPanel::~EnhancedZipPanel() = default;

void EnhancedZipPanel::updateFileList()
{
    m_fileList->DeleteAllItems();
    for (size_t i = 0; i < m_selectedFiles.size(); ++i) {
        const auto& filePath = m_selectedFiles[i];
        auto filename = std::filesystem::path(filePath).filename().string();
        m_fileList->InsertItem(i, filename);
        m_fileList->SetItem(i, 1, filePath);
    }
}

void EnhancedZipPanel::OnOptimizeOrder(wxCommandEvent& event)
{
    wxMutexLocker lock(m_mutex);

    if (m_selectedFiles.empty()) {
        wxMessageBox("Please add files to optimize", "No Files Selected",
                    wxOK | wxICON_INFORMATION);
        return;
    }

    m_optimizeBtn->Disable();
    m_createBtn->Disable();
    m_browseFilesBtn->Disable();
    m_browseFolderBtn->Disable();
    updateProgress(0, "Optimizing file order...");

    m_pathOptimizer->Clear();

    // Add files to optimizer
    for (const auto& filePath : m_selectedFiles) {
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(filePath, ec);
        if (!ec) {
            m_pathOptimizer->AddFile(filePath, fileSize);
        }
    }

    // Build graph and get optimized order
    m_pathOptimizer->BuildCompressionGraph();
    auto optimizedFiles = m_pathOptimizer->GetOptimizedFileOrder();

    // Update file list with new order
    m_selectedFiles.clear();
    for (const auto& fileNode : optimizedFiles) {
        m_selectedFiles.push_back(fileNode.path);
    }

    updateFileList();
    updateProgress(100, "File order optimized for compression");

    m_optimizeBtn->Enable();
    m_createBtn->Enable();
    m_browseFilesBtn->Enable();
    m_browseFolderBtn->Enable();

    wxMessageBox("Files reordered for optimal compression", "Optimization Complete",
                wxOK | wxICON_INFORMATION);
}

void EnhancedZipPanel::OnCreateArchive(wxCommandEvent& event)
{
    wxMutexLocker lock(m_mutex);

    if (m_selectedFiles.empty()) {
        wxMessageBox("Please add files to create archive", "No Files Selected",
                    wxOK | wxICON_INFORMATION);
        return;
    }

    wxString outputPath = m_outputPath->GetValue();
    if (outputPath.empty()) {
        wxMessageBox("Please specify output file path", "No Output Path",
                    wxOK | wxICON_INFORMATION);
        return;
    }

    m_createBtn->Disable();
    m_browseFilesBtn->Disable();
    m_browseFolderBtn->Disable();
    m_optimizeBtn->Disable();

    int compressionLevel = 6;
    switch (m_compressionLevel->GetSelection()) {
        case 0: compressionLevel = Z_NO_COMPRESSION; break;
        case 1: compressionLevel = 1; break;
        case 2: compressionLevel = 3; break;
        case 3: compressionLevel = 6; break;
        case 4: compressionLevel = Z_BEST_COMPRESSION; break;
    }

    std::vector<std::string> files = m_selectedFiles;

    std::thread([this, outputPath, compressionLevel, files]() {
        bool success = createZipArchive(outputPath.ToStdString(), files, compressionLevel);

        CallAfter([this, success]() {
            m_createBtn->Enable();
            m_browseFilesBtn->Enable();
            m_browseFolderBtn->Enable();
            m_optimizeBtn->Enable();

            if (success) {
                wxMessageBox("Archive created successfully!", "Success",
                           wxOK | wxICON_INFORMATION);
            }
        });
    }).detach();
}
#include <zip.h>

bool EnhancedZipPanel::createZipArchive(const std::string& outputPath,
                                       const std::vector<std::string>& files,
                                       int compressionLevel)
{
    int error;
    zip_t *archive = zip_open(outputPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!archive) {
        char errorMsg[100];
        zip_error_to_str(errorMsg, sizeof(errorMsg), error, errno);
        updateProgress(0, std::string("Failed to create archive: ") + errorMsg);
        return false;
    }

    int filesAdded = 0;
    for (const auto& filePath : files) {
        std::error_code ec;
        if (!std::filesystem::exists(filePath, ec)) {
            updateProgress(0, "File not found: " + filePath);
            continue;
        }

        auto filename = std::filesystem::path(filePath).filename().string();

        zip_source_t *source = zip_source_file(archive, filePath.c_str(), 0, -1);
        if (!source) {
            updateProgress(0, "Failed to create source for: " + filename);
            continue;
        }

        zip_int64_t index = zip_file_add(archive, filename.c_str(), source, ZIP_FL_OVERWRITE);
        if (index < 0) {
            zip_source_free(source);
            updateProgress(0, "Failed to add file: " + filename);
            continue;
        }

        // Set compression level
        if (zip_set_file_compression(archive, index, ZIP_CM_DEFLATE,
                                   static_cast<uint32_t>(compressionLevel)) < 0) {
            updateProgress(0, "Failed to set compression for: " + filename);
            continue;
                                   }

        filesAdded++;
        updateProgress((filesAdded * 100) / files.size(), "Added: " + filename);
    }

    if (zip_close(archive) < 0) {
        updateProgress(0, "Failed to finalize archive");
        return false;
    }

    updateProgress(100, "Archive created successfully");
    return filesAdded > 0;
}

void EnhancedZipPanel::updateProgress(int percent, const std::string& status)
{
    CallAfter([this, percent, status]() {
        m_progressBar->SetValue(percent);
        m_statusText->SetLabel(status);
    });
}


void EnhancedZipPanel::setupUI() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    m_titleLabel = new wxStaticText(this, wxID_ANY, "Enhanced ZIP Archiver");
    wxFont titleFont = m_titleLabel->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_titleLabel->SetFont(titleFont);
    mainSizer->Add(m_titleLabel, 0, wxALL, 5);

    // File list
    m_fileList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300),
                               wxLC_REPORT | wxLC_SINGLE_SEL);
    m_fileList->InsertColumn(0, "File Name", wxLIST_FORMAT_LEFT, 500);
    m_fileList->InsertColumn(1, "Path", wxLIST_FORMAT_LEFT, 300);
    mainSizer->Add(m_fileList, 1, wxEXPAND | wxALL, 5);

    // Buttons panel
    auto* buttonPanel = new wxPanel(this);
    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_browseFilesBtn = new wxButton(buttonPanel, ID_BROWSE_FILES, "Add Files");
    m_browseFolderBtn = new wxButton(buttonPanel, ID_BROWSE_FOLDER, "Add Folder");
    m_removeBtn = new wxButton(buttonPanel, ID_REMOVE_SELECTED, "Remove");
    m_clearBtn = new wxButton(buttonPanel, ID_CLEAR_ALL, "Clear All");
    m_optimizeBtn = new wxButton(buttonPanel, ID_OPTIMIZE_ORDER, "Optimize Order");

    buttonSizer->Add(m_browseFilesBtn, 0, wxRIGHT, 5);
    buttonSizer->Add(m_browseFolderBtn, 0, wxRIGHT, 5);
    buttonSizer->Add(m_removeBtn, 0, wxRIGHT, 5);
    buttonSizer->Add(m_clearBtn, 0, wxRIGHT, 5);
    buttonSizer->Add(m_optimizeBtn, 0);

    buttonPanel->SetSizer(buttonSizer);
    mainSizer->Add(buttonPanel, 0, wxALL, 5);

    // Output path
    auto* outputPanel = new wxPanel(this);
    auto* outputSizer = new wxBoxSizer(wxHORIZONTAL);

    m_outputLabel = new wxStaticText(outputPanel, wxID_ANY, "Output:");
    m_outputPath = new wxTextCtrl(outputPanel, wxID_ANY, getDefaultOutputPath());
    m_browseOutputBtn = new wxButton(outputPanel, ID_BROWSE_OUTPUT, "Browse");

    outputSizer->Add(m_outputLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    outputSizer->Add(m_outputPath, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    outputSizer->Add(m_browseOutputBtn, 0, wxALIGN_CENTER_VERTICAL);

    outputPanel->SetSizer(outputSizer);
    mainSizer->Add(outputPanel, 0, wxEXPAND | wxALL, 5);

    // Compression options
    auto* compressionPanel = new wxPanel(this);
    auto* compressionSizer = new wxBoxSizer(wxHORIZONTAL);

    m_compressionLabel = new wxStaticText(compressionPanel, wxID_ANY, "Compression:");
    m_compressionLevel = new wxChoice(compressionPanel, ID_COMPRESSION_CHANGE);
    m_compressionLevel->Append(std::vector<wxString>{"None", "Fastest", "Fast", "Normal", "Best"});
    m_compressionLevel->SetSelection(3); // Normal compression by default

    compressionSizer->Add(m_compressionLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    compressionSizer->Add(m_compressionLevel, 0, wxALIGN_CENTER_VERTICAL);

    compressionPanel->SetSizer(compressionSizer);
    mainSizer->Add(compressionPanel, 0, wxALL, 5);

    // Create archive button
    m_createBtn = new wxButton(this, ID_CREATE_ARCHIVE, "Create Archive");
    mainSizer->Add(m_createBtn, 0, wxALL | wxALIGN_RIGHT, 5);

    // Progress bar and status
    m_progressBar = new wxGauge(this, wxID_ANY, 100);
    mainSizer->Add(m_progressBar, 0, wxEXPAND | wxALL, 5);

    m_statusText = new wxStaticText(this, wxID_ANY, "Ready");
    mainSizer->Add(m_statusText, 0, wxALL, 5);

    SetSizer(mainSizer);
}

void EnhancedZipPanel::addFilesToList(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        m_selectedFiles.push_back(file);
    }
    updateFileList();
}

std::string EnhancedZipPanel::getDefaultOutputPath() const {
    wxString documentsDir = wxStandardPaths::Get().GetDocumentsDir();
    return (documentsDir + wxFILE_SEP_PATH + "archive.zip").ToStdString();
}


void EnhancedZipPanel::OnClearAll(wxCommandEvent& event) {
    if (m_fileList) {
        m_fileList->DeleteAllItems();
        m_selectedFiles.clear();
    }
}

void EnhancedZipPanel::OnBrowseFiles(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Choose files to compress", "", "",
                       "All files (*.*)|*.*",
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxArrayString paths;
    dialog.GetPaths(paths);

    for (const auto& path : paths) {
        m_selectedFiles.push_back(path.ToStdString());
    }
    updateFileList();
}

void EnhancedZipPanel::OnBrowseFolder(wxCommandEvent& event) {
    wxDirDialog dialog(this, "Choose a folder to compress", "",
                      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString dirPath = dialog.GetPath();
    std::filesystem::path folder(dirPath.ToStdString());

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                m_selectedFiles.push_back(entry.path().string());
            }
        }
        updateFileList();
    } catch (const std::filesystem::filesystem_error& e) {
        wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
    }
}

void EnhancedZipPanel::OnBrowseOutput(wxCommandEvent& event) {
    wxFileDialog dialog(this, "Save ZIP file", "", "",
                       "ZIP files (*.zip)|*.zip",
                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    m_outputPath->SetValue(dialog.GetPath());
}

void EnhancedZipPanel::OnRemoveSelected(wxCommandEvent& event) {
    if (!m_fileList) {
        return;
    }

    long item = m_fileList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1) {
        return;
    }

    wxString path = m_fileList->GetItemText(item, 1);
    auto it = std::find(m_selectedFiles.begin(), m_selectedFiles.end(), path.ToStdString());
    if (it != m_selectedFiles.end()) {
        m_selectedFiles.erase(it);
    }

    m_fileList->DeleteItem(item);
}

void EnhancedZipPanel::OnCompressionChange(wxCommandEvent& event) {
    // Nothing special needed here as the compression level is read directly
    // when creating the archive
    event.Skip();
}
