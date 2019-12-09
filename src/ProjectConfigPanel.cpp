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


#include <sdk.h>
#include "ProjectConfigPanel.h"
#include <wx/intl.h>
#include <wx/xrc/xmlres.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/checkbox.h>

#include <cbproject.h>
#include <editpathdlg.h>
#include <manager.h>
#include <globals.h>
#include <logmanager.h>

//(*InternalHeaders(ProjectConfigPanel)
#include <wx/xrc/xmlres.h>
//*)

//(*IdInit(ProjectConfigPanel)
//*)

BEGIN_EVENT_TABLE(ProjectConfigPanel, wxPanel)
    //(*EventTable(ProjectConfigPanel)
    //*)
END_EVENT_TABLE()



ProjectConfigPanel::ProjectConfigPanel(wxWindow* parent,
                                       cbSystemView* plugin,
                                       cbProject* project)
{
    m_project = project;
    m_plugin = plugin;
    m_settings = plugin->GetSettings(m_project); // Copy the settings in a temporary buffer
    //(*Initialize(ProjectConfigPanel)
    wxXmlResource::Get()->LoadObject(this, parent, _T("ProjectConfigPanel"), _T("wxPanel"));
    m_lstTargets = (wxListBox*)FindWindow(XRCID("ID_LISTBOX1"));
    m_CtrlSvdFilePath = (wxTextCtrl*)FindWindow(XRCID("ID_TEXTCTRL1"));
    m_BtnBrowseSvdFilePath = (wxButton*)FindWindow(XRCID("ID_BUTTON1"));

    Connect(XRCID("ID_LISTBOX1"), wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&ProjectConfigPanel::OnlstTargetsSelect);
    Connect(XRCID("ID_BUTTON1"), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ProjectConfigPanel::OnBtnBrowseSvdFilePathClick);
    //*)

    Manager::Get()->RegisterEventSink(cbEVT_BUILDTARGET_REMOVED, new cbEventFunctor<ProjectConfigPanel, CodeBlocksEvent>(this, &ProjectConfigPanel::OnBuildTargetRemoved));
    Manager::Get()->RegisterEventSink(cbEVT_BUILDTARGET_ADDED,   new cbEventFunctor<ProjectConfigPanel, CodeBlocksEvent>(this, &ProjectConfigPanel::OnBuildTargetAdded));
    Manager::Get()->RegisterEventSink(cbEVT_BUILDTARGET_RENAMED, new cbEventFunctor<ProjectConfigPanel, CodeBlocksEvent>(this, &ProjectConfigPanel::OnBuildTargetRenamed));

    m_lstTargets->Clear();
    m_lstTargets->Append(_T("<Project>"));

    for (int i = 0; i < project->GetBuildTargetsCount(); ++i)
    {
        m_lstTargets->Append(project->GetBuildTarget(i)->GetTitle());
    }

    m_lstTargets->SetSelection(0);
    m_lastTargetSel = _T("<Project>");
    LoadCurrentSettings();
}

ProjectConfigPanel::~ProjectConfigPanel()
{
    //(*Destroy(ProjectConfigPanel)
    //*)
}


void ProjectConfigPanel::OnBtnBrowseSvdFilePathClick(wxCommandEvent& event)
{
    EditPathDlg dlg(this,
                    m_CtrlSvdFilePath->GetValue(),
                    m_project ? m_project->GetBasePath() : _T(""),
                    _("SVD file"),
                    _("Choose svd file for the debugger"),
                    false,
                    false,
                    _("SVD files (*.svd, *.xml)|*.svd;*.xml;|All files (*)|*"));

    PlaceWindow(&dlg);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_CtrlSvdFilePath->SetValue(dlg.GetPath());
    }
}

void ProjectConfigPanel::OnApply()
{
    SaveCurrentSettings();
    m_plugin->SetSettings(m_settings, m_project);
}

