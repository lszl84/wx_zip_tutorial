#pragma once

#include <wx/dir.h>
#include <wx/filename.h>

struct IgnoringTraverser : public wxDirTraverser
{
    std::function<bool(const std::string &)> shouldIgnoreDir;
    std::function<void(const std::string &)> fileEnterCallback;

    virtual wxDirTraverseResult OnFile(const wxString &filename) override
    {
        fileEnterCallback(filename.ToStdString());
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString &dirname) override
    {
        auto lastPathComponent = wxFileName(dirname).GetFullName();

        return shouldIgnoreDir(lastPathComponent.ToStdString()) ? wxDIR_IGNORE : wxDIR_CONTINUE;
    }
};