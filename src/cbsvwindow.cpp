#include "cbsvwindow.h"

#include <wx/progdlg.h>
#include <debuggermanager.h>

const long cbSVWindow::ID_SEARCH_CTRL       = wxNewId();
const long cbSVWindow::ID_BTN_EXPAND_TREE   = wxNewId();
const long cbSVWindow::ID_BTN_COLLAPSE_TREE = wxNewId();
const long cbSVWindow::ID_SEARCH_TIMER      = wxNewId();
const long cbSVWindow::ID_PROP_GRID         = wxNewId();

BEGIN_EVENT_TABLE(cbSVWindow,wxPanel)
	//(*EventTable(CPURegistersDlg)
	//*)
	EVT_TEXT( ID_SEARCH_CTRL, cbSVWindow::OnSearchCtrl )

	EVT_TOOL( ID_BTN_EXPAND_TREE,   cbSVWindow::OnModifyTree )
	EVT_TOOL( ID_BTN_COLLAPSE_TREE, cbSVWindow::OnModifyTree )

	EVT_PG_ITEM_EXPANDED(ID_PROP_GRID, cbSVWindow::OnItemExpand)
	EVT_PG_ITEM_COLLAPSED(ID_PROP_GRID, cbSVWindow::OnItemCollapsed)

	EVT_TIMER( ID_SEARCH_TIMER, cbSVWindow::OnSearchTimer)

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
	wxFSFile *file = filesystem.OpenFile( filename);
	if(file == nullptr)
        throw bad_file_execp(_("File not found:") + filename);
	wxImage img;
	img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_PNG);
	wxBitmap ret(img);
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

    try
	{
        toolbar->AddTool(ID_BTN_EXPAND_TREE,    _("Expand all nodes"),   LoadPNGFromResourceFile(wxT("images/expand_16x16.png")) );
        toolbar->AddTool(ID_BTN_COLLAPSE_TREE,  _("Collapse all nodes"), LoadPNGFromResourceFile(wxT("images/collapse_16x16.png")) );

	} catch (bad_file_execp e)
	{
        Manager::Get()->GetLogManager()->LogError(_("cbSystemView: ") + e.m_msg);
	}

	wxSize ToolSize = toolbar->GetToolSize();
	unsigned int search_control_pos = ToolSize.GetWidth() * toolbar->GetToolsCount() + 10;
	m_SearchCtrl = new wxTextCtrl(m_pg_man->GetToolBar(), ID_SEARCH_CTRL, _(""), wxPoint(search_control_pos, 0), wxSize(150, ToolSize.GetHeight()) );



	toolbar->AddControl(m_SearchCtrl);
    toolbar->Realize();
    toolbar->SetInitialSize();

    m_reader = new cbSVDFileReader();
    m_device = new SVDDevice();

    m_CurSVDFile = wxEmptyString;

    m_pg_first_page->SetColumnCount(4);
    m_pg_first_page->Append( new wxPropertyCategory(_T("Category per") ));

    m_searchTimer = new wxTimer(this, ID_SEARCH_TIMER);

    PopulateGrid();
}

cbSVWindow::~cbSVWindow()
{
    delete m_reader;
    delete m_device;
    delete m_searchTimer;
    //dtor
}

void cbSVWindow::DeleteAllWatches()
{
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end() ; ++itr)
    {
        dbg->DeleteWatch(itr->m_watch);
        m_RegisterWatches.erase(itr);
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
        m_pg_first_page->ExpandAll();
    else if(event.GetId() == ID_BTN_COLLAPSE_TREE)
        m_pg_first_page->ExpandAll(false);
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
        //PopulateSVDPherypheryPropGrid(m_pg_first_page,*(*itr_per).get());
    }
}

void cbSVWindow::UpdateWatches()
{
    auto itr = m_RegisterWatches.begin();
    for(; itr != m_RegisterWatches.end() ; ++itr)
    {
        wxString val;
        (*itr).m_watch->GetValue(val);
        dynamic_cast<svPGRegisterProp*>((*itr).m_property)->SetValueFromString(val);
    }
}

void cbSVWindow::GenerateWatchesRecursive(wxPGProperty* prop, cbDebuggerPlugin *dbg)
{
    size_t child_count = prop->GetChildCount();
    bool isRegister = prop->IsKindOf(CLASSINFO(svPGRegisterProp) );
    if (isRegister )
    {
        std::list<RegisterWatch>::iterator watch_itr = FindWatchFromProperty(prop);
        if(prop->IsExpanded() && watch_itr == m_RegisterWatches.end() )
        {
            RegisterWatch watch;
            svPGRegisterProp* reg = dynamic_cast<svPGRegisterProp*>(prop);
            if(reg != nullptr)
            {
                watch.m_property = prop;
                watch.m_watch    = dbg->AddMemoryRange( reg->GetAddress() , reg->GetSize(), reg->GetName() );

                m_RegisterWatches.push_back(watch);
            }
            else
            {
                Manager::Get()->GetLogManager()->LogError(_T("cbSVWindow::GenerateWatchesRecursive: if(reg == nullptr)"));
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

/*bool cbSVWindow::FindWatchFromProperty(wxPGProperty* prop )
{
    return FindWatchFromProperty(prop) == m_RegisterWatches.end();
}*/

std::list<RegisterWatch>::iterator  cbSVWindow::FindWatchFromProperty(wxPGProperty* prop )
{
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
    bool isRegister = prop->IsKindOf(CLASSINFO(svPGRegisterProp) );

    if(dbg->IsRunning() && isRegister )
    {
        svPGRegisterProp* reg = dynamic_cast<svPGRegisterProp*>(prop);
        if(reg == nullptr)
        {
            // Error
        }

        svPGPropBase* reg_base = dynamic_cast<svPGPropBase*>(reg);

        RegisterWatch watch;
        watch.m_property = prop;
        watch.m_watch    = dbg->AddMemoryRange( reg_base->GetAddress() , reg_base->GetSize(), reg->GetName() );

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
             break;
        }
    }
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
        {
             //prop->Item(i)->Hide( false );
             child_found = true;
        }
        //else
            //prop->Item(i)->Hide( true );
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

        //m_pg_first_page->ExpandAll();
        m_searchTimer->Stop();
        return;
    }
    else
    {
        m_searchTimer->Start(500, wxTIMER_ONE_SHOT);
    }

}