void ProjectConfigPanel::OnBuildTargetRemoved(CodeBlocksEvent& event)
{
    cbProject* project = event.GetProject();

    if (project != m_project)
    {
        return;
    }

    wxString theTarget = event.GetBuildTargetName();
    ProjectBuildTarget* bt = project->GetBuildTarget(theTarget);

    int idx = m_lstTargets->FindString(theTarget);

    if (idx > 0)
    {
        m_lstTargets->Delete(idx);
    }

    if ((size_t)idx >= m_lstTargets->GetCount())
    {
        idx--;
    }

    m_lstTargets->SetSelection(0);
    m_lastTargetSel = _T("<Project>");

    // remove the target from the map to ensure that there are no dangling pointers in it.
    if (bt != 0) // don't remove the project global setting
        m_settings.m_targetSettings.erase(bt);

    // This is not expected behaviour, but we can not undo the delete, so maybe this is expected...
    m_plugin->SetSettings(m_settings, m_project);
}

void ProjectConfigPanel::OnBuildTargetAdded(CodeBlocksEvent& event)
{
    cbProject* project = event.GetProject();

    if (project != m_project)
    {
        return;
    }

    wxString newTarget = event.GetBuildTargetName();
    wxString oldTarget = event.GetOldBuildTargetName();

    if (!oldTarget.IsEmpty())
    {
        for (auto itr = m_settings.m_targetSettings.begin(); itr != m_settings.m_targetSettings.end(); ++itr)
        {
            // find our target
            if (!itr->first || itr->first->GetTitle() != oldTarget)
                continue;

            ProjectBuildTarget* bt = m_project->GetBuildTarget(newTarget);

            if (bt)
                m_settings.m_targetSettings.insert(m_settings.m_targetSettings.end(), std::make_pair(bt, itr->second));

            // if we inserted it, just break, there can only be one map per target
            break;
        }
    }

    int idx = m_lstTargets->FindString(newTarget);

    if (idx == wxNOT_FOUND)
    {
        idx = m_lstTargets->Append(newTarget);
    }

    m_lstTargets->SetSelection(0);
    m_lastTargetSel = _T("<Project>");
}

void ProjectConfigPanel::OnBuildTargetRenamed(CodeBlocksEvent& event)
{
    cbProject* project = event.GetProject();

    if (project != m_project)
    {
        return;
    }

    wxString newTarget = event.GetBuildTargetName();
    wxString oldTarget = event.GetOldBuildTargetName();

    for (auto itr = m_settings.m_targetSettings.begin(); itr != m_settings.m_targetSettings.end(); ++itr)
    {
        // find our target
        if (!itr->first || itr->first->GetTitle() != oldTarget)
            continue;

        itr->first->SetTitle(newTarget);
        // if we renamed it, just break, there can only be one map per target
        break;
    }

    int idx = m_lstTargets->FindString(oldTarget);

    if (idx == wxNOT_FOUND)
    {
        return;
    }

    m_lstTargets->SetString(idx, newTarget);

    m_lstTargets->SetSelection(0);
    m_lastTargetSel = _T("<Project>");
}

void ProjectConfigPanel::SaveCurrentSettings()
{
    ProjectBuildTarget *bt = 0;

    if (m_lastTargetSel == _T("<Project>"))
    {
        bt = 0;
    }
    else
    {
        bt = m_project->GetBuildTarget(m_lastTargetSel);

        if (bt == nullptr)
            Manager::Get()->GetLogManager()->LogError(wxT("ProjectConfigPanel::SaveCurrentSettings(): The following build target does not exist; ") + m_lastTargetSel);
    }

    m_settings.m_targetSettings[bt].m_svdFilePath = m_CtrlSvdFilePath->GetValue();

}

void ProjectConfigPanel::LoadCurrentSettings()
{
    ProjectBuildTarget *bt = 0;

    if (m_lastTargetSel == _T("<Project>"))
    {
        bt = 0;
    }
    else
    {
        bt = m_project->GetBuildTarget(m_lastTargetSel);

        if (bt == nullptr)
            Manager::Get()->GetLogManager()->LogError(wxT("ProjectConfigPanel::LoadCurrentSettings(): The following build target does not exist; ") + m_lastTargetSel);
    }

    m_CtrlSvdFilePath->SetValue(m_settings.m_targetSettings[bt].m_svdFilePath);
}

void ProjectConfigPanel::OnlstTargetsSelect(wxCommandEvent& event)
{
    SaveCurrentSettings();
    m_lastTargetSel = m_lstTargets->GetString(m_lstTargets->GetSelection());
    LoadCurrentSettings();
}


