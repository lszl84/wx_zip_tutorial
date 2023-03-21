#pragma once

#include <wx/wx.h>

#include <wx/filesys.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>

#include <wx/listctrl.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>

#include "ignoringtraverser.h"

struct ZipPanel : public wxPanel
{
    ZipPanel(wxWindow *parent);

    wxBoxSizer *mainSizer;

    wxTextCtrl *dirToCompressText;
    wxListView *filesList;
    wxListView *ignoredList;

    wxTextCtrl *zipFileText;

    void SetupDirectoryLoadSection();
    void SetupFileListSection();
    void SetupCompressSection();

    void LoadFilesToCompress();
    void PerformCompression();
};

ZipPanel::ZipPanel(wxWindow *parent) : wxPanel(parent)
{
    mainSizer = new wxBoxSizer(wxVERTICAL);

    SetupDirectoryLoadSection();
    SetupFileListSection();
    SetupCompressSection();

    SetSizer(mainSizer);
}

void ZipPanel::SetupDirectoryLoadSection()
{
    auto zipTitleLabel = new wxStaticText(this, wxID_ANY, "Zip Directory");
    zipTitleLabel->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    dirToCompressText = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    dirToCompressText->SetEditable(false);
    auto dirButton = new wxButton(this, wxID_ANY, "Browse...");

    auto dirSectionSizer = new wxBoxSizer(wxHORIZONTAL);
    dirSectionSizer->Add(dirToCompressText, 1, wxEXPAND | wxLEFT | wxBOTTOM, FromDIP(8));
    dirSectionSizer->Add(dirButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    mainSizer->AddSpacer(FromDIP(8));
    mainSizer->Add(zipTitleLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    mainSizer->Add(dirSectionSizer, 0, wxEXPAND);

    dirButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                    {
                        wxDirDialog dialog(this, "Choose a directory", wxGetCwd(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                        if (dialog.ShowModal() == wxID_OK)
                        {
                            dirToCompressText->SetValue(dialog.GetPath());
                            
                            LoadFilesToCompress();
                        } });
}

void ZipPanel::SetupFileListSection()
{
    auto filesLabel = new wxStaticText(this, wxID_ANY, "Files to compress");
    filesList = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);

    filesList->AppendColumn("File");

    auto ignoredLabel = new wxStaticText(this, wxID_ANY, "Ignored directories: ");
    ignoredList = new wxListView(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(100, wxDefaultCoord)),
                                 wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);

    ignoredList->AppendColumn("Ignored");

    auto ignoredAddButton = new wxButton(this, wxID_ANY, "Add...");
    auto ignoredRemoveButton = new wxButton(this, wxID_ANY, "Remove");

    auto sizer = new wxGridBagSizer(FromDIP(8), FromDIP(8));

    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(1);

    sizer->Add(filesLabel, {0, 0}, {1, 1}, wxEXPAND);
    sizer->Add(filesList, {1, 0}, {1, 1}, wxEXPAND);

    sizer->Add(ignoredLabel, {0, 1}, {1, 2}, wxEXPAND);
    sizer->Add(ignoredList, {1, 1}, {1, 2}, wxEXPAND);
    sizer->Add(ignoredAddButton, {2, 1}, {1, 1}, wxEXPAND);
    sizer->Add(ignoredRemoveButton, {2, 2}, {1, 1}, wxEXPAND);

    mainSizer->Add(sizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    auto headerResize = [this](wxSizeEvent &event)
    {
        wxListView *list = dynamic_cast<wxListView *>(event.GetEventObject());
        list->SetColumnWidth(0, list->GetClientSize().GetWidth());

        event.Skip();
    };

    filesList->Bind(wxEVT_SIZE, headerResize);
    ignoredList->Bind(wxEVT_SIZE, headerResize);

    ignoredAddButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                           {
        wxTextEntryDialog dialog(this, "Enter the directory name to ignore", "Add ignored directory");
        if (dialog.ShowModal() == wxID_OK)
        {
            ignoredList->InsertItem(ignoredList->GetItemCount(), dialog.GetValue());

            LoadFilesToCompress();
        } });

    ignoredRemoveButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                              {
        long item = ignoredList->GetFirstSelected();
        if (item >= 0)
        {
            ignoredList->DeleteItem(item);

            LoadFilesToCompress();
        } });

    ignoredRemoveButton->Disable();

    ignoredList->Bind(wxEVT_LIST_ITEM_SELECTED, [this, ignoredRemoveButton](wxListEvent &event)
                      { ignoredRemoveButton->Enable(); });

    ignoredList->Bind(wxEVT_LIST_ITEM_DESELECTED, [this, ignoredRemoveButton](wxListEvent &event)
                      { ignoredRemoveButton->Disable(); });
}

