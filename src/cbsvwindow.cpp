#include "cbsvwindow.h"

#include <wx/progdlg.h>
#include <wx/filesys.h>

#include <debuggermanager.h>
#include <logmanager.h>
#include <cbplugin.h>

const long cbSVWindow::ID_SEARCH_CTRL       = wxNewId();
const long cbSVWindow::ID_ANI_CTRL          = wxNewId();
const long cbSVWindow::ID_BTN_EXPAND_TREE   = wxNewId();
const long cbSVWindow::ID_BTN_COLLAPSE_TREE = wxNewId();
const long cbSVWindow::ID_SEARCH_TIMER      = wxNewId();
const long cbSVWindow::ID_PROP_GRID         = wxNewId();

const long cbSVWindow::ID_MENU_VIEW_HEX     = wxNewId();
const long cbSVWindow::ID_MENU_VIEW_BIN     = wxNewId();
const long cbSVWindow::ID_MENU_VIEW_DEC     = wxNewId();
const long cbSVWindow::ID_MENU_VIEW_UDEC    = wxNewId();
const long cbSVWindow::ID_MENU_VIEW_FLOAT   = wxNewId();
const long cbSVWindow::ID_MENU_VIEW_CHAR    = wxNewId();

const long cbSVWindow::ID_MENU_COLLAPSE    = wxNewId();

BEGIN_EVENT_TABLE(cbSVWindow,wxPanel)
	//(*EventTable(CPURegistersDlg)
	//*)
	EVT_TEXT( ID_SEARCH_CTRL, cbSVWindow::OnSearchCtrl )

	EVT_TOOL( ID_BTN_EXPAND_TREE,   cbSVWindow::OnModifyTree )
	EVT_TOOL( ID_BTN_COLLAPSE_TREE, cbSVWindow::OnModifyTree )

	EVT_PG_ITEM_EXPANDED(ID_PROP_GRID, cbSVWindow::OnItemExpand)
	EVT_PG_ITEM_COLLAPSED(ID_PROP_GRID, cbSVWindow::OnItemCollapsed)
	EVT_PG_RIGHT_CLICK(ID_PROP_GRID, cbSVWindow::OnRightClick)

	EVT_PG_CHANGED(ID_PROP_GRID, cbSVWindow::OnItemChanged)

	EVT_TIMER( ID_SEARCH_TIMER, cbSVWindow::OnSearchTimer)

	EVT_MENU(ID_MENU_VIEW_HEX  , cbSVWindow::OnContextMenu )
	EVT_MENU(ID_MENU_VIEW_BIN  , cbSVWindow::OnContextMenu )
	EVT_MENU(ID_MENU_VIEW_DEC  , cbSVWindow::OnContextMenu )
	EVT_MENU(ID_MENU_VIEW_UDEC , cbSVWindow::OnContextMenu )
	EVT_MENU(ID_MENU_VIEW_FLOAT, cbSVWindow::OnContextMenu )
	EVT_MENU(ID_MENU_VIEW_CHAR , cbSVWindow::OnContextMenu )

	EVT_MENU(ID_MENU_COLLAPSE,  cbSVWindow::OnContextMenu )

   // EVT_PG_CHANGED(ID_CUSTOM1, CPURegistersDlg::OnPropertyChanged)
   // EVT_PG_CHANGING(ID_CUSTOM1, CPURegistersDlg::OnPropertyChanging)
END_EVENT_TABLE()

struct bad_file_execp
{
    bad_file_execp(wxString msg) : m_msg(msg)    {};
    wxString m_msg;
};

wxBitmap LoadPNGFromResourceFile(wxString name)
{
	wxFileSystem filesystem;
	wxString filename =  wxT("file:") + ConfigManager::GetDataFolder() + wxT("/cbSystemView.zip#zip:/") + name;
	wxFSFile *file = filesystem.OpenFile( filename, wxFS_READ | wxFS_SEEKABLE );
	if(file == nullptr)
        throw bad_file_execp(_("File not found:") + filename);
	wxImage img;
	img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_PNG);
	wxBitmap ret(img);
	if(!ret.IsOk())
        throw bad_file_execp(_("File not loaded correctly:") + filename);

	return ret;
}

