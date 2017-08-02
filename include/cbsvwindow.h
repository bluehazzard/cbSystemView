#ifndef CBSVWINDOW_H
#define CBSVWINDOW_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


#include <wx/process.h>
#include <wx/aui/aui.h>
#include <sdk.h>
#include <wx/propgrid/manager.h>
#include "cbsvdfilereader.h"

class cbSVWindow : public wxPanel
{
    public:
        cbSVWindow(wxWindow* parent);
        virtual ~cbSVWindow();

        void PopulateGrid();

    protected:

    private:

        void OnSearchCtrl(wxCommandEvent& event);
        void OnModifyTree(wxCommandEvent& event);

        static const long ID_SEARCH_CTRL;
        static const long ID_BTN_EXPAND_TREE;
        static const long ID_BTN_COLLAPSE_TREE;

        wxPropertyGridManager*  m_pg_man;
        wxPropertyGridPage*     m_pg_first_page;
        wxTextCtrl*             m_SearchCtrl;

        cbSVDFileReader* m_reader;
        SVDDevice* m_device;

        DECLARE_EVENT_TABLE();
};

#endif // CBSVWINDOW_H
