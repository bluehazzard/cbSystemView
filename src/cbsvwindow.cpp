#include "cbsvwindow.h"

const long cbSVWindow::ID_SEARCH_CTRL = wxNewId();
const long cbSVWindow::ID_BTN_EXPAND_TREE = wxNewId();
const long cbSVWindow::ID_BTN_COLLAPSE_TREE = wxNewId();

BEGIN_EVENT_TABLE(cbSVWindow,wxPanel)
	//(*EventTable(CPURegistersDlg)
	//*)
	EVT_TEXT( ID_SEARCH_CTRL, cbSVWindow::OnSearchCtrl )

	EVT_TOOL( ID_BTN_EXPAND_TREE,   cbSVWindow::OnModifyTree )
	EVT_TOOL( ID_BTN_COLLAPSE_TREE, cbSVWindow::OnModifyTree )

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
	img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_ANY
              );
	wxBitmap ret(img);
	if(!ret.IsOk())
        throw bad_file_execp(_("File not loaded correctly:") + filename);

	return ret;
}

cbSVWindow::cbSVWindow(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL/* | wxCLIP_CHILDREN*/)
{
    //ctor
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_pg_man = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_DESCRIPTION | wxPG_TOOLBAR);
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

    m_reader->LoadSVDFile(wxT("ATSAM3X8E.svd"), m_device);

    m_pg_first_page->SetColumnCount(4);
    m_pg_first_page->Append( new wxPropertyCategory(_T("Category per") ));

    PopulateGrid();
}

cbSVWindow::~cbSVWindow()
{
    delete m_reader;
    delete m_device;
    //dtor
}

void cbSVWindow::OnModifyTree(wxCommandEvent& event)
{
    if(event.GetId() == ID_BTN_EXPAND_TREE)
        m_pg_first_page->ExpandAll();
    else if(event.GetId() == ID_BTN_COLLAPSE_TREE)
        m_pg_first_page->ExpandAll(false);
}

bool FindString(wxString a, wxString b)
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

int PopulateSVDFieldPropGrid(wxPropertyGridPage *grid, wxPGProperty* parent, std::shared_ptr< SVDField >& per)
{

    if((per->m_bitRange.GetWidth() == 1) && (per->m_enumerated_value.m_values.size() == 0))
    {
        wxBoolProperty* field_prop = new wxBoolProperty(per->m_name);
        field_prop->SetHelpString(per->m_description);
        field_prop->SetAttribute(wxPG_BOOL_USE_CHECKBOX,true);
        //field_prop->SetCell(4,per->m_description);
        field_prop->SetValue(per->m_resetValue == 1 ? true : false);
        if(per->m_access == SVD_ACCESS_READ)
            field_prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
        //parent->AddPrivateChild(field_prop);
        grid->AppendIn(parent,field_prop);
    }
    else if(per->m_enumerated_value.m_values.size() != 0)
    {
        wxEnumProperty* enum_prop = new wxEnumProperty(per->m_name);
        grid->AppendIn(parent,enum_prop);
        enum_prop->SetHelpString(per->m_description);
        //enum_prop->SetCell(4,per->m_description);
        if(per->m_access == SVD_ACCESS_READ)
            enum_prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
        //enum_prop->SetValue(per->m_resetValue)
        auto value_itr = per->m_enumerated_value.m_values.begin();
        for(;value_itr != per->m_enumerated_value.m_values.end(); ++value_itr)
        {
            //enum_prop->AddChoice((*value_itr)->m_name + wxT("     : ") + (*value_itr)->m_description );
            enum_prop->AppendChoice((*value_itr)->m_name + wxT("     : ") + (*value_itr)->m_description);
        }
        //parent->AddPrivateChild(enum_prop);

    }
    else
    {
        wxStringProperty* field_prop = new wxStringProperty(per->m_name);
        field_prop->SetHelpString(per->m_description);
        //field_prop->SetCell(4,per->m_description);
        field_prop->SetValue(wxString::Format(wxT("0x%08llx") , per->m_resetValue));
        if(per->m_access == SVD_ACCESS_READ)
            field_prop->SetFlagRecursively(wxPG_PROP_READONLY, true);
        //parent->AddPrivateChild(field_prop);
        grid->AppendIn(parent,field_prop);
    }

}


int PopulateSVDRegisterPropGrid(wxPropertyGridPage *grid, wxPGProperty* parent, std::shared_ptr<SVDRegisterBase>& per)
{
    wxStringProperty* reg_prop = new wxStringProperty(per->m_name,parent->GetName() + wxT(":")+per->m_name);

    std::shared_ptr<SVDRegister> reg = std::dynamic_pointer_cast<SVDRegister>(per);
    if(reg)
    {
        reg_prop->SetHelpString(reg->m_description + wxT("\n") + reg->m_displayName);
        //grid->Insert(parent,reg_prop);
        grid->AppendIn(parent,reg_prop);
        //parent->AddPrivateChild(reg_prop);
        auto itr_field = reg->m_fields.begin();
        int i=0;
        for(;itr_field != reg->m_fields.end() ; ++itr_field)
        {
            PopulateSVDFieldPropGrid(grid,reg_prop,*itr_field);
            i++;
        }
    }
}

int PopulateSVDPherypheryPropGrid( wxPropertyGridPage *grid, SVDPeriphery& per)
{
    wxStringProperty* per_prop = new wxStringProperty(per.m_name);
    grid->Append(per_prop);
    auto itr_reg = per.m_registers.begin();
    for(;itr_reg != per.m_registers.end() ; ++itr_reg)
    {
        PopulateSVDRegisterPropGrid(grid,per_prop,*itr_reg);
    }
}


void cbSVWindow::PopulateGrid()
{
    m_pg_first_page->Clear();

    auto itr_per = m_device->m_peripherals.begin();
    for(;itr_per != m_device->m_peripherals.end(); ++itr_per)
    {
        PopulateSVDPherypheryPropGrid(m_pg_first_page,*(*itr_per).get());
    }
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

        m_pg_first_page->ExpandAll();
        return;
    }

    for (size_t i = 0 ; i < child_count; i++)
    {
       wxPGProperty* prop = root->Item(i);
       if (FindString(prop->GetLabel(),searchStr) == false )
           prop->Hide( true );
       else
           prop->Hide( false );
    }
    m_pg_first_page->ExpandAll();
}
