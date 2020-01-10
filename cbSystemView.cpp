/*
 * cbSystemView
 * Copyright (C) 2017  bluehazzard
 *
 * This program is free software:   you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; this program is ONLY licensed under
 * version 3 of the License, later versions are explicitly excluded.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0>.
*/

#include <sdk.h> // Code::Blocks SDK
#include <cbdebugger_interfaces.h>
#include <macrosmanager.h>
#include <configurationpanel.h>
#include <debuggermanager.h>
#include <logmanager.h>
#include <projectloader_hooks.h>

#include "cbSystemView.h"
#include "cbSVProjectConfigPanel.h"

int ID_DEBUG_WINDOW_MENU = wxNewId();

const wxString cbSystemViewPerTargetSetting::PROJECT_TARGET_NAME = wxT("#project#");

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<cbSystemView> reg(_T("cbSystemView"));
}


// events handling
BEGIN_EVENT_TABLE(cbSystemView, cbPlugin)
    // add any events you want to handle here
    EVT_MENU(ID_DEBUG_WINDOW_MENU, cbSystemView::OnWindowMenu)
END_EVENT_TABLE()

// constructor
cbSystemView::cbSystemView()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if (!Manager::LoadResource(_T("cbSystemView.zip")))
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

    typedef cbEventFunctor<cbSystemView, CodeBlocksEvent> Functor;

    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_STARTED,
                                      new Functor(this, &cbSystemView::OnDebuggerStarted));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_FINISHED,
                                      new Functor(this, &cbSystemView::OnDebuggerFinished));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_CONTINUED,
                                      new Functor(this, &cbSystemView::OnDebuggerContinued));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_CURSOR_CHANGED,
                                      new Functor(this, &cbSystemView::OnDebuggerCursorChanged));
    Manager::Get()->RegisterEventSink(cbEVT_DEBUGGER_UPDATED,
                                      new Functor(this, &cbSystemView::OnDebuggerUpdated));
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE,
                                      new Functor(this, &cbSystemView::OnProjectActivated));
    Manager::Get()->RegisterEventSink(cbEVT_BUILDTARGET_SELECTED,
                                      new Functor(this, &cbSystemView::OnTargetSelected));

    ProjectLoaderHooks::HookFunctorBase* myhook = new ProjectLoaderHooks::HookFunctor<cbSystemView>(this, &cbSystemView::OnProjectLoadingHook);
    m_HookId = ProjectLoaderHooks::RegisterHook(myhook);

    DebuggerManager *debuggerManager = Manager::Get()->GetDebuggerManager();
    cbDebuggerMenuHandler *menuHandler = debuggerManager->GetMenuHandler();

    if (menuHandler)
    {
        struct Item : cbDebuggerWindowMenuItem
        {
                explicit Item(cbSVWindow* window) : m_window(window) {}
                void OnClick(bool enable) override
                {
                    CodeBlocksDockEvent evt(enable ? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
                    evt.pWindow = m_window;
                    Manager::Get()->ProcessEvent(evt);
                }
                bool IsEnabled() override
                {
                    return true;
                }
                bool IsChecked() override
                {
                    return IsWindowReallyShown(m_window);
                }
            private:
                cbSVWindow* m_window;
        };

        menuHandler->RegisterWindowMenu(_("System View"),
                                        _("Displays the content of MCU's registers, similar to the watches window"),
                                        new Item(m_window));
    }
}

cbSystemViewPerTargetSetting cbSystemView::GetCurrentActiveSetting()
{
    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();
    cbProject* project = Manager::Get()->GetProjectManager()->GetActiveProject();

    wxString build_target_name = project->GetActiveBuildTarget();
    ProjectBuildTarget *bt = project->GetBuildTarget(build_target_name);

    auto itr_pro = m_settings.find(project);

    if (itr_pro == m_settings.end())
    {
        Manager::Get()->GetLogManager()->LogError(_T("cbSystemView::OnDebuggerStarted(): active project has no sv setting"));
        return cbSystemViewPerTargetSetting();
    }

    auto itr_bt = itr_pro->second.m_targetSettings.find(bt);

    if (itr_bt == itr_pro->second.m_targetSettings.end())
    {
        // the current target has no settings, so fall back to project settings
        itr_bt = itr_pro->second.m_targetSettings.find(0);
    }

    // Load the correct svd File
    return itr_bt->second;
}

void cbSystemView::OnDebuggerStarted(CodeBlocksEvent& evt)
{
    LoadSVDFile();
    m_window->OnDebuggerStarted();
}

void cbSystemView::OnDebuggerFinished(CodeBlocksEvent& evt)
{
    //cbMessageBox(wxT("finished"));
}

void cbSystemView::OnDebuggerContinued(CodeBlocksEvent& evt)
{
    m_window->OnDebuggerContinued();
}

void cbSystemView::OnDebuggerCursorChanged(CodeBlocksEvent& evt)
{
    if (!IsWindowReallyShown(m_window))
        return;

    cbDebuggerPlugin *dbg = Manager::Get()->GetDebuggerManager()->GetActiveDebugger();

    if (dbg)
        dbg->RequestUpdate(cbDebuggerPlugin::DebugWindows::MemoryRange);
}

void cbSystemView::OnDebuggerUpdated(CodeBlocksEvent& evt)
{
    if (cbDebuggerPlugin::DebugWindows(evt.GetInt()) != cbDebuggerPlugin::DebugWindows::MemoryRange)
        return;

    m_window->UpdateWatches();
}

