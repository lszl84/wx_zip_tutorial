#include <wx/wx.h>

#include <wx/notebook.h>
#include "zippanel.h"
#include "unzippanel.h"

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
};

bool MyApp::OnInit()
{
    wxFileSystem::AddHandler(new wxZipFSHandler);

    MyFrame *frame = new MyFrame("Hello World", wxDefaultPosition, wxDefaultSize);
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto tabs = new wxNotebook(this, wxID_ANY);
    tabs->SetInternalBorder(0);
    tabs->AddPage(new ZipPanel(tabs), "Zip");
    tabs->AddPage(new UnZipPanel(tabs), "UnZip");

    sizer->Add(tabs, 1, wxEXPAND | wxALL, FromDIP(10));
    this->SetSizer(sizer);

    this->SetSize(FromDIP(wxSize(700, 600)));
    this->SetMinSize(FromDIP(wxSize(600, 500)));
}