wxAnimation LoadGifFromResourceFile(wxString name)
{
	wxFileSystem filesystem;
	wxString filename =  wxT("file:") + ConfigManager::GetDataFolder() + wxT("/cbSystemView.zip#zip:/") + name;
	wxFSFile *file = filesystem.OpenFile( filename, wxFS_READ | wxFS_SEEKABLE);
	if(file == nullptr)
        throw bad_file_execp(_("File not found:") + filename);

	wxAnimation ret;
	if( ret.Load(*file->GetStream(), wxANIMATION_TYPE_GIF) == false)
        throw bad_file_execp(_("File not loaded correctly:") + filename);
	if(!ret.IsOk())
        throw bad_file_execp(_("File not loaded correctly:") + filename);

	return ret;
}

cbSVWindow::cbSVWindow(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL/* | wxCLIP_CHILDREN*/)
{
    //ctor
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_pg_man = new wxPropertyGridManager(this, ID_PROP_GRID, wxDefaultPosition, wxDefaultSize, wxPG_DESCRIPTION | wxPG_TOOLBAR);
    #if wxCHECK_VERSION(3, 0, 0)
        m_pg_first_page = m_pg_man->AddPage(wxT("First Page"));
    #else
        int page = m_pg_man->AddPage(wxT("First Page"));
        m_pg_first_page = m_pg_man->GetPage(page);
    #endif
    bs->Add(m_pg_man, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);



    wxToolBar* toolbar = m_pg_man->GetToolBar();
    toolbar->SetToolBitmapSize(wxSize(16,16));

    wxBitmap okImg;

    try
	{
    toolbar->AddTool(ID_BTN_EXPAND_TREE,    _("Expand all nodes"),   LoadPNGFromResourceFile(wxT("images/expand_16x16.png")) );
    toolbar->AddTool(ID_BTN_COLLAPSE_TREE,  _("Collapse all nodes"), LoadPNGFromResourceFile(wxT("images/collapse_16x16.png")) );


	wxSize ToolSize = toolbar->GetToolSize();
	unsigned int search_control_pos = ToolSize.GetWidth() * toolbar->GetToolsCount() + 10;
	m_SearchCtrl = new wxTextCtrl(m_pg_man->GetToolBar(), ID_SEARCH_CTRL, _(""), wxPoint(search_control_pos, 0), wxSize(150, ToolSize.GetHeight()) );


    search_control_pos = m_SearchCtrl->GetPosition().x + m_SearchCtrl->GetSize().GetWidth() + 10;
    m_anictrl = new wxAnimationCtrl(m_pg_man->GetToolBar(), ID_ANI_CTRL, wxNullAnimation, wxPoint(search_control_pos,0), wxSize(ToolSize.GetWidth(), ToolSize.GetHeight()), wxAC_NO_AUTORESIZE );

    okImg = LoadPNGFromResourceFile(wxT("images/ok_16x16.png"));

    m_anictrl->SetAnimation(LoadGifFromResourceFile(wxT("images/throbber_16x16.gif")));

    } catch (bad_file_execp e)
	{
        Manager::Get()->GetLogManager()->LogError(_("cbSystemView: ") + e.m_msg);
	}


	toolbar->AddControl(m_SearchCtrl);
	toolbar->AddControl(m_anictrl);

    m_anictrl->SetInactiveBitmap(okImg);


    toolbar->Realize();
    toolbar->SetInitialSize();

    m_reader = new cbSVDFileReader();
    m_device = new SVDDevice();

    m_CurSVDFile = wxEmptyString;

    m_pg_first_page->SetColumnCount(4);
    m_pg_first_page->Append( new wxPropertyCategory(_T("Category per") ));

    m_searchTimer = new wxTimer(this, ID_SEARCH_TIMER);

    PopulateGrid();

    m_pg_first_page->GetGrid()->CenterSplitter();
}

cbSVWindow::~cbSVWindow()
{
    delete m_reader;
    delete m_device;
    delete m_searchTimer;
}

void cbSVWindow::DeleteAllWatches()
{
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end() ;)
    {
        dbg->DeleteWatch(itr->m_watch);
        m_RegisterWatches.erase(itr++);
    }
}

void cbSVWindow::DeleteWatch(wxPGProperty* prop)
{
    std::list<RegisterWatch>::iterator watchItr = FindWatchFromProperty(prop);
    if(watchItr != m_RegisterWatches.end() )
    {
        cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
        dbg->DeleteWatch(watchItr->m_watch);
        m_RegisterWatches.erase(watchItr);
    }
}

