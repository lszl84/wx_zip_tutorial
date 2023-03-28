#pragma once

#include <wx/wx.h>

#include <wx/filesys.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <wx/fs_zip.h>

#include <wx/progdlg.h>

#include <memory>

struct UnZipPanel : public wxPanel
{
    UnZipPanel(wxWindow *parent);

    wxBoxSizer *mainSizer;

    wxTextCtrl *zipFileText;
    wxTextCtrl *outputDirText;
    wxTextCtrl *singleFileText;
    wxCheckBox *singleFileCheckBox;

    void SetupUnZipSection();

    void PerformUnZip();
    void UnZipAllFiles();
    void UnZipSingleFile();
};

UnZipPanel::UnZipPanel(wxWindow *parent) : wxPanel(parent)
{
    mainSizer = new wxBoxSizer(wxVERTICAL);

    SetupUnZipSection();

    SetSizer(mainSizer);

    wxFileSystem::AddHandler(new wxZipFSHandler);
}

void UnZipPanel::SetupUnZipSection()
{
    auto unzipTitleLabel = new wxStaticText(this, wxID_ANY, "Unzip File");
    unzipTitleLabel->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    zipFileText = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    zipFileText->SetEditable(false);

    auto browseZipFileButton = new wxButton(this, wxID_ANY, "Browse...");

    auto outputDirLabel = new wxStaticText(this, wxID_ANY, "Output Directory");
    outputDirLabel->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    outputDirText = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    outputDirText->SetEditable(false);

    auto setOutputDirButton = new wxButton(this, wxID_ANY, "Browse...");

    singleFileCheckBox = new wxCheckBox(this, wxID_ANY, "only one file");
    singleFileText = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    singleFileText->Enable(false);

    auto unzipButton = new wxButton(this, wxID_ANY, "Unzip");

    auto unzipSizer = new wxBoxSizer(wxHORIZONTAL);
    unzipSizer->Add(zipFileText, 1, wxEXPAND | wxRIGHT, FromDIP(8));
    unzipSizer->Add(browseZipFileButton, 0, wxEXPAND, FromDIP(8));

    auto outputDirSizer = new wxBoxSizer(wxHORIZONTAL);
    outputDirSizer->Add(outputDirText, 1, wxEXPAND | wxRIGHT, FromDIP(8));
    outputDirSizer->Add(setOutputDirButton, 0, wxEXPAND, FromDIP(8));

    mainSizer->Add(unzipTitleLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    mainSizer->Add(unzipSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    mainSizer->Add(outputDirLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    mainSizer->Add(outputDirSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    mainSizer->AddSpacer(FromDIP(8));

    mainSizer->Add(singleFileCheckBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    mainSizer->Add(singleFileText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    mainSizer->AddSpacer(FromDIP(8));

    mainSizer->Add(unzipButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    browseZipFileButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                              {
                                  wxFileDialog openFileDialog(this, "Open zip file", "", "", "Zip files (*.zip)|*.zip", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

                                  if (openFileDialog.ShowModal() == wxID_CANCEL)
                                  {
                                      return;
                                  }

                                  zipFileText->SetValue(openFileDialog.GetPath()); });

    setOutputDirButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                             {
                                 wxDirDialog dirDialog(this, "Select output directory", "", wxDD_DEFAULT_STYLE | wxDD_NEW_DIR_BUTTON);

                                 if (dirDialog.ShowModal() == wxID_CANCEL)
                                 {
                                     return;
                                 }

                                 outputDirText->SetValue(dirDialog.GetPath()); });

    singleFileCheckBox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent &event)
                             {
                                 singleFileText->Enable(event.IsChecked());

                                 if (event.IsChecked())
                                 {
                                     singleFileText->SetFocus();
                                 }
                             });

    unzipButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event)
                      { PerformUnZip(); });
}

void UnZipPanel::PerformUnZip()
{
    if (zipFileText->GetValue().IsEmpty() || outputDirText->GetValue().IsEmpty())
    {
        wxMessageBox("Please fill in all the fields.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (singleFileCheckBox->IsChecked())
    {
        UnZipSingleFile();
    }
    else
    {
        UnZipAllFiles();
    }
}

void UnZipPanel::UnZipAllFiles()
{
    wxProgressDialog progressDialog("Unzipping", "Unzipping file...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    wxFileInputStream inStream(zipFileText->GetValue());
    wxZipInputStream zipIn(inStream);
    std::unique_ptr<wxZipEntry> entry(zipIn.GetNextEntry());

    while (entry)
    {
        progressDialog.Pulse();

        wxString entryName = entry->GetName();

        wxFileName destFileName = outputDirText->GetValue() + wxFileName::GetPathSeparator() + entryName;
        bool isFile = !entry->IsDir();

        if (!wxDirExists(destFileName.GetPath()))
        {
            wxFileName::Mkdir(destFileName.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        }

        if (isFile)
        {
            if (!zipIn.CanRead())
            {
                wxLogError("Couldn't read the zip entry '%s'.", entry->GetName());
                return;
            }

            wxFileOutputStream outStream(destFileName.GetFullPath());

            if (!outStream.IsOk())
            {
                wxLogError("Couldn't create the file '%s'.", destFileName.GetFullPath());
                return;
            }

            zipIn.Read(outStream);

            zipIn.CloseEntry();
            outStream.Close();
        }

        entry.reset(zipIn.GetNextEntry());
    }
}

void UnZipPanel::UnZipSingleFile()
{
    if (singleFileText->GetValue().IsEmpty())
    {
        wxMessageBox("Please enter a file name to extract.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxFileSystem fs;
    std::unique_ptr<wxFSFile> zip(fs.OpenFile(zipFileText->GetValue() + "#zip:" + singleFileText->GetValue()));

    if (zip)
    {
        wxInputStream *in = zip->GetStream();

        if (in)
        {
            wxFileName destFileName = outputDirText->GetValue() + wxFileName::GetPathSeparator() + singleFileText->GetValue();
            if (!wxDirExists(destFileName.GetPath()))
            {
                wxFileName::Mkdir(destFileName.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
            }

            wxFileOutputStream out(destFileName.GetFullPath());
            out.Write(*in);
            out.Close();
        }
    }
    else
    {
        wxMessageBox("File not found in zip.", "Error", wxOK | wxICON_ERROR);
    }
}
