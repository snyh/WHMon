///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 20 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __gui__
#define __gui__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameBase
///////////////////////////////////////////////////////////////////////////////
class MainFrameBase : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menuBar;
		wxMenu* m_menuFile;
		wxListCtrl* m_listctrl;
		wxTextCtrl* m_logger;
		wxStatusBar* m_statusBar;
		wxMenu* action_menu;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnCloseFrame( wxCloseEvent& event ) { event.Skip(); }
		virtual void m_OnAddClient( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExitClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_ListColClick( wxListEvent& event ) { event.Skip(); }
		virtual void m_ListRightClick( wxListEvent& event ) { event.Skip(); }
		virtual void m_viewOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_controlOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_shutdownOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_messageOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_modifyOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_deleteOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		MainFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("文华学院"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 665,488 ), long style = wxCLOSE_BOX|wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~MainFrameBase();
		
		void MainFrameBaseOnContextMenu( wxMouseEvent &event )
		{
			this->PopupMenu( action_menu, event.GetPosition() );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class AddDialog
///////////////////////////////////////////////////////////////////////////////
class AddDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText3;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		wxTextCtrl* m_name;
		wxTextCtrl* m_ip;
		
		AddDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("添加/修改客户端信息"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~AddDialog();
	
};

#endif //__gui__
