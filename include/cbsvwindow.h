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
#include "cbsvpgproperties.h"

struct RegisterWatch
{
    wxPGProperty*           m_property;
    cb::shared_ptr<cbWatch> m_watch;
};

class cbSVWindow : public wxPanel
{
    public:
        cbSVWindow(wxWindow* parent);
        virtual ~cbSVWindow();

        void PopulateGrid();

        void UpdateWatches();
        void OnDebuggerStarted();
        void SetSVDFile(const wxString& file);

        void DeleteAllWatches();

        std::list<RegisterWatch>::iterator FindWatchFromProperty(wxPGProperty* prop);
        //bool FindWatchFromProperty(wxPGProperty* prop);
        void GenerateWatchesRecursive(wxPGProperty* prop, cbDebuggerPlugin *dbg);


    protected:

    private:

        void OnSearchCtrl(wxCommandEvent& event);
        void OnModifyTree(wxCommandEvent& event);
        void OnSearchTimer(wxTimerEvent& event);

        void OnItemExpand(wxPropertyGridEvent& event);
        void OnItemCollapsed(wxPropertyGridEvent& event);
        void OnRightClick(wxPropertyGridEvent &evt);

        void OnContextMenu(wxCommandEvent& evt);

        static const long ID_SEARCH_CTRL;
        static const long ID_BTN_EXPAND_TREE;
        static const long ID_BTN_COLLAPSE_TREE;
        static const long ID_SEARCH_TIMER;
        static const long ID_PROP_GRID ;
        static const long ID_MENU_VIEW_HEX;
        static const long ID_MENU_VIEW_BIN;
        static const long ID_MENU_VIEW_DEC;
        static const long ID_MENU_VIEW_UDEC;
        static const long ID_MENU_VIEW_FLOAT;
        static const long ID_MENU_VIEW_CHAR;


        wxPropertyGridManager*  m_pg_man;
        wxPropertyGridPage*     m_pg_first_page;
        wxTextCtrl*             m_SearchCtrl;

        wxString                m_CurSVDFile;

        wxTimer*                m_searchTimer;

        svPGPropBase*           m_curSelProp;

        cbSVDFileReader* m_reader;
        SVDDevice* m_device;

        std::list<RegisterWatch> m_RegisterWatches;

        DECLARE_EVENT_TABLE();
};

#endif // CBSVWINDOW_H