void cbSystemView::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...

    ProjectLoaderHooks::UnregisterHook(m_HookId, true);

    if (m_window) //remove the Shell Terminals Notebook from its dockable window and delete it
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_window;
        Manager::Get()->ProcessEvent(evt);
        m_window->Destroy();
    }

    m_window = nullptr;

    DebuggerManager *debuggerManager = Manager::Get()->GetDebuggerManager();
    cbDebuggerMenuHandler *menuHandler = debuggerManager->GetMenuHandler();

    if (menuHandler)
        menuHandler->UnregisterWindowMenu(_("System View"));
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
}

cbConfigurationPanel* cbSystemView::GetProjectConfigurationPanel(wxWindow* parent, cbProject* project)
{
    ProjectConfigPanel* panel = new ProjectConfigPanel(parent, this, project);
    return panel;
}

void cbSystemView::LoadSVDFile()
{
    cbSystemViewPerTargetSetting settings = GetCurrentActiveSetting();
    cbProject* project = Manager::Get()->GetProjectManager()->GetActiveProject();

    if (project)
    {
        wxString build_target_name = project->GetActiveBuildTarget();
        ProjectBuildTarget *bt = project->GetBuildTarget(build_target_name);

        wxString svdFilePath = settings.m_svdFilePath;

        if (svdFilePath == wxEmptyString)
        {
            svdFilePath = m_settings[project].m_targetSettings[0].m_svdFilePath;
        }

        Manager::Get()->GetMacrosManager()->ReplaceMacros(svdFilePath, bt);
        Manager::Get()->GetLogManager()->Log(_T("Load SVD file: ") + svdFilePath + " for target:" + build_target_name);

        m_window->SetSVDFile(svdFilePath);
    }
}


void cbSystemView::OnProjectActivated(CodeBlocksEvent& evt)
{
    LoadSVDFile();
}

void cbSystemView::OnTargetSelected(CodeBlocksEvent& evt)
{
    // Get target selected, and load the appropriate SVD file, update the grid to match, if we have a project
    if (Manager::Get()->GetProjectManager()->GetActiveProject())
        LoadSVDFile();
}

void cbSystemView::OnProjectLoadingHook(cbProject* project, TiXmlElement* elem, bool loading)
{
    auto itr = m_settings.find(project);

    if (loading)
    {
        if (itr == m_settings.end())
            m_settings[project] = cbSystemViewSetting();

        m_settings[project].LoadFromNode(elem, project);
    }
    else
    {
        if (itr == m_settings.end())
            m_settings[project] = cbSystemViewSetting();

        m_settings[project].SaveToNode(elem, project);
    }

}

cbSystemViewSetting cbSystemView::GetSettings(cbProject* project)
{
    auto itr = m_settings.find(project);

    if (itr == m_settings.end())
    {
        m_settings[project] = cbSystemViewSetting();
    }

    return m_settings[project];
}

void cbSystemView::SetSettings(cbSystemViewSetting settings, cbProject* project)
{
    m_settings[project] = settings;
}

void cbSystemViewPerTargetSetting::SaveToNode(TiXmlNode* node)
{

    TiXmlElement* svdFileNode = node->FirstChildElement("svdFile");

    if (svdFileNode == nullptr)  // We have not found the right node, so we create it
        svdFileNode = node->InsertEndChild(TiXmlElement("svdFile"))->ToElement();

    svdFileNode->SetAttribute("path", cbU2C(m_svdFilePath));
}

void cbSystemViewPerTargetSetting::LoadFromNode(TiXmlNode* node)
{
    TiXmlElement* svdFileNode = node->FirstChildElement("svdFile");

    if (svdFileNode != nullptr)
    {
        m_svdFilePath = cbC2U(svdFileNode->Attribute("path"));
    }
}


void cbSystemViewSetting::SaveToNode(TiXmlNode* node, cbProject* project)
{
    // First search for the target node with the right target name
    TiXmlElement* conf = node->FirstChildElement("cbSystemView");

    if (!conf)
    {
        conf = node->InsertEndChild(TiXmlElement("cbSystemView"))->ToElement();
    }

    for (auto itr = m_targetSettings.begin(); itr != m_targetSettings.end(); ++itr)
    {
        TiXmlElement* targetNode = conf->FirstChildElement("target");
        wxString settingTargetName = itr->first == 0 ? cbSystemViewPerTargetSetting::PROJECT_TARGET_NAME : itr->first->GetTitle();

        while (targetNode)
        {
            wxString targetName = cbC2U(targetNode->Attribute("name"));

            if (targetName == settingTargetName)
                break;

            targetNode = targetNode->NextSiblingElement("target");
        }

        if (targetNode == nullptr)
        {
            targetNode = conf->InsertEndChild(TiXmlElement("target"))->ToElement();
            targetNode->SetAttribute("name", cbU2C(settingTargetName));
        }

        itr->second.SaveToNode(targetNode);
    }
}

void cbSystemViewSetting::LoadFromNode(TiXmlNode* node, cbProject* project)
{
    TiXmlElement* conf = node->FirstChildElement("cbSystemView");

    if (conf)
    {
        TiXmlElement* targetNode = conf->FirstChildElement("target");

        while (targetNode)
        {
            wxString targetName = cbC2U(targetNode->Attribute("name"));
            ProjectBuildTarget* bt = 0;

            if (targetName == cbSystemViewPerTargetSetting::PROJECT_TARGET_NAME)
            {
                bt = 0;
            }
            else
            {
                bt = project->GetBuildTarget(targetName);

                if (bt == nullptr)
                {
                    targetNode = targetNode->NextSiblingElement("target");
                    continue;
                }
            }

            cbSystemViewPerTargetSetting trgSet;
            trgSet.LoadFromNode(targetNode);
            m_targetSettings[bt] = trgSet;

            targetNode = targetNode->NextSiblingElement("target");
        }
    }
}



