///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 20 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gui.h"

///////////////////////////////////////////////////////////////////////////

MainFrameBase::MainFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menuBar = new wxMenuBar( 0 );
	m_menuFile = new wxMenu();
	wxMenuItem* m_add;
	m_add = new wxMenuItem( m_menuFile, wxID_ANY, wxString( _("添加") ) , _("添加一个客户端"), wxITEM_NORMAL );
	m_menuFile->Append( m_add );
	
	wxMenuItem* menuFileExit;
	menuFileExit = new wxMenuItem( m_menuFile, wxID_EXIT, wxString( _("E&xit") ) + wxT('\t') + wxT("Alt+X"), wxEmptyString, wxITEM_NORMAL );
	m_menuFile->Append( menuFileExit );
	
	m_menuBar->Append( m_menuFile, _("&File") ); 
	
	this->SetMenuBar( m_menuBar );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	m_listctrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	mainSizer->Add( m_listctrl, 1, wxALL|wxEXPAND, 5 );
	
	m_logger = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP );
	mainSizer->Add( m_logger, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	m_statusBar = this->CreateStatusBar( 1, wxST_SIZEGRIP, wxID_ANY );
	action_menu = new wxMenu();
	wxMenuItem* m_view;
	m_view = new wxMenuItem( action_menu, wxID_ANY, wxString( _("观看屏幕") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_view );
	
	wxMenuItem* m_control;
	m_control = new wxMenuItem( action_menu, wxID_ANY, wxString( _("远程协助") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_control );
	
	wxMenuItem* m_shutdown;
	m_shutdown = new wxMenuItem( action_menu, wxID_ANY, wxString( _("远程关机") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_shutdown );
	
	wxMenuItem* m_message;
	m_message = new wxMenuItem( action_menu, wxID_ANY, wxString( _("发送信息") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_message );
	
	wxMenuItem* m_modify;
	m_modify = new wxMenuItem( action_menu, wxID_ANY, wxString( _("修改条目") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_modify );
	
	wxMenuItem* m_delete;
	m_delete = new wxMenuItem( action_menu, wxID_ANY, wxString( _("删除/恢复") ) , wxEmptyString, wxITEM_NORMAL );
	action_menu->Append( m_delete );
	
	this->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MainFrameBase::MainFrameBaseOnContextMenu ), NULL, this ); 
	
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainFrameBase::OnCloseFrame ) );
	this->Connect( m_add->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_OnAddClient ) );
	this->Connect( menuFileExit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnExitClick ) );
	m_listctrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainFrameBase::m_ListColClick ), NULL, this );
	m_listctrl->Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( MainFrameBase::m_ListRightClick ), NULL, this );
	this->Connect( m_view->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_viewOnMenuSelection ) );
	this->Connect( m_control->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_controlOnMenuSelection ) );
	this->Connect( m_shutdown->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_shutdownOnMenuSelection ) );
	this->Connect( m_message->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_messageOnMenuSelection ) );
	this->Connect( m_modify->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_modifyOnMenuSelection ) );
	this->Connect( m_delete->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_deleteOnMenuSelection ) );
}

MainFrameBase::~MainFrameBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainFrameBase::OnCloseFrame ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_OnAddClient ) );
	this->Disconnect( wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::OnExitClick ) );
	m_listctrl->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainFrameBase::m_ListColClick ), NULL, this );
	m_listctrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( MainFrameBase::m_ListRightClick ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_viewOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_controlOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_shutdownOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_messageOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_modifyOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainFrameBase::m_deleteOnMenuSelection ) );
	
	delete action_menu; 
}

AddDialog::AddDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, _("教室:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer3->Add( m_staticText2, 0, wxALL, 5 );
	
	m_name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_name, 1, wxALL, 5 );
	
	bSizer2->Add( bSizer3, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, _("IP:    "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer4->Add( m_staticText3, 0, wxALL, 5 );
	
	m_ip = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_ip, 1, wxALL, 5 );
	
	bSizer2->Add( bSizer4, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	bSizer2->Add( m_sdbSizer1, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer2 );
	this->Layout();
	bSizer2->Fit( this );
	
	this->Centre( wxBOTH );
}

AddDialog::~AddDialog()
{
}
