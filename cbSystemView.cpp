#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include "cbSystemView.h"
#include <debuggermanager.h>

int ID_DEBUG_WINDOW_MENU=wxNewId();

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<cbSystemView> reg(_T("cbSystemView"));
}


// events handling
BEGIN_EVENT_TABLE(cbSystemView, cbPlugin)
    // add any events you want to handle here
    EVT_MENU(ID_DEBUG_WINDOW_MENU,cbSystemView::OnWindowMenu)
END_EVENT_TABLE()

// constructor
cbSystemView::cbSystemView()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("cbSystemView.zip")))
    {
        NotifyMissingFile(_T("cbSystemView.zip"));
    }
}

// destructor
cbSystemView::~cbSystemView()
{
}

void cbSystemView::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...
    m_window = new cbSVWindow(Manager::Get()->GetAppWindow());
    CodeBlocksDockEvent evt(cbEVT_ADD_DOCK_WINDOW);
    evt.name = _T("SystemView");
    evt.title = _("System View Register window");
    evt.pWindow = m_window;
    evt.dockSide = CodeBlocksDockEvent::dsFloating;
    evt.desiredSize.Set(400, 300);
    evt.floatingSize.Set(400, 300);
    evt.minimumSize.Set(200, 150);
    Manager::Get()->ProcessEvent(evt);

    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_STARTED,  new cbEventFunctor<cbSystemView, CodeBlocksEvent>(this, &cbSystemView::OnDebuggerStarted));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_FINISHED, new cbEventFunctor<cbSystemView, CodeBlocksEvent>(this, &cbSystemView::OnDebuggerFinished));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_PAUSED,   new cbEventFunctor<cbSystemView, CodeBlocksEvent>(this, &cbSystemView::OnDebuggerPaused));

}

void cbSystemView::OnDebuggerStarted(CodeBlocksEvent& evt)
{
    //cbMessageBox(wxT("started"));
    //cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    //m_watch = dbg->AddMemoryRange(0x0028fefe, 1024, wxT("bla"));
}

void cbSystemView::OnDebuggerFinished(CodeBlocksEvent& evt)
{
    //cbMessageBox(wxT("finished"));
}

void cbSystemView::OnDebuggerPaused(CodeBlocksEvent& evt)
{
    //cbMessageBox(wxT("paused"));
    /*cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    wxString data;
    wxString msg = wxT("memory watch data: ");
    m_watch->GetValue(data);
    wxCharBuffer dd = data.To8BitData();
    msg << wxString::Format(wxT(" size: %d "), data.length());
    for(size_t i = 0; i < data.size(); ++i)
    {
        msg << wxString::Format(wxT(" 0x%x"), dd[i]);
    }
    dbg->Log(msg);*/
    m_window->UpdateWatches();

}

void cbSystemView::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    if (m_window) //remove the Shell Terminals Notebook from its dockable window and delete it
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_window;
        Manager::Get()->ProcessEvent(evt);
        m_window->Destroy();
    }
    m_window = 0;
}

void cbSystemView::OnWindowMenu(wxCommandEvent& event)
{
    CodeBlocksDockEvent evt(event.IsChecked() ? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_window;
    Manager::Get()->ProcessEvent(evt);
}

void cbSystemView::BuildMenu(wxMenuBar* menuBar)
{
    //The application is offering its menubar for your plugin,
    //to add any menu items you want...
    //Append any items you need in the menu...
    //NOTE: Be careful in here... The application's menubar is at your disposal.
    //NotImplemented(_T("cbSystemView::BuildMenu()"));
    /*int id = menuBar->FindMenu(wxT("Debug"));
    wxMenu* debug_menu = menuBar->GetMenu(id);
    if(debug_menu == nullptr)
    {
        Manager::Get()->GetLogManager()->LogError(_("\"Debug\" menu entry not found"));
    }
    else
    {
        wxMenuItem* window_menu = debug_menu->FindItem(debug_menu->FindItem(wxT("Debugging windows")));
        if(window_menu == nullptr)
        {
            Manager::Get()->GetLogManager()->LogError(_("\"Debug/Debugging windows\"  menu entry not found"));
        }
        else
        {
            window_menu->GetSubMenu()->AppendCheckItem(ID_DEBUG_WINDOW_MENU,wxT("System view"),wxT("Show/Hide System view window"));
        }
    }*/

    wxMenu* system_view_menu = new wxMenu();
    system_view_menu->AppendCheckItem(ID_DEBUG_WINDOW_MENU, wxT("Window"));

    menuBar->Append(system_view_menu, wxT("System View"));
}