void cbSVWindow::SetSVDFile(const wxString& file)
{
    if(m_CurSVDFile == file)
        return;

    DeleteAllWatches();
    delete m_reader;
    delete m_device;

    m_reader = new cbSVDFileReader();
    m_device = new SVDDevice();

    m_reader->LoadSVDFile(file, m_device);
    PopulateGrid();
    m_CurSVDFile = file;
}

void cbSVWindow::OnModifyTree(wxCommandEvent& event)
{
    if(event.GetId() == ID_BTN_EXPAND_TREE)
    {
        // TODO (bluehazzard#1#): Clean UP
        m_pg_first_page->ExpandAll();
        wxPGProperty* root = m_pg_first_page->GetRoot();
        size_t child_count = root->GetChildCount();
        cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

        for (size_t i = 0 ; i < child_count; i++)
        {
            GenerateWatchesRecursive(root->Item(i), dbg);
        }
    }
    else if(event.GetId() == ID_BTN_COLLAPSE_TREE)
    {
        m_pg_first_page->ExpandAll(false);
        DeleteAllWatches();
    }

}

void cbSVWindow::PopulateGrid()
{
    m_pg_first_page->Clear();

    auto itr_per = m_device->GetPeriperyBegin();
    for(;itr_per != m_device->GetPeriperyEnd(); ++itr_per)
    {
        svPGPeripheryProp* prop = new svPGPeripheryProp(*(*itr_per).get());
        m_pg_first_page->Append(prop);
        prop->Populate();
    }
}

void cbSVWindow::UpdateWatches()
{
    size_t wcount =  m_RegisterWatches.size();
    wxProgressDialog *pd = nullptr;

    UpdateWorkingStat(WORKING_STAT_UPDATING);

    if(wcount > 10)  // Arbitrary number
        pd = new  wxProgressDialog(wxT("Update svd view"), wxT("update"),wcount);

    size_t i = 0;
    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end() ; ++itr)
    {
        wxString val;
        (*itr).m_watch->GetValue(val);
        dynamic_cast<svPGPropBase*>((*itr).m_property)->SetDataFromBinary(val);
        if(pd != nullptr)
        {
            pd->Update(i);
            ++i;
        }
    }

    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    itr = m_TempRegisterWatches.begin();
    for(; itr != m_TempRegisterWatches.end() ;)
    {
        dbg->DeleteWatch(itr->m_watch);
        m_TempRegisterWatches.erase(itr++);
    }

    if(pd != nullptr)
        delete pd;

    UpdateWorkingStat(WORKING_STAT_UPDATING, false);
}

void cbSVWindow::GenerateWatchesRecursive(wxPGProperty* prop, cbDebuggerPlugin *dbg)
{
    UpdateWorkingStat(WORKING_STAT_UPDATING);

    size_t child_count = prop->GetChildCount();
    bool isPhery = prop->IsKindOf(CLASSINFO(svPGPeripheryProp) );
    if (isPhery )
    {
        std::list<RegisterWatch>::iterator watch_itr = FindWatchFromProperty(prop);
        if(prop->IsExpanded() && watch_itr == m_RegisterWatches.end() )
        {
            RegisterWatch watch;
            svPGPeripheryProp* per = dynamic_cast<svPGPeripheryProp*>(prop);
            if(per != nullptr)
            {
                watch.m_property = prop;
                watch.m_watch    = dbg->AddMemoryRange( per->GetAddress() , per->GetSize(), wxEmptyString );

                m_RegisterWatches.push_back(watch);
            }
            else
            {
                Manager::Get()->GetLogManager()->LogError(_T("cbSVWindow::GenerateWatchesRecursive: if(per == nullptr)"));
            }
        }
        else if(prop->IsExpanded() == false && watch_itr != m_RegisterWatches.end())
        {
            m_RegisterWatches.erase(watch_itr);
        }

    }

    for (size_t i = 0; i < child_count; ++i)
    {
        GenerateWatchesRecursive(prop->Item(i), dbg);
    }
}

std::list<RegisterWatch>::iterator  cbSVWindow::FindWatchFromProperty(wxPGProperty* prop )
{
    if(prop == nullptr)
        return m_RegisterWatches.end();

    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end(); ++itr)
    {
        if(itr->m_property == prop)
        {
            break;
        }
    }
    return itr;
}