void ZipPanel::SetupCompressSection()
{
    auto outFileLabel = new wxStaticText(this, wxID_ANY, "Output file");
    outFileLabel->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    zipFileText = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    zipFileText->SetEditable(false);

    auto outFileButton = new wxButton(this, wxID_ANY, "Change...");

    auto startButton = new wxButton(this, wxID_ANY, "Start!");

    auto outFileSizer = new wxBoxSizer(wxHORIZONTAL);
    outFileSizer->Add(zipFileText, 1, wxEXPAND | wxLEFT | wxBOTTOM, FromDIP(8));
    outFileSizer->Add(outFileButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    mainSizer->Add(outFileLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    mainSizer->Add(outFileSizer, 0, wxEXPAND);
    mainSizer->Add(startButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    outFileButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                        {
                            wxFileDialog dialog(this, "Choose a file", wxGetCwd(), "out.zip", "Zip files (*.zip)|*.zip", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                            if (dialog.ShowModal() == wxID_OK)
                            {
                                zipFileText->SetValue(dialog.GetPath());
                            } });

    startButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                      {
                          if (filesList->GetItemCount() == 0)
                          {
                              wxMessageBox("No files to compress", "Error", wxOK | wxICON_ERROR);
                          }
                          else if (zipFileText->GetValue().empty())
                          {
                              wxMessageBox("No output file specified", "Error", wxOK | wxICON_ERROR);
                          }
                          else
                          {
                              this->PerformCompression();
                          } });
}

void ZipPanel::LoadFilesToCompress()
{
    static constexpr int PulseInterval = 100;

    if (!wxDirExists(dirToCompressText->GetValue()))
    {
        return;
    }

    wxProgressDialog dialog("Loading files", "Loading files to compress...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    filesList->DeleteAllItems();

    IgnoringTraverser traverser;
    traverser.shouldIgnoreDir = [this](const std::string &dirName)
    {
        return this->ignoredList->FindItem(-1, dirName) >= 0;
    };

    traverser.fileEnterCallback = [this, &dialog](const std::string &fileName)
    {
        auto itemCount = filesList->GetItemCount();

        if (itemCount % PulseInterval == 0)
        {
            dialog.Pulse();
        }

        filesList->InsertItem(itemCount, fileName);
    };

    auto dirToCompress = dirToCompressText->GetValue();

    wxDir(dirToCompress).Traverse(traverser);
}

void ZipPanel::PerformCompression()
{
    auto directoryToCompress = dirToCompressText->GetValue();
    auto zipFile = zipFileText->GetValue();

    auto outStream = wxFileOutputStream(zipFile);

    if (!outStream.IsOk())
    {
        wxMessageBox("Failed to open zip file", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxZipOutputStream zip(outStream);

    wxProgressDialog dialog("Compressing", "Compressing files...", filesList->GetItemCount(), this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    for (int i = 0; i < filesList->GetItemCount(); i++)
    {
        auto file = filesList->GetItemText(i);

        wxFileName fileName(file);
        fileName.MakeRelativeTo(directoryToCompress);

        auto relativePath = fileName.GetFullPath(wxPATH_NATIVE);

        wxLogDebug("Compressing %s as %s", file, relativePath);

        zip.PutNextEntry(relativePath);
        wxFFileInputStream(file).Read(zip);

        zip.CloseEntry();

        dialog.Update(i);
    }

    zip.Close();
    outStream.Close();
}
