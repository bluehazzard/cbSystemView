#ifndef PROJECTCONFIGPANEL_H
#define PROJECTCONFIGPANEL_H
#include "cbproject.h"
//(*Headers(ProjectConfigPanel)
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
//*)
#include "configurationpanel.h"
#include "cbSystemView.h"

class cbSystemViewSetting;

class ProjectConfigPanel: public cbConfigurationPanel
{
	public:

		ProjectConfigPanel(wxWindow* parent,
                           cbSystemView* plugin,
                           cbProject* project);

		virtual ~ProjectConfigPanel();

		//(*Declarations(ProjectConfigPanel)
		wxButton* m_BtnBrowseSvdFilePath;
		wxListBox* m_lstTargets;
		wxTextCtrl* m_CtrlSvdFilePath;
		//*)

        /// @return the panel's title.
        virtual wxString GetTitle() const { return _("System view configuration"); };
        /// @return the panel's bitmap base name. You must supply two bitmaps: \<basename\>.png and \<basename\>-off.png...
        virtual wxString GetBitmapBaseName() const {{ return wxT("systemview"); };};
        /// Called when the user chooses to apply the configuration.
        virtual void OnApply();
        /// Called when the user chooses to cancel the configuration.
        virtual void OnCancel() {};

        void ReadSettings();


        void OnBuildTargetRemoved(CodeBlocksEvent& event);
        void OnBuildTargetAdded(CodeBlocksEvent& event);
        void OnBuildTargetRenamed(CodeBlocksEvent& event);

        void SaveCurrentSettings();
        void LoadCurrentSettings();

	protected:

		//(*Identifiers(ProjectConfigPanel)
		//*)

	private:

	    cbProject* m_project;
	    cbSystemView* m_plugin;
	    cbSystemViewSetting m_settings;
	    wxString m_lastTargetSel;

		//(*Handlers(ProjectConfigPanel)
		void OnBtnBrowseSvdFilePathClick(wxCommandEvent& event);
		void OnlstTargetsSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};


#endif
