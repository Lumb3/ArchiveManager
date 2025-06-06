#include <wx/wx.h>
#include <wx/notebook.h>
#include "EnhancedZipPanel.h"
#include "EnhancedUnZipPanel.h"

class ArchiveApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MainFrame : public wxFrame
{
public:
    MainFrame();
    ~MainFrame();  // Added destructor

private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    wxNotebook* m_notebook;
    EnhancedZipPanel* m_zipPanel;
    EnhancedUnZipPanel* m_unzipPanel;

    DECLARE_EVENT_TABLE()
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(ArchiveApp);

bool ArchiveApp::OnInit()
{
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    return true;
}

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Enhanced Archive Manager",
              wxDefaultPosition, wxSize(800, 600))
{
    // Create menu bar
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit the application");

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Ready");

    // Create notebook for tabs
    m_notebook = new wxNotebook(this, wxID_ANY);

    // Create zip panel
    m_zipPanel = new EnhancedZipPanel(m_notebook);
    m_notebook->AddPage(m_zipPanel, "Create Archive", true);

    // Create unzip panel
    m_unzipPanel = new EnhancedUnZipPanel(m_notebook);
    m_notebook->AddPage(m_unzipPanel, "Extract Archive", false);

    // Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 5);
    SetSizer(mainSizer);

    Centre();
}

MainFrame::~MainFrame()
{
    // wxWidgets will handle cleanup of child windows
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Enhanced Archive Manager v1.0\n\n"
                 "A powerful file archiving application with optimized performance.\n"
                 "Features ZIP/UNZIP operations with path optimization.",
                 "About Enhanced Archive Manager",
                 wxOK | wxICON_INFORMATION);
}