void cbSVWindow::OnDebuggerStarted()
{
    wxPGProperty* root = m_pg_first_page->GetRoot();
    size_t child_count = root->GetChildCount();
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

    for (size_t i = 0 ; i < child_count; i++)
    {
        GenerateWatchesRecursive(root->Item(i), dbg);
    }
}

void cbSVWindow::OnItemExpand(wxPropertyGridEvent &evt)
{
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    wxPGProperty* prop = evt.GetProperty();;

    bool isPeriphery = prop->IsKindOf(CLASSINFO(svPGPeripheryProp) );
    if(dbg->IsRunning() && isPeriphery)
    {
        UpdateWorkingStat(WORKING_STAT_UPDATING);
        svPGPropBase* regBase = dynamic_cast<svPGPropBase*>(prop);
        if(regBase == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_T("cbSVWindow::GenerateWatchesRecursive: if(regBase == nullptr)"));
            return;
        }

        RegisterWatch watch;
        watch.m_property = prop;
        watch.m_watch    = dbg->AddMemoryRange( regBase->GetAddress() , regBase->GetSize(), wxEmptyString );

        m_RegisterWatches.push_back(watch);
    }

}

void cbSVWindow::OnItemCollapsed(wxPropertyGridEvent &evt)
{
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end() ; ++itr)
    {
        if ( itr->m_property == evt.GetProperty() )
        {
            dbg->DeleteWatch(itr->m_watch);
            m_RegisterWatches.erase(itr);
            return;
        }
    }
}

void cbSVWindow::OnItemChanged(wxPropertyGridEvent &evt)
{
    svPGPropBase *prop = dynamic_cast<svPGPropBase*>(evt.GetProperty());
   /* if (prop->IsKindOf(CLASSINFO(svPGPeripheryProp) ))
    {
        // we don't change register so do nothing
        prop = dynamic_cast<svPGPeripheryProp*>(prop)->GetRegisterChanged();
    }
    // Try to get the register

    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    RegisterWatch watch;
    watch.m_property = evt.GetProperty();
    watch.m_watch    = dbg->AddMemoryRange( prop->GetAddress() , prop->GetSize(), wxEmptyString );

    uint64_t da = prop->GetData();
    wxString data;
    data.From8BitData((char*) &da, sizeof(uint64_t));

    dbg->SetWatchValue(watch.m_watch, data);
    m_TempRegisterWatches.push_back(watch);*/
}

void cbSVWindow::OnRightClick(wxPropertyGridEvent &evt)
{
    svPGPropBase *prop = dynamic_cast<svPGPropBase*>(evt.GetProperty());
    m_curSelProp = evt.GetProperty();
    wxMenu m;
    wxMenu* sub_view = nullptr;
    if (prop)
    {
        sub_view = new wxMenu();
        if(prop->CanRepresent(svPGPropBase::REP_HEX))
            sub_view->AppendRadioItem(ID_MENU_VIEW_HEX, _("Hex"));
        if(prop->CanRepresent(svPGPropBase::REP_DEC))
            sub_view->AppendRadioItem(ID_MENU_VIEW_DEC, _("Dec"));
        if(prop->CanRepresent(svPGPropBase::REP_UDEC))
            sub_view->AppendRadioItem(ID_MENU_VIEW_UDEC, _("unsigned dec"));
        if(prop->CanRepresent(svPGPropBase::REP_BIN))
            sub_view->AppendRadioItem(ID_MENU_VIEW_BIN, _("bin"));
        if(prop->CanRepresent(svPGPropBase::REP_CHAR))
            sub_view->AppendRadioItem(ID_MENU_VIEW_CHAR, _("char"));
        if(prop->CanRepresent(svPGPropBase::REP_FLOAT))
            sub_view->AppendRadioItem(ID_MENU_VIEW_FLOAT, _("float"));

        if(prop->GetRepresentation() == svPGPropBase::REP_HEX)
            sub_view->Check(ID_MENU_VIEW_HEX, true);
        if(prop->GetRepresentation() == svPGPropBase::REP_DEC)
            sub_view->Check(ID_MENU_VIEW_DEC, true);
        if(prop->GetRepresentation() == svPGPropBase::REP_UDEC)
            sub_view->Check(ID_MENU_VIEW_UDEC, true);
        if(prop->GetRepresentation() == svPGPropBase::REP_BIN)
            sub_view->Check(ID_MENU_VIEW_BIN, true);
        if(prop->GetRepresentation() == svPGPropBase::REP_CHAR)
            sub_view->Check(ID_MENU_VIEW_CHAR, true);
        if(prop->GetRepresentation() == svPGPropBase::REP_FLOAT)
            sub_view->Check(ID_MENU_VIEW_FLOAT, true);

    }

    m.Append(ID_MENU_COLLAPSE, _("Collapse parent"));
    if(sub_view != nullptr )
        m.AppendSubMenu(sub_view, _("View"));

    PopupMenu(&m);
}

