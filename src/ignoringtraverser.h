#pragma once

#include <wx/dir.h>
#include <wx/filename.h>

struct IgnoringTraverser : public wxDirTraverser
{
    // std::vector<std::string> files;
    std::function<bool(const std::string &)> shouldIgnoreDir;
    std::function<void(const std::string &)> fileEnterCallback;

    virtual wxDirTraverseResult OnFile(const wxString &filename) override
    {
        // files.push_back(filename.ToStdString());

        fileEnterCallback(filename.ToStdString());
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString &dirname) override
    {
        // This is ok. Ignoring any dir, also if inside
        auto lastPathComponent = wxFileName(dirname).GetFullName();

        return shouldIgnoreDir(lastPathComponent.ToStdString()) ? wxDIR_IGNORE : wxDIR_CONTINUE;
    }
};