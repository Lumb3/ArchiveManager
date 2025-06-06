// Author: Erkhembileg Ariunbold
// Project: ArchiveManager
// Date: 2025.06.06

#include "EnhancedUnZipPanel.h"

wxBEGIN_EVENT_TABLE(EnhancedUnZipPanel, wxPanel)
    EVT_BUTTON(ID_LOAD_ZIP, EnhancedUnZipPanel::OnLoadZip)
    EVT_BUTTON(ID_EXTRACT_ALL, EnhancedUnZipPanel::OnExtractAll)
    EVT_BUTTON(ID_EXTRACT_SELECTED, EnhancedUnZipPanel::OnExtractSelected)
    EVT_LIST_ITEM_SELECTED(ID_FILE_LIST, EnhancedUnZipPanel::OnItemSelect)
wxEND_EVENT_TABLE();

EnhancedUnZipPanel::EnhancedUnZipPanel(wxWindow* parent)
    : wxPanel(parent)
{
    SetupUI();
    EnableControls(false);
}

void EnhancedUnZipPanel::SetupUI()
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    SetupFileList();
    SetupButtons();
    SetupProgressBar();

    m_statusText = std::make_unique<wxStaticText>(this, wxID_ANY, "Ready");

    mainSizer->Add(m_fileList.get(), 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_progressBar.get(), 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_loadZipButton.get(), 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_extractButton.get(), 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_extractAllButton.get(), 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_statusText.get(), 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
}

void EnhancedUnZipPanel::SetupFileList()
{
    m_fileList = std::make_unique<wxListCtrl>(this, ID_FILE_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_fileList->InsertColumn(0, "File Name", wxLIST_FORMAT_LEFT, 500);
    m_fileList->InsertColumn(1, "Size", wxLIST_FORMAT_LEFT, 300);
}

void EnhancedUnZipPanel::SetupButtons()
{
    m_loadZipButton = std::make_unique<wxButton>(this, ID_LOAD_ZIP, "Load Zip File");
    m_extractButton = std::make_unique<wxButton>(this, ID_EXTRACT_SELECTED, "Extract Selected");
    m_extractAllButton = std::make_unique<wxButton>(this, ID_EXTRACT_ALL, "Extract All");
}

void EnhancedUnZipPanel::SetupProgressBar()
{
    m_progressBar = std::make_unique<wxGauge>(this, wxID_ANY, 100);
}

bool EnhancedUnZipPanel::LoadArchiveEntries()
{
    wxFileInputStream input(m_archivePath);
    if (!input.IsOk())
        return false;

    wxZipInputStream zipStream(input);
    std::unique_ptr<wxZipEntry> entry(zipStream.GetNextEntry());
    m_fileList->DeleteAllItems();

    while (entry)
    {
        long index = m_fileList->InsertItem(m_fileList->GetItemCount(), entry->GetName());
        m_fileList->SetItem(index, 1, wxString::Format("%ld", entry->GetSize()));
        entry.reset(zipStream.GetNextEntry());
    }
    return true;
}

void EnhancedUnZipPanel::ExtractAll(const wxString& destPath)
{
    wxFileInputStream input(m_archivePath);
    wxZipInputStream zipStream(input);
    std::unique_ptr<wxZipEntry> entry(zipStream.GetNextEntry());

    wxProgressDialog progress("Extracting All", "Please wait...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    while (entry)
    {
        wxString name = entry->GetName();
        wxFileName destFile(destPath, name);

        if (!wxFileName::DirExists(destFile.GetPath()))
            wxFileName::Mkdir(destFile.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

        if (!entry->IsDir())
        {
            wxFileOutputStream out(destFile.GetFullPath());
            if (out.IsOk())
                zipStream.Read(out);
        }

        entry.reset(zipStream.GetNextEntry());
        progress.Pulse(name);
    }
    m_statusText->SetLabel("Extraction complete");
}

void EnhancedUnZipPanel::ExtractSelected(const wxString& destPath)
{
    wxString selectedPath = wxFileSelector("Choose zip file to extract from", "", "", "zip", "Zip files (*.zip)|*.zip");
    if (selectedPath.IsEmpty())
    {
        m_statusText->SetLabel("No zip file selected");
        return;
    }

    m_archivePath = selectedPath;
    if (!LoadArchiveEntries())
    {
        m_statusText->SetLabel("Failed to load zip file");
        return;
    }

    long item = m_fileList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1)
    {
        m_statusText->SetLabel("No file selected to extract");
        return;
    }

    wxString fileName = m_fileList->GetItemText(item);
    wxFileInputStream input(m_archivePath);
    wxZipInputStream zipStream(input);
    std::unique_ptr<wxZipEntry> entry(zipStream.GetNextEntry());

    while (entry)
    {
        if (entry->GetName() == fileName)
        {
            wxFileName destFile(destPath, fileName);

            if (!wxFileName::DirExists(destFile.GetPath()))
                wxFileName::Mkdir(destFile.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

            wxFileOutputStream out(destFile.GetFullPath());
            if (out.IsOk())
                zipStream.Read(out);

            m_statusText->SetLabel("Extracted: " + fileName);
            break;
        }
        entry.reset(zipStream.GetNextEntry());
    }

    if (wxRemoveFile(m_archivePath))
        m_statusText->SetLabel("Zip file extracted and deleted: " + fileName);
    else
        m_statusText->SetLabel("Zip file extracted but could not be deleted");
}

void EnhancedUnZipPanel::OnLoadZip(wxCommandEvent&)
{
    wxString selectedPath = wxFileSelector("Choose zip file", "", "", "zip", "Zip files (*.zip)|*.zip");
    if (!selectedPath.IsEmpty())
    {
        m_archivePath = selectedPath;
        if (LoadArchiveEntries())
        {
            m_statusText->SetLabel("Loaded: " + m_archivePath);
            EnableControls(true);
        }
        else
        {
            m_statusText->SetLabel("Failed to load zip file");
            EnableControls(false);
        }
    }
    else
    {
        m_statusText->SetLabel("No file selected");
    }
}

void EnhancedUnZipPanel::OnExtractAll(wxCommandEvent&)
{
    wxDirDialog dirDialog(this, "Choose extraction directory");
    if (dirDialog.ShowModal() == wxID_OK)
        ExtractAll(dirDialog.GetPath());
}

void EnhancedUnZipPanel::OnExtractSelected(wxCommandEvent&)
{
    wxDirDialog dirDialog(this, "Choose extraction directory");
    if (dirDialog.ShowModal() == wxID_OK)
        ExtractSelected(dirDialog.GetPath());
}

void EnhancedUnZipPanel::OnItemSelect(wxListEvent& event)
{
    EnableControls(true);
}

void EnhancedUnZipPanel::EnableControls(bool enable)
{
    if (m_extractButton)
        m_extractButton->Enable(enable);
    if (m_extractAllButton)
        m_extractAllButton->Enable(enable);
}