void cbSVWindow::OnContextMenu(wxCommandEvent& evt)
{
    long id = evt.GetId();
    svPGPropBase *propBase = dynamic_cast<svPGPropBase*>(m_curSelProp);

    if (id == ID_MENU_COLLAPSE)
    {
        wxPGProperty* parent = 	m_curSelProp->GetParent();
        if(parent)
        {
            parent->SetExpanded(false);
            DeleteWatch(parent);
            m_pg_first_page->RefreshGrid();
            m_pg_first_page->GetGrid()->EnsureVisible(parent);
        }

        return;
    }

    if(propBase == nullptr)
        return;

	if ( id == ID_MENU_VIEW_HEX)
	    propBase->SetRepresentation(svPGPropBase::REP_HEX);

	else if ( id == ID_MENU_VIEW_BIN)
        propBase->SetRepresentation(svPGPropBase::REP_BIN);

	else if ( id == ID_MENU_VIEW_DEC)
        propBase->SetRepresentation(svPGPropBase::REP_DEC);

	else if ( id == ID_MENU_VIEW_UDEC)
        propBase->SetRepresentation(svPGPropBase::REP_UDEC);

	else if ( id == ID_MENU_VIEW_FLOAT)
        propBase->SetRepresentation(svPGPropBase::REP_FLOAT);

	else if ( id == ID_MENU_VIEW_CHAR)
        propBase->SetRepresentation(svPGPropBase::REP_CHAR);

}

bool FindString(const wxString& a, const wxString& b)
{
    for(size_t i = 0 ; i < a.length();i++)
    {
        size_t k = 0;
        for(k = 0; k < b.length() && i + k < a.length();k++)
        {
            if(a[i+k] != b[k]) break;
        }
        if(k == b.length())
            return true;
    }
    return false;
}

bool FindStringRecursive( wxPGProperty* prop, const wxString& str)
{
    bool child_found = false;
    size_t child_count = prop->GetChildCount();
    wxString label = prop->GetLabel();
    if(FindString(label.Upper(), str.Upper()) == true)
    {
        prop->Hide(false);
        return true;
    }

    for (size_t i = 0; i < child_count; ++i)
    {
        if(FindStringRecursive(prop->Item(i), str ) == true )
             child_found = true;
    }

    if(child_found == false)
        prop->Hide(true);

    return child_found;
}

void cbSVWindow::OnSearchTimer(wxTimerEvent& event)
{
    wxPGProperty* root = m_pg_first_page->GetRoot();
    size_t child_count = root->GetChildCount();
    wxString searchStr = m_SearchCtrl->GetValue();

    if(searchStr == wxEmptyString)
        return;

    UpdateWorkingStat(WORKING_STAT_SEARCHING);

    wxProgressDialog pd(wxT("search"),wxT("execute search"),child_count * 2);
    int update_state = 0;

    for (size_t i = 0 ; i < child_count; i++)
    {
        FindStringRecursive(root->Item(i),searchStr);
        pd.Update(update_state++, wxT("searching"));
    }

    for (size_t i = 0 ; i < child_count; i++)
    {
        root->Item(i)->SetExpanded(true);
        pd.Update(update_state++, wxT("expanding"));
    }

    m_pg_first_page->RefreshGrid();
    UpdateWorkingStat(WORKING_STAT_SEARCHING, false);
}

void cbSVWindow::OnSearchCtrl(wxCommandEvent& event)
{
    wxPGProperty* root = m_pg_first_page->GetRoot();
    size_t child_count = root->GetChildCount();

    wxString searchStr = event.GetString();

    if(searchStr == wxEmptyString)
    {
        for (size_t i = 0 ; i < child_count; i++)
           root->Item(i)->Hide( false );

        m_searchTimer->Stop();
        return;
    }
    else
    {
        m_searchTimer->Start(500, wxTIMER_ONE_SHOT);
    }

}
