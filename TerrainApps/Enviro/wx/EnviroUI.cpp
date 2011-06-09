///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "EnviroUI.h"

///////////////////////////////////////////////////////////////////////////

StartupDlgBase::StartupDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_text1 = new wxStaticText( this, ID_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( 0 );
	bSizer6->Add( m_text1, 0, wxALIGN_CENTER, 0 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Launch with:") ), wxVERTICAL );
	
	id_earthview = new wxRadioButton( this, ID_EARTHVIEW, wxT("Earth View"), wxDefaultPosition, wxDefaultSize, 0 );
	id_earthview->SetValue( true ); 
	sbSizer2->Add( id_earthview, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 10 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer7->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_imagetext = new wxStaticText( this, ID_IMAGETEXT, wxT("Image:  "), wxDefaultPosition, wxDefaultSize, 0 );
	m_imagetext->Wrap( -1 );
	bSizer7->Add( m_imagetext, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_image = new wxComboBox( this, ID_IMAGE, wxEmptyString, wxDefaultPosition, wxSize( 125,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer7->Add( m_image, 1, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer2->Add( bSizer7, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_terrain = new wxRadioButton( this, ID_TERRAIN, wxT("Terrain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrain->SetValue( true ); 
	sbSizer2->Add( m_terrain, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer8->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_tnameChoices[] = { wxT("Item") };
	int m_tnameNChoices = sizeof( m_tnameChoices ) / sizeof( wxString );
	m_tname = new wxChoice( this, ID_TNAME, wxDefaultPosition, wxSize( 190,-1 ), m_tnameNChoices, m_tnameChoices, 0 );
	m_tname->SetSelection( 0 );
	bSizer8->Add( m_tname, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_editprop = new wxButton( this, ID_EDITPROP, wxT("Edit Properties"), wxDefaultPosition, wxDefaultSize, 0 );
	m_editprop->SetDefault(); 
	bSizer8->Add( m_editprop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer2->Add( bSizer8, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	bSizer6->Add( sbSizer2, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 10 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_terrman = new wxButton( this, ID_TERRMAN, wxT("Terrain Manager"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrman->SetDefault(); 
	bSizer9->Add( m_terrman, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_options = new wxButton( this, ID_OPTIONS, wxT("Global Options"), wxDefaultPosition, wxDefaultSize, 0 );
	m_options->SetDefault(); 
	bSizer9->Add( m_options, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_opengl = new wxButton( this, ID_OPENGL, wxT("OpenGL Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_opengl->SetDefault(); 
	bSizer9->Add( m_opengl, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer6->Add( bSizer9, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ok->SetDefault(); 
	bSizer10->Add( ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	cancel->SetDefault(); 
	bSizer10->Add( cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer6->Add( bSizer10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer6 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

StartupDlgBase::~StartupDlgBase()
{
}

SceneGraphDlgBase::SceneGraphDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_scenetree = new wxTreeCtrl( this, ID_SCENETREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxSUNKEN_BORDER );
	bSizer11->Add( m_scenetree, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_enabled = new wxCheckBox( this, ID_ENABLED, wxT("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enabled->SetValue(true); 
	bSizer12->Add( m_enabled, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_zoomto = new wxButton( this, ID_ZOOMTO, wxT("Zoom To"), wxDefaultPosition, wxDefaultSize, 0 );
	m_zoomto->SetDefault(); 
	bSizer12->Add( m_zoomto, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_refresh = new wxButton( this, ID_REFRESH, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
	m_refresh->SetDefault(); 
	bSizer12->Add( m_refresh, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_log = new wxButton( this, ID_LOG, wxT("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_log->SetDefault(); 
	bSizer12->Add( m_log, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer11->Add( bSizer12, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer11 );
	this->Layout();
	bSizer11->Fit( this );
	
	this->Centre( wxBOTH );
}

SceneGraphDlgBase::~SceneGraphDlgBase()
{
}

PlantDlgBase::PlantDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text41 = new wxStaticText( this, ID_TEXT, wxT(" Species:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text41->Wrap( -1 );
	bSizer86->Add( m_text41, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString mmpeciesChoices;
	mmpecies = new wxChoice( this, ID_SPECIES, wxDefaultPosition, wxSize( 160,-1 ), mmpeciesChoices, 0 );
	mmpecies->SetSelection( 0 );
	bSizer86->Add( mmpecies, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer85->Add( bSizer86, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxHORIZONTAL );
	
	m_common_names = new wxCheckBox( this, ID_COMMON_NAMES, wxT("Common names"), wxDefaultPosition, wxDefaultSize, 0 );
	m_common_names->SetValue(true); 
	bSizer87->Add( m_common_names, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text42 = new wxStaticText( this, ID_TEXT, wxT("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text42->Wrap( -1 );
	bSizer87->Add( m_text42, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString id_languageChoices;
	id_language = new wxChoice( this, ID_LANGUAGE, wxDefaultPosition, wxSize( 60,-1 ), id_languageChoices, 0 );
	id_language->SetSelection( 0 );
	bSizer87->Add( id_language, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer85->Add( bSizer87, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer88;
	bSizer88 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text221 = new wxStaticText( this, ID_TEXT, wxT("    Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text221->Wrap( -1 );
	bSizer88->Add( m_text221, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_height_edit = new wxTextCtrl( this, ID_PLANT_HEIGHT_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer88->Add( m_plant_height_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text43 = new wxStaticText( this, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text43->Wrap( -1 );
	bSizer88->Add( m_text43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_height_slider = new wxSlider( this, ID_HEIGHT_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 120,-1 ), wxSL_HORIZONTAL );
	bSizer88->Add( m_height_slider, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer85->Add( bSizer88, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Planting Options") ), wxVERTICAL );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxVERTICAL );
	
	m_plant_individual = new wxRadioButton( this, ID_PLANT_INDIVIDUAL, wxT("Individual instances"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_individual->SetValue( true ); 
	bSizer89->Add( m_plant_individual, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_plant_linear = new wxRadioButton( this, ID_PLANT_LINEAR, wxT("Straight line"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_linear->SetValue( true ); 
	m_plant_linear->Enable( false );
	
	bSizer89->Add( m_plant_linear, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_plant_continuous = new wxRadioButton( this, ID_PLANT_CONTINUOUS, wxT("Continuously"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_continuous->SetValue( true ); 
	m_plant_continuous->Enable( false );
	
	bSizer89->Add( m_plant_continuous, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer18->Add( bSizer89, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text44 = new wxStaticText( this, ID_TEXT, wxT("Height variance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text44->Wrap( -1 );
	bSizer90->Add( m_text44, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_variance_edit = new wxTextCtrl( this, ID_PLANT_VARIANCE_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer90->Add( m_plant_variance_edit, 0, wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM, 5 );
	
	m_text45 = new wxStaticText( this, ID_TEXT, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text45->Wrap( 0 );
	bSizer90->Add( m_text45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_variance_slider = new wxSlider( this, ID_PLANT_VARIANCE_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer90->Add( m_plant_variance_slider, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer18->Add( bSizer90, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text46 = new wxStaticText( this, ID_TEXT, wxT("Minimum spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text46->Wrap( -1 );
	bSizer91->Add( m_text46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_spacing_edit = new wxTextCtrl( this, ID_PLANT_SPACING_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer91->Add( m_plant_spacing_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text47 = new wxStaticText( this, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text47->Wrap( 0 );
	bSizer91->Add( m_text47, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer18->Add( bSizer91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	bSizer85->Add( sbSizer18, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( bSizer85 );
	this->Layout();
	bSizer85->Fit( this );
	
	this->Centre( wxBOTH );
}

PlantDlgBase::~PlantDlgBase()
{
}

LocationDlgBase::LocationDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer94;
	bSizer94 = new wxBoxSizer( wxVERTICAL );
	
	m_loclist = new wxListBox( this, ID_LOCLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer94->Add( m_loclist, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer95;
	bSizer95 = new wxBoxSizer( wxHORIZONTAL );
	
	m_save = new wxButton( this, ID_SAVE, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save->SetDefault(); 
	bSizer95->Add( m_save, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_load = new wxButton( this, ID_LOAD, wxT("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	m_load->SetDefault(); 
	bSizer95->Add( m_load, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer94->Add( bSizer95, 0, wxALIGN_CENTER|wxALL, 0 );
	
	bSizer93->Add( bSizer94, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0 );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxVERTICAL );
	
	m_recall = new wxButton( this, ID_RECALL, wxT("Recall ->"), wxDefaultPosition, wxDefaultSize, 0 );
	m_recall->SetDefault(); 
	bSizer96->Add( m_recall, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_store = new wxButton( this, ID_STORE, wxT("<- Store"), wxDefaultPosition, wxDefaultSize, 0 );
	m_store->SetDefault(); 
	bSizer96->Add( m_store, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_storeas = new wxButton( this, ID_STOREAS, wxT("Store As.."), wxDefaultPosition, wxDefaultSize, 0 );
	m_storeas->SetDefault(); 
	bSizer96->Add( m_storeas, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_remove = new wxButton( this, ID_REMOVE, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	m_remove->SetDefault(); 
	bSizer96->Add( m_remove, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer93->Add( bSizer96, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );
	
	bSizer92->Add( bSizer93, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_line = new wxStaticLine( this, ID_LINE, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer92->Add( m_line, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer97;
	bSizer97 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxVERTICAL );
	
	m_animtree = new wxTreeCtrl( this, ID_ANIMTREE, wxDefaultPosition, wxSize( 120,-1 ), wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxSUNKEN_BORDER );
	bSizer98->Add( m_animtree, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer99;
	bSizer99 = new wxBoxSizer( wxHORIZONTAL );
	
	m_new_anim = new wxButton( this, ID_NEW_ANIM, wxT("New"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_new_anim->SetDefault(); 
	bSizer99->Add( m_new_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	m_save_anim = new wxButton( this, ID_SAVE_ANIM, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save_anim->SetDefault(); 
	bSizer99->Add( m_save_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	m_load_anim = new wxButton( this, ID_LOAD_ANIM, wxT("Load"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_load_anim->SetDefault(); 
	bSizer99->Add( m_load_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	
	bSizer99->Add( 7, 14, 0, wxALIGN_CENTER, 5 );
	
	m_reset = new wxBitmapButton( this, ID_RESET, wxBitmap( wxT("bitmap/dummy_16x15.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_reset->SetDefault(); 
	bSizer99->Add( m_reset, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_stop = new wxBitmapButton( this, ID_STOP, wxBitmap( wxT("bitmap/dummy_16x15.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_stop->SetDefault(); 
	bSizer99->Add( m_stop, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_record1 = new wxBitmapButton( this, ID_RECORD1, wxBitmap( wxT("bitmap/dummy_16x15.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_record1->SetDefault(); 
	bSizer99->Add( m_record1, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_play = new wxBitmapButton( this, ID_PLAY, wxBitmap( wxT("bitmap/dummy_16x15.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_play->SetDefault(); 
	bSizer99->Add( m_play, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	bSizer98->Add( bSizer99, 0, wxALIGN_CENTER, 5 );
	
	bSizer97->Add( bSizer98, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer92->Add( bSizer97, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxHORIZONTAL );
	
	m_anim_pos = new wxSlider( this, ID_ANIM_POS, 0, 0, 1000, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer100->Add( m_anim_pos, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_active = new wxCheckBox( this, ID_ACTIVE, wxT("Active"), wxDefaultPosition, wxDefaultSize, 0 );
	m_active->SetValue(true); 
	bSizer100->Add( m_active, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer92->Add( bSizer100, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Playback") ), wxVERTICAL );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text49 = new wxStaticText( this, ID_TEXT, wxT("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text49->Wrap( 0 );
	bSizer101->Add( m_text49, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speedslider = new wxSlider( this, ID_SPEEDSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer101->Add( m_speedslider, 1, wxALIGN_CENTER, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer101->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text223 = new wxStaticText( this, wxID_ANY, wxT("m/s"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text223->Wrap( 0 );
	bSizer101->Add( m_text223, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer19->Add( bSizer101, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxHORIZONTAL );
	
	m_loop = new wxCheckBox( this, ID_LOOP, wxT("Loop"), wxDefaultPosition, wxDefaultSize, 0 );
	m_loop->SetValue(true); 
	bSizer102->Add( m_loop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_continuous = new wxCheckBox( this, ID_CONTINUOUS, wxT("Continuous"), wxDefaultPosition, wxDefaultSize, 0 );
	m_continuous->SetValue(true); 
	bSizer102->Add( m_continuous, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_smooth = new wxCheckBox( this, ID_SMOOTH, wxT("Smooth"), wxDefaultPosition, wxDefaultSize, 0 );
	m_smooth->SetValue(true); 
	bSizer102->Add( m_smooth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_pos_only = new wxCheckBox( this, ID_POS_ONLY, wxT("Position Only"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pos_only->SetValue(true); 
	bSizer102->Add( m_pos_only, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer19->Add( bSizer102, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_play_to_disk = new wxButton( this, ID_PLAY_TO_DISK, wxT("Play animation to disk as a series of images"), wxDefaultPosition, wxDefaultSize, 0 );
	m_play_to_disk->SetDefault(); 
	sbSizer19->Add( m_play_to_disk, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer92->Add( sbSizer19, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer20;
	sbSizer20 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Recording") ), wxVERTICAL );
	
	m_record_linear = new wxRadioButton( this, ID_RECORD_LINEAR, wxT("Use linear distance"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_record_linear->SetValue( true ); 
	sbSizer20->Add( m_record_linear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxHORIZONTAL );
	
	m_record_interval = new wxRadioButton( this, ID_RECORD_INTERVAL, wxT("Time interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_record_interval->SetValue( true ); 
	bSizer103->Add( m_record_interval, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_record_spacing = new wxTextCtrl( this, ID_RECORD_SPACING, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer103->Add( m_record_spacing, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text50 = new wxStaticText( this, ID_TEXT, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text50->Wrap( 0 );
	bSizer103->Add( m_text50, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer20->Add( bSizer103, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer92->Add( sbSizer20, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( bSizer92 );
	this->Layout();
	bSizer92->Fit( this );
	
	this->Centre( wxBOTH );
}

LocationDlgBase::~LocationDlgBase()
{
}

CameraDlgBase::CameraDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* CamSizerTop;
	CamSizerTop = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text51 = new wxStaticText( this, ID_TEXT, wxT("Position:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text51->Wrap( 0 );
	bSizer105->Add( m_text51, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_camx = new wxTextCtrl( this, ID_CAMX, wxEmptyString, wxDefaultPosition, wxSize( 120,-1 ), wxTE_PROCESS_ENTER );
	bSizer105->Add( m_camx, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_camy = new wxTextCtrl( this, ID_CAMY, wxEmptyString, wxDefaultPosition, wxSize( 120,-1 ), wxTE_PROCESS_ENTER );
	bSizer105->Add( m_camy, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	CamSizerTop->Add( bSizer105, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text52 = new wxStaticText( this, ID_TEXT, wxT("Elevation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text52->Wrap( 0 );
	bSizer106->Add( m_text52, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_camz = new wxTextCtrl( this, ID_CAMZ, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), wxTE_PROCESS_ENTER );
	bSizer106->Add( m_camz, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( bSizer106, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxHORIZONTAL );
	
	m_fov_text = new wxStaticText( this, ID_FOV_TEXT, wxT("Horizontal FOV (degrees)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fov_text->Wrap( -1 );
	bSizer107->Add( m_fov_text, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fov = new wxTextCtrl( this, ID_FOV, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer107->Add( m_fov, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_fovslider = new wxSlider( this, ID_FOVSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer107->Add( m_fovslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( bSizer107, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	wxBoxSizer* CamSizerNear;
	CamSizerNear = new wxBoxSizer( wxHORIZONTAL );
	
	m_text53 = new wxStaticText( this, ID_TEXT, wxT("Near Clipping Plane (meters):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text53->Wrap( -1 );
	CamSizerNear->Add( m_text53, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_near = new wxTextCtrl( this, ID_NEAR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	CamSizerNear->Add( m_near, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_nearslider = new wxSlider( this, ID_NEARSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	CamSizerNear->Add( m_nearslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( CamSizerNear, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* CamSizerFar;
	CamSizerFar = new wxBoxSizer( wxHORIZONTAL );
	
	m_text54 = new wxStaticText( this, ID_TEXT, wxT("Far Clipping Plane (meters):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text54->Wrap( -1 );
	CamSizerFar->Add( m_text54, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_far = new wxTextCtrl( this, ID_FAR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	CamSizerFar->Add( m_far, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_farslider = new wxSlider( this, ID_FARSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	CamSizerFar->Add( m_farslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( CamSizerFar, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer110;
	bSizer110 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text55 = new wxStaticText( this, ID_TEXT, wxT("Stereo Eye Separation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text55->Wrap( -1 );
	bSizer110->Add( m_text55, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_eye_sep = new wxTextCtrl( this, ID_EYE_SEP, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer110->Add( m_eye_sep, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_eye_sepslider = new wxSlider( this, ID_EYE_SEPSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer110->Add( m_eye_sepslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( bSizer110, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text56 = new wxStaticText( this, ID_TEXT, wxT("Stereo Fusion Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text56->Wrap( -1 );
	bSizer111->Add( m_text56, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fusion_dist = new wxTextCtrl( this, ID_FUSION_DIST, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer111->Add( m_fusion_dist, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_fusion_dist_slider = new wxSlider( this, ID_FUSION_DIST_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer111->Add( m_fusion_dist_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	CamSizerTop->Add( bSizer111, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_line = new wxStaticLine( this, ID_LINE, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	CamSizerTop->Add( m_line, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer112;
	bSizer112 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text8 = new wxStaticText( this, ID_TEXT, wxT("Navigation Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text8->Wrap( -1 );
	bSizer112->Add( id_text8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer112->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_speedslider = new wxSlider( this, ID_SPEEDSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer112->Add( m_speedslider, 0, wxALIGN_CENTER, 0 );
	
	CamSizerTop->Add( bSizer112, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer113;
	bSizer113 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text57 = new wxStaticText( this, ID_TEXT, wxT("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text57->Wrap( 0 );
	bSizer113->Add( m_text57, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_speed_unitsChoices;
	m_speed_units = new wxChoice( this, ID_SPEED_UNITS, wxDefaultPosition, wxSize( 100,-1 ), m_speed_unitsChoices, 0 );
	m_speed_units->SetSelection( 0 );
	bSizer113->Add( m_speed_units, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer113->Add( 55, 20, 0, wxALIGN_CENTER, 0 );
	
	CamSizerTop->Add( bSizer113, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer114;
	bSizer114 = new wxBoxSizer( wxHORIZONTAL );
	
	m_accel = new wxCheckBox( this, ID_ACCEL, wxT("Accelerate by height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_accel->SetValue(true); 
	bSizer114->Add( m_accel, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	CamSizerTop->Add( bSizer114, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* CamSizerLOD;
	CamSizerLOD = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LOD Distance") ), wxVERTICAL );
	
	wxBoxSizer* bSizer115;
	bSizer115 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text58 = new wxStaticText( this, ID_TEXT, wxT("Vegetation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text58->Wrap( 0 );
	bSizer115->Add( m_text58, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_veg = new wxTextCtrl( this, ID_LOD_VEG, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer115->Add( m_lod_veg, 0, wxALIGN_CENTER, 5 );
	
	m_slider_veg = new wxSlider( this, ID_SLIDER_VEG, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer115->Add( m_slider_veg, 0, wxALIGN_CENTER, 0 );
	
	CamSizerLOD->Add( bSizer115, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer116;
	bSizer116 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text59 = new wxStaticText( this, ID_TEXT, wxT("Structures"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text59->Wrap( 0 );
	bSizer116->Add( m_text59, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_struct = new wxTextCtrl( this, ID_LOD_STRUCT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer116->Add( m_lod_struct, 0, wxALIGN_CENTER, 5 );
	
	m_slider_struct = new wxSlider( this, ID_SLIDER_STRUCT, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer116->Add( m_slider_struct, 0, wxALIGN_CENTER, 0 );
	
	CamSizerLOD->Add( bSizer116, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer117;
	bSizer117 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text60 = new wxStaticText( this, ID_TEXT, wxT("Roads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text60->Wrap( 0 );
	bSizer117->Add( m_text60, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_road = new wxTextCtrl( this, ID_LOD_ROAD, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer117->Add( m_lod_road, 0, wxALIGN_CENTER, 5 );
	
	m_slider_road = new wxSlider( this, ID_SLIDER_ROAD, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer117->Add( m_slider_road, 0, wxALIGN_CENTER, 0 );
	
	CamSizerLOD->Add( bSizer117, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	CamSizerTop->Add( CamSizerLOD, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( CamSizerTop );
	this->Layout();
	CamSizerTop->Fit( this );
	
	this->Centre( wxBOTH );
}

CameraDlgBase::~CameraDlgBase()
{
}

TextDlgBase::TextDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer118;
	bSizer118 = new wxBoxSizer( wxVERTICAL );
	
	m_text61 = new wxTextCtrl( this, ID_TEXT, wxEmptyString, wxDefaultPosition, wxSize( 450,250 ), wxTE_MULTILINE|wxTE_READONLY );
	bSizer118->Add( m_text61, 0, wxALIGN_CENTER|wxALL, 5 );
	
	ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ok->SetDefault(); 
	bSizer118->Add( ok, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	this->SetSizer( bSizer118 );
	this->Layout();
	bSizer118->Fit( this );
	
	this->Centre( wxBOTH );
}

TextDlgBase::~TextDlgBase()
{
}

UtilDlgBase::UtilDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer119;
	bSizer119 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer120;
	bSizer120 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text62 = new wxStaticText( this, ID_TEXT, wxT("Structure Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text62->Wrap( -1 );
	bSizer120->Add( m_text62, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_structtypeChoices[] = { wxT("Item") };
	int m_structtypeNChoices = sizeof( m_structtypeChoices ) / sizeof( wxString );
	m_structtype = new wxChoice( this, ID_STRUCTTYPE, wxDefaultPosition, wxSize( 250,-1 ), m_structtypeNChoices, m_structtypeChoices, 0 );
	m_structtype->SetSelection( 0 );
	bSizer120->Add( m_structtype, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer119->Add( bSizer120, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer119 );
	this->Layout();
	bSizer119->Fit( this );
	
	this->Centre( wxBOTH );
}

UtilDlgBase::~UtilDlgBase()
{
}

TParamsDlgBase::TParamsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text2 = new wxStaticText( this, ID_TEXT, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2->Wrap( 0 );
	bSizer14->Add( m_text2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tname = new wxTextCtrl( this, ID_TNAME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer14->Add( m_tname, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer13->Add( bSizer14, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	TParamsPanel1 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, wxT("Grid") ), wxVERTICAL );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_grid = new wxRadioButton( TParamsPanel1, ID_USE_GRID, wxT("Grid Filename: "), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_use_grid->SetValue( true ); 
	bSizer16->Add( m_use_grid, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_filename = new wxComboBox( TParamsPanel1, ID_FILENAME, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename->Append( wxT("Item") );
	bSizer16->Add( m_filename, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer3->Add( bSizer16, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text3 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("Terrain LOD Method:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text3->Wrap( -1 );
	bSizer17->Add( m_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_lodmethodChoices[] = { wxT("Item") };
	int m_lodmethodNChoices = sizeof( m_lodmethodChoices ) / sizeof( wxString );
	m_lodmethod = new wxChoice( TParamsPanel1, ID_LODMETHOD, wxDefaultPosition, wxSize( 120,-1 ), m_lodmethodNChoices, m_lodmethodChoices, 0 );
	m_lodmethod->SetSelection( 0 );
	bSizer17->Add( m_lodmethod, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer3->Add( bSizer17, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text4 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("Triangle count target: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( -1 );
	bSizer18->Add( m_text4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_tri_count = new wxTextCtrl( TParamsPanel1, ID_TRI_COUNT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer18->Add( m_tri_count, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tristrips = new wxCheckBox( TParamsPanel1, ID_TRISTRIPS, wxT("Use strips/fans"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tristrips->SetValue(true); 
	bSizer18->Add( m_tristrips, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer3->Add( bSizer18, 0, wxALIGN_CENTER, 5 );
	
	bSizer15->Add( sbSizer3, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, wxT("TIN") ), wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_tin = new wxRadioButton( TParamsPanel1, ID_USE_TIN, wxT("TIN Filename:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_tin->SetValue( true ); 
	bSizer19->Add( m_use_tin, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_filename_tin = new wxComboBox( TParamsPanel1, ID_FILENAME_TIN, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename_tin->Append( wxT("Item") );
	bSizer19->Add( m_filename_tin, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer4->Add( bSizer19, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	bSizer15->Add( sbSizer4, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, wxT("Grid Tiles") ), wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_tileset = new wxRadioButton( TParamsPanel1, ID_USE_TILESET, wxT("Tileset .ini file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_tileset->SetValue( true ); 
	bSizer20->Add( m_use_tileset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_filename_tiles = new wxComboBox( TParamsPanel1, ID_FILENAME_TILES, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename_tiles->Append( wxT("Item") );
	bSizer20->Add( m_filename_tiles, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer5->Add( bSizer20, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text5 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("Vertex count target: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( -1 );
	bSizer21->Add( m_text5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_vtx_count = new wxTextCtrl( TParamsPanel1, ID_VTX_COUNT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer21->Add( m_vtx_count, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text6 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("RAM cache size (MB):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text6->Wrap( -1 );
	m_text6->Enable( false );
	
	bSizer21->Add( m_text6, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tile_cache_size = new wxTextCtrl( TParamsPanel1, ID_TILE_CACHE_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_tile_cache_size->Enable( false );
	
	bSizer21->Add( m_tile_cache_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer5->Add( bSizer21, 0, wxALIGN_CENTER, 5 );
	
	m_tile_threading = new wxCheckBox( TParamsPanel1, ID_TILE_THREADING, wxT("Use multithreading for asynchronous tile loading"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tile_threading->SetValue(true); 
	sbSizer5->Add( m_tile_threading, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer15->Add( sbSizer5, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, wxT("External") ), wxVERTICAL );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_external = new wxRadioButton( TParamsPanel1, ID_USE_EXTERNAL, wxT("External Data:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_external->SetValue( true ); 
	bSizer22->Add( m_use_external, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tt_external_data = new wxTextCtrl( TParamsPanel1, ID_TT_EXTERNAL_DATA, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer22->Add( m_tt_external_data, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer6->Add( bSizer22, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer15->Add( sbSizer6, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text7 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("Vertical Exaggeration:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( -1 );
	bSizer23->Add( m_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_vertexag = new wxTextCtrl( TParamsPanel1, ID_VERTEXAG, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer23->Add( m_vertexag, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text8 = new wxStaticText( TParamsPanel1, ID_TEXT, wxT("x"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text8->Wrap( 0 );
	bSizer23->Add( m_text8, 0, wxALIGN_CENTER|wxALL, 0 );
	
	bSizer15->Add( bSizer23, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	TParamsPanel1->SetSizer( bSizer15 );
	TParamsPanel1->Layout();
	bSizer15->Fit( TParamsPanel1 );
	m_notebook->AddPage( TParamsPanel1, wxT("Elevation"), true );
	TParamsPanel2 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel2, wxID_ANY, wxT("Texture") ), wxVERTICAL );
	
	m_none = new wxRadioButton( TParamsPanel2, ID_NONE, wxT("No texture"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_none->SetValue( true ); 
	sbSizer7->Add( m_none, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	m_single = new wxRadioButton( TParamsPanel2, ID_SINGLE, wxT("Single texture "), wxDefaultPosition, wxDefaultSize, 0 );
	m_single->SetValue( true ); 
	bSizer25->Add( m_single, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_tfile_single = new wxComboBox( TParamsPanel2, ID_TFILE_SINGLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_tfile_single->Append( wxT("Item") );
	bSizer25->Add( m_tfile_single, 1, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer7->Add( bSizer25, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_derived = new wxRadioButton( TParamsPanel2, ID_DERIVED, wxT("Derive texture from elevation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_derived->SetValue( true ); 
	sbSizer7->Add( m_derived, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer26->Add( 20, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_colorsChoices[] = { wxT("Item") };
	int m_choice_colorsNChoices = sizeof( m_choice_colorsChoices ) / sizeof( wxString );
	m_choice_colors = new wxChoice( TParamsPanel2, ID_CHOICE_COLORS, wxDefaultPosition, wxSize( 180,-1 ), m_choice_colorsNChoices, m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	bSizer26->Add( m_choice_colors, 1, wxALIGN_CENTER|wxALL, 5 );
	
	id_edit_colors = new wxButton( TParamsPanel2, ID_EDIT_COLORS, wxT("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	id_edit_colors->SetDefault(); 
	bSizer26->Add( id_edit_colors, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer7->Add( bSizer26, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	mmiled_4by4 = new wxRadioButton( TParamsPanel2, ID_TILED_4BY4, wxT("4x4 Tiled texture"), wxDefaultPosition, wxDefaultSize, 0 );
	mmiled_4by4->SetValue( true ); 
	sbSizer7->Add( mmiled_4by4, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	id_text1 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Tile size: "), wxDefaultPosition, wxDefaultSize, 0 );
	id_text1->Wrap( -1 );
	fgSizer1->Add( id_text1, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_choice_tilesizeChoices[] = { wxT("ChoiceItem") };
	int m_choice_tilesizeNChoices = sizeof( m_choice_tilesizeChoices ) / sizeof( wxString );
	m_choice_tilesize = new wxChoice( TParamsPanel2, ID_CHOICE_TILESIZE, wxDefaultPosition, wxSize( 100,-1 ), m_choice_tilesizeNChoices, m_choice_tilesizeChoices, 0 );
	m_choice_tilesize->SetSelection( 0 );
	fgSizer1->Add( m_choice_tilesize, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_text213 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Filename base: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text213->Wrap( -1 );
	fgSizer1->Add( m_text213, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_tfile_baseChoices[] = { wxT("ChoiceItem") };
	int m_tfile_baseNChoices = sizeof( m_tfile_baseChoices ) / sizeof( wxString );
	m_tfile_base = new wxChoice( TParamsPanel2, ID_TFILE_BASE, wxDefaultPosition, wxSize( 100,-1 ), m_tfile_baseNChoices, m_tfile_baseChoices, 0 );
	m_tfile_base->SetSelection( 0 );
	fgSizer1->Add( m_tfile_base, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5 );
	
	id_text2 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Filename: "), wxDefaultPosition, wxDefaultSize, 0 );
	id_text2->Wrap( -1 );
	fgSizer1->Add( id_text2, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	mmfilename = new wxTextCtrl( TParamsPanel2, ID_TFILENAME, wxEmptyString, wxDefaultPosition, wxSize( 220,-1 ), 0 );
	fgSizer1->Add( mmfilename, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer7->Add( fgSizer1, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	m_tileset = new wxRadioButton( TParamsPanel2, ID_TILESET, wxT("Tileset .ini file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tileset->SetValue( true ); 
	bSizer27->Add( m_tileset, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_tfile_tileset = new wxComboBox( TParamsPanel2, ID_TFILE_TILESET, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	id_tfile_tileset->Append( wxT("Item") );
	bSizer27->Add( id_tfile_tileset, 1, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer7->Add( bSizer27, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer28->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_texture_gradual = new wxCheckBox( TParamsPanel2, ID_TEXTURE_GRADUAL, wxT("Start with minimal texture tiles and load gradually"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texture_gradual->SetValue(true); 
	bSizer28->Add( m_texture_gradual, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer7->Add( bSizer28, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer29->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	id_text3 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Texture LOD factor:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text3->Wrap( -1 );
	bSizer29->Add( id_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tex_lod = new wxTextCtrl( TParamsPanel2, ID_TEX_LOD, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer29->Add( m_tex_lod, 0, wxALIGN_CENTER, 5 );
	
	sbSizer7->Add( bSizer29, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	mmipmap = new wxCheckBox( TParamsPanel2, ID_MIPMAP, wxT("Mipmapping"), wxDefaultPosition, wxDefaultSize, 0 );
	mmipmap->SetValue(true); 
	bSizer31->Add( mmipmap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_16bit = new wxCheckBox( TParamsPanel2, ID_16BIT, wxT("Request 16-bit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_16bit->SetValue(true); 
	bSizer31->Add( m_16bit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer30->Add( bSizer31, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );
	
	m_prelight = new wxCheckBox( TParamsPanel2, ID_PRELIGHT, wxT("Precompute lighting"), wxDefaultPosition, wxDefaultSize, 0 );
	m_prelight->SetValue(true); 
	bSizer32->Add( m_prelight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text9 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Lighting factor: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text9->Wrap( -1 );
	bSizer33->Add( m_text9, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_light_factor = new wxTextCtrl( TParamsPanel2, ID_LIGHT_FACTOR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer33->Add( m_light_factor, 0, wxALIGN_CENTER|wxALL, 0 );
	
	bSizer32->Add( bSizer33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_cast_shadows = new wxCheckBox( TParamsPanel2, ID_CAST_SHADOWS, wxT("Cast shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cast_shadows->SetValue(true); 
	bSizer32->Add( m_cast_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer30->Add( bSizer32, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
	
	sbSizer7->Add( bSizer30, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );
	
	m_retain = new wxCheckBox( TParamsPanel2, ID_RETAIN, wxT("Retain loaded texture in memory for faster re-lighting"), wxDefaultPosition, wxDefaultSize, 0 );
	m_retain->SetValue(true); 
	bSizer34->Add( m_retain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer7->Add( bSizer34, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	bSizer24->Add( sbSizer7, 2, wxALL, 5 );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel2, wxID_ANY, wxT("Detail Texture") ), wxVERTICAL );
	
	m_detailtexture = new wxCheckBox( TParamsPanel2, ID_DETAILTEXTURE, wxT("Show detail texture"), wxDefaultPosition, wxDefaultSize, 0 );
	m_detailtexture->SetValue(true); 
	sbSizer8->Add( m_detailtexture, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_dt_name = new wxComboBox( TParamsPanel2, ID_DT_NAME, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	sbSizer8->Add( m_dt_name, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text10 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Scale"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text10->Wrap( 0 );
	bSizer36->Add( m_text10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_dt_scale = new wxTextCtrl( TParamsPanel2, ID_DT_SCALE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer36->Add( m_dt_scale, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	sbSizer8->Add( bSizer36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text11 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("(meter size of detail texture)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text11->Wrap( -1 );
	sbSizer8->Add( m_text11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text12 = new wxStaticText( TParamsPanel2, ID_TEXT, wxT("Distance"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text12->Wrap( 0 );
	bSizer37->Add( m_text12, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_dt_distance = new wxTextCtrl( TParamsPanel2, ID_DT_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer37->Add( m_dt_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer8->Add( bSizer37, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer35->Add( sbSizer8, 0, wxEXPAND|wxALL, 5 );
	
	bSizer24->Add( bSizer35, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	TParamsPanel2->SetSizer( bSizer24 );
	TParamsPanel2->Layout();
	bSizer24->Fit( TParamsPanel2 );
	m_notebook->AddPage( TParamsPanel2, wxT("Texture"), false );
	TParamsPanel3 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxVERTICAL );
	
	m_plants = new wxCheckBox( TParamsPanel3, ID_PLANTS, wxT("Plants"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plants->SetValue(true); 
	bSizer39->Add( m_plants, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel3, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text13 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Source file: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text13->Wrap( -1 );
	bSizer40->Add( m_text13, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_treefile = new wxComboBox( TParamsPanel3, ID_TREEFILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT ); 
	bSizer40->Add( id_treefile, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer9->Add( bSizer40, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text14 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT(" Visibility distance: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text14->Wrap( -1 );
	bSizer41->Add( m_text14, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	mmegdistance = new wxTextCtrl( TParamsPanel3, ID_VEGDISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer41->Add( mmegdistance, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text15 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text15->Wrap( 0 );
	bSizer41->Add( m_text15, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer9->Add( bSizer41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	bSizer39->Add( sbSizer9, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	id_roads = new wxCheckBox( TParamsPanel3, ID_ROADS, wxT("Roads"), wxDefaultPosition, wxDefaultSize, 0 );
	id_roads->SetValue(true); 
	bSizer39->Add( id_roads, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel3, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text16 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Source file: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text16->Wrap( -1 );
	bSizer42->Add( m_text16, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_roadfile = new wxComboBox( TParamsPanel3, ID_ROADFILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT ); 
	bSizer42->Add( m_roadfile, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer10->Add( bSizer42, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text214 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Ground offset: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text214->Wrap( -1 );
	fgSizer2->Add( m_text214, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_roadheight = new wxTextCtrl( TParamsPanel3, ID_ROADHEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	fgSizer2->Add( m_roadheight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text17 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text17->Wrap( 0 );
	fgSizer2->Add( m_text17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_text4 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Visibility distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text4->Wrap( -1 );
	fgSizer2->Add( id_text4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_roaddistance = new wxTextCtrl( TParamsPanel3, ID_ROADDISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	fgSizer2->Add( m_roaddistance, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text18 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("km"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text18->Wrap( 0 );
	fgSizer2->Add( m_text18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer44->Add( fgSizer2, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_texroads = new wxCheckBox( TParamsPanel3, ID_TEXROADS, wxT("Texture-map roads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texroads->SetValue(true); 
	bSizer44->Add( m_texroads, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_roadculture = new wxCheckBox( TParamsPanel3, ID_ROADCULTURE, wxT("Road culture (stoplights, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_roadculture->SetValue(true); 
	bSizer44->Add( m_roadculture, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer43->Add( bSizer44, 0, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer10->Add( bSizer43, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text215 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Include:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text215->Wrap( 0 );
	bSizer45->Add( m_text215, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_highways = new wxCheckBox( TParamsPanel3, ID_HIGHWAYS, wxT("Highways"), wxDefaultPosition, wxDefaultSize, 0 );
	m_highways->SetValue(true); 
	bSizer45->Add( m_highways, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	id_paved = new wxCheckBox( TParamsPanel3, ID_PAVED, wxT("Paved"), wxDefaultPosition, wxDefaultSize, 0 );
	id_paved->SetValue(true); 
	bSizer45->Add( id_paved, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_dirt = new wxCheckBox( TParamsPanel3, ID_DIRT, wxT("Dirt"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dirt->SetValue(true); 
	bSizer45->Add( m_dirt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	sbSizer10->Add( bSizer45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer39->Add( sbSizer10, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxVERTICAL );
	
	m_text216 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Terrain-specific content:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text216->Wrap( -1 );
	bSizer46->Add( m_text216, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_content_file = new wxComboBox( TParamsPanel3, ID_CONTENT_FILE, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer46->Add( m_content_file, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer39->Add( bSizer46, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer38->Add( bSizer39, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel3, wxID_ANY, wxT("Structure Files") ), wxVERTICAL );
	
	id_structfiles = new wxListBox( TParamsPanel3, ID_STRUCTFILES, wxDefaultPosition, wxSize( 80,110 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer11->Add( id_structfiles, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text19 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Visibility distance: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text19->Wrap( -1 );
	bSizer48->Add( m_text19, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_struct_distance = new wxTextCtrl( TParamsPanel3, ID_STRUCT_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	bSizer48->Add( m_struct_distance, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text20 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text20->Wrap( 0 );
	bSizer48->Add( m_text20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer11->Add( bSizer48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxVERTICAL );
	
	mmheck_structure_shadows = new wxCheckBox( TParamsPanel3, ID_CHECK_STRUCTURE_SHADOWS, wxT("Shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	mmheck_structure_shadows->SetValue(true); 
	bSizer49->Add( mmheck_structure_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer50;
	bSizer50 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer50->Add( 20, 20, 0, wxALIGN_CENTER|wxLEFT, 5 );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer52;
	bSizer52 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text5 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Resolution:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text5->Wrap( -1 );
	bSizer52->Add( id_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString mmhoice_shadow_rezChoices;
	mmhoice_shadow_rez = new wxChoice( TParamsPanel3, ID_CHOICE_SHADOW_REZ, wxDefaultPosition, wxSize( 100,-1 ), mmhoice_shadow_rezChoices, 0 );
	mmhoice_shadow_rez->SetSelection( 0 );
	bSizer52->Add( mmhoice_shadow_rez, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer51->Add( bSizer52, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text21 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Darkness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text21->Wrap( -1 );
	bSizer53->Add( m_text21, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_darkness = new wxTextCtrl( TParamsPanel3, ID_DARKNESS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer53->Add( m_darkness, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer51->Add( bSizer53, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_shadows_default_on = new wxCheckBox( TParamsPanel3, ID_SHADOWS_DEFAULT_ON, wxT("All structures cast shadows by default"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_default_on->SetValue(true); 
	bSizer51->Add( m_shadows_default_on, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_shadows_every_frame = new wxCheckBox( TParamsPanel3, ID_SHADOWS_EVERY_FRAME, wxT("Recompute shadows every frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_every_frame->SetValue(true); 
	bSizer51->Add( m_shadows_every_frame, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxHORIZONTAL );
	
	id_shadow_limit = new wxCheckBox( TParamsPanel3, ID_SHADOW_LIMIT, wxT("Limit shadow area:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_shadow_limit->SetValue(true); 
	bSizer54->Add( id_shadow_limit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_shadow_limit_radius = new wxTextCtrl( TParamsPanel3, ID_SHADOW_LIMIT_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	bSizer54->Add( m_shadow_limit_radius, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text22 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text22->Wrap( 0 );
	bSizer54->Add( m_text22, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer51->Add( bSizer54, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer50->Add( bSizer51, 0, wxALIGN_CENTER, 5 );
	
	bSizer49->Add( bSizer50, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_check_structure_paging = new wxCheckBox( TParamsPanel3, ID_CHECK_STRUCTURE_PAGING, wxT("Paging"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_structure_paging->SetValue(true); 
	bSizer49->Add( m_check_structure_paging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer55->Add( 15, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text23 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Maximum structures:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text23->Wrap( -1 );
	bSizer55->Add( m_text23, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_paging_max_structures = new wxTextCtrl( TParamsPanel3, ID_PAGING_MAX_STRUCTURES, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer55->Add( m_paging_max_structures, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer49->Add( bSizer55, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer56->Add( 15, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text24 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("Page-out distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text24->Wrap( -1 );
	bSizer56->Add( m_text24, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_page_out_distance = new wxTextCtrl( TParamsPanel3, ID_PAGE_OUT_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer56->Add( m_page_out_distance, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text25 = new wxStaticText( TParamsPanel3, ID_TEXT, wxT("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text25->Wrap( 0 );
	bSizer56->Add( m_text25, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer49->Add( bSizer56, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	sbSizer11->Add( bSizer49, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer47->Add( sbSizer11, 0, wxEXPAND|wxALL, 5 );
	
	bSizer38->Add( bSizer47, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
	
	TParamsPanel3->SetSizer( bSizer38 );
	TParamsPanel3->Layout();
	bSizer38->Fit( TParamsPanel3 );
	m_notebook->AddPage( TParamsPanel3, wxT("Culture"), false );
	TParamsPanel4 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer57->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel4, wxID_ANY, wxT("Sky && Water") ), wxVERTICAL );
	
	m_oceanplane = new wxCheckBox( TParamsPanel4, ID_OCEANPLANE, wxT("Ocean plane"), wxDefaultPosition, wxDefaultSize, 0 );
	m_oceanplane->SetValue(true); 
	sbSizer12->Add( m_oceanplane, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer58->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text217 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text217->Wrap( -1 );
	bSizer58->Add( m_text217, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_oceanplaneoffset = new wxTextCtrl( TParamsPanel4, ID_OCEANPLANEOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer58->Add( m_oceanplaneoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer12->Add( bSizer58, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_water = new wxCheckBox( TParamsPanel4, ID_WATER, wxT("Water surface"), wxDefaultPosition, wxDefaultSize, 0 );
	m_water->SetValue(true); 
	sbSizer12->Add( m_water, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_filename_water = new wxComboBox( TParamsPanel4, ID_FILENAME_WATER, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	id_filename_water->Append( wxT("Item") );
	sbSizer12->Add( id_filename_water, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_depressocean = new wxCheckBox( TParamsPanel4, ID_DEPRESSOCEAN, wxT("Depress Ocean"), wxDefaultPosition, wxDefaultSize, 0 );
	m_depressocean->SetValue(true); 
	sbSizer12->Add( m_depressocean, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer59->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text218 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text218->Wrap( -1 );
	bSizer59->Add( m_text218, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_depressoceanoffset = new wxTextCtrl( TParamsPanel4, ID_DEPRESSOCEANOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer59->Add( m_depressoceanoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer12->Add( bSizer59, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	id_sky = new wxCheckBox( TParamsPanel4, ID_SKY, wxT("Sky dome"), wxDefaultPosition, wxDefaultSize, 0 );
	id_sky->SetValue(true); 
	sbSizer12->Add( id_sky, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer60->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_skytexture = new wxComboBox( TParamsPanel4, ID_SKYTEXTURE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer60->Add( m_skytexture, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer12->Add( bSizer60, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	m_horizon = new wxCheckBox( TParamsPanel4, ID_HORIZON, wxT("Artificial Horizon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_horizon->SetValue(true); 
	sbSizer12->Add( m_horizon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	mmog = new wxCheckBox( TParamsPanel4, ID_FOG, wxT("Fog"), wxDefaultPosition, wxDefaultSize, 0 );
	mmog->SetValue(true); 
	bSizer61->Add( mmog, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text26 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Distance (km)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text26->Wrap( -1 );
	bSizer61->Add( m_text26, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_fog_distance = new wxTextCtrl( TParamsPanel4, ID_FOG_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer61->Add( id_fog_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer12->Add( bSizer61, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text27 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Scene background color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text27->Wrap( -1 );
	bSizer62->Add( m_text27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( TParamsPanel4, ID_COLOR3, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer62->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_bgcolor = new wxButton( TParamsPanel4, ID_BGCOLOR, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bgcolor->SetDefault(); 
	bSizer62->Add( m_bgcolor, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer12->Add( bSizer62, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer57->Add( sbSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel4, wxID_ANY, wxT("Time") ), wxVERTICAL );
	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );
	
	m_text219 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Initial Time: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text219->Wrap( -1 );
	bSizer63->Add( m_text219, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_init_time = new wxTextCtrl( TParamsPanel4, ID_TEXT_INIT_TIME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	bSizer63->Add( m_text_init_time, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_set_init_time = new wxButton( TParamsPanel4, ID_SET_INIT_TIME, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	id_set_init_time->SetDefault(); 
	bSizer63->Add( id_set_init_time, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer13->Add( bSizer63, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxHORIZONTAL );
	
	m_timemoves = new wxCheckBox( TParamsPanel4, ID_TIMEMOVES, wxT("Time Moves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_timemoves->SetValue(true); 
	bSizer64->Add( m_timemoves, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer13->Add( bSizer64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer65;
	bSizer65 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text28 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("Faster than real: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text28->Wrap( -1 );
	bSizer65->Add( m_text28, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_timespeed = new wxTextCtrl( TParamsPanel4, ID_TIMESPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer65->Add( m_timespeed, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text29 = new wxStaticText( TParamsPanel4, ID_TEXT, wxT("x"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text29->Wrap( 0 );
	bSizer65->Add( m_text29, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer13->Add( bSizer65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	bSizer57->Add( sbSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer57->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	TParamsPanel4->SetSizer( bSizer57 );
	TParamsPanel4->Layout();
	bSizer57->Fit( TParamsPanel4 );
	m_notebook->AddPage( TParamsPanel4, wxT("Ephemeris"), false );
	TParamsPanel5 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer66->Add( 5, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel5, wxID_ANY, wxT("Abstract Layers") ), wxHORIZONTAL );
	
	mmawfiles = new wxListBox( TParamsPanel5, ID_RAWFILES, wxDefaultPosition, wxSize( 180,120 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer14->Add( mmawfiles, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxVERTICAL );
	
	m_style = new wxButton( TParamsPanel5, ID_STYLE, wxT("Style..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_style->SetDefault(); 
	bSizer67->Add( m_style, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer14->Add( bSizer67, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
	
	bSizer66->Add( sbSizer14, 3, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer15;
	sbSizer15 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel5, wxID_ANY, wxT("Image Layers") ), wxHORIZONTAL );
	
	m_imagefiles = new wxListBox( TParamsPanel5, ID_IMAGEFILES, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer15->Add( m_imagefiles, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer66->Add( sbSizer15, 2, wxEXPAND|wxALL, 5 );
	
	
	bSizer66->Add( 5, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	TParamsPanel5->SetSizer( bSizer66 );
	TParamsPanel5->Layout();
	bSizer66->Fit( TParamsPanel5 );
	m_notebook->AddPage( TParamsPanel5, wxT("Abstracts"), false );
	TParamsPanel6 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer68;
	bSizer68 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer69;
	bSizer69 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel6, wxID_ANY, wxT("Window Overlay") ), wxVERTICAL );
	
	id_text6 = new wxStaticText( TParamsPanel6, ID_TEXT, wxT("Image file:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text6->Wrap( -1 );
	sbSizer16->Add( id_text6, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer70;
	bSizer70 = new wxBoxSizer( wxHORIZONTAL );
	
	m_overlay_file = new wxTextCtrl( TParamsPanel6, ID_OVERLAY_FILE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	bSizer70->Add( m_overlay_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_overlay_dotdotdot = new wxButton( TParamsPanel6, ID_OVERLAY_DOTDOTDOT, wxT("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_overlay_dotdotdot->SetDefault(); 
	bSizer70->Add( m_overlay_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer16->Add( bSizer70, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text30 = new wxStaticText( TParamsPanel6, ID_TEXT, wxT("Placement:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text30->Wrap( 0 );
	bSizer71->Add( m_text30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_overlay_x = new wxTextCtrl( TParamsPanel6, ID_OVERLAY_X, wxEmptyString, wxDefaultPosition, wxSize( 44,-1 ), 0 );
	bSizer71->Add( m_overlay_x, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_overlay_y = new wxTextCtrl( TParamsPanel6, ID_OVERLAY_Y, wxEmptyString, wxDefaultPosition, wxSize( 44,-1 ), 0 );
	bSizer71->Add( m_overlay_y, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer16->Add( bSizer71, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text31 = new wxStaticText( TParamsPanel6, ID_TEXT, wxT("(pixels from the lower-left corner)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text31->Wrap( -1 );
	sbSizer16->Add( m_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer69->Add( sbSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_check_overview = new wxCheckBox( TParamsPanel6, ID_CHECK_OVERVIEW, wxT("Terrain overview map"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_overview->SetValue(true); 
	bSizer69->Add( m_check_overview, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_check_compass = new wxCheckBox( TParamsPanel6, ID_CHECK_COMPASS, wxT("Compass"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_compass->SetValue(true); 
	bSizer69->Add( m_check_compass, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer68->Add( bSizer69, 0, wxALIGN_CENTER|wxALL, 5 );
	
	TParamsPanel6->SetSizer( bSizer68 );
	TParamsPanel6->Layout();
	bSizer68->Fit( TParamsPanel6 );
	m_notebook->AddPage( TParamsPanel6, wxT("HUD"), false );
	TParamsPanel7 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer72->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer17;
	sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel7, wxID_ANY, wxT("Navigation") ), wxVERTICAL );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text32 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Style:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text32->Wrap( 0 );
	bSizer73->Add( m_text32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString mmav_styleChoices[] = { wxT("Item") };
	int mmav_styleNChoices = sizeof( mmav_styleChoices ) / sizeof( wxString );
	mmav_style = new wxChoice( TParamsPanel7, ID_NAV_STYLE, wxDefaultPosition, wxSize( 100,-1 ), mmav_styleNChoices, mmav_styleChoices, 0 );
	mmav_style->SetSelection( 0 );
	bSizer73->Add( mmav_style, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer17->Add( bSizer73, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text33 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Minimum height above ground: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text33->Wrap( -1 );
	bSizer74->Add( m_text33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_minheight = new wxTextCtrl( TParamsPanel7, ID_MINHEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer74->Add( m_minheight, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text34 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text34->Wrap( 0 );
	bSizer74->Add( m_text34, 0, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer17->Add( bSizer74, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text7 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Navigation speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text7->Wrap( -1 );
	bSizer75->Add( id_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_navspeed = new wxTextCtrl( TParamsPanel7, ID_NAVSPEED, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer75->Add( m_navspeed, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_text35 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("meters/frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text35->Wrap( 0 );
	bSizer75->Add( m_text35, 0, wxALIGN_CENTER|wxALL, 0 );
	
	sbSizer17->Add( bSizer75, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxVERTICAL );
	
	m_accel = new wxCheckBox( TParamsPanel7, ID_ACCEL, wxT("Accelerate by height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_accel->SetValue(true); 
	bSizer76->Add( m_accel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_allow_roll = new wxCheckBox( TParamsPanel7, ID_ALLOW_ROLL, wxT("Allow Roll"), wxDefaultPosition, wxDefaultSize, 0 );
	m_allow_roll->SetValue(true); 
	bSizer76->Add( m_allow_roll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	sbSizer17->Add( bSizer76, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text36 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Default Locations File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text36->Wrap( -1 );
	bSizer77->Add( m_text36, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_locfile = new wxComboBox( TParamsPanel7, ID_LOCFILE, wxEmptyString, wxDefaultPosition, wxSize( 140,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT ); 
	bSizer77->Add( m_locfile, 1, wxALIGN_CENTER|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer17->Add( bSizer77, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text37 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Initial Camera Location"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text37->Wrap( -1 );
	bSizer78->Add( m_text37, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_init_locationChoices[] = { wxT("Item") };
	int m_init_locationNChoices = sizeof( m_init_locationChoices ) / sizeof( wxString );
	m_init_location = new wxChoice( TParamsPanel7, ID_INIT_LOCATION, wxDefaultPosition, wxSize( 140,-1 ), m_init_locationNChoices, m_init_locationChoices, 0 );
	m_init_location->SetSelection( 0 );
	bSizer78->Add( m_init_location, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	sbSizer17->Add( bSizer78, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text38 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Near clipping (\"Hither\") distance"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text38->Wrap( -1 );
	bSizer79->Add( m_text38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_hither = new wxTextCtrl( TParamsPanel7, ID_HITHER, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer79->Add( m_hither, 0, wxALIGN_CENTER|wxBOTTOM, 5 );
	
	m_text39 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text39->Wrap( 0 );
	bSizer79->Add( m_text39, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer17->Add( bSizer79, 0, wxALIGN_CENTER, 5 );
	
	m_text220 = new wxStaticText( TParamsPanel7, ID_TEXT, wxT("Animation Paths:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text220->Wrap( -1 );
	sbSizer17->Add( m_text220, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_anim_paths = new wxListBox( TParamsPanel7, ID_ANIM_PATHS, wxDefaultPosition, wxSize( 80,60 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer17->Add( m_anim_paths, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer72->Add( sbSizer17, 0, wxEXPAND|wxALL, 10 );
	
	
	bSizer72->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	TParamsPanel7->SetSizer( bSizer72 );
	TParamsPanel7->Layout();
	bSizer72->Fit( TParamsPanel7 );
	m_notebook->AddPage( TParamsPanel7, wxT("Camera"), false );
	ScenariosPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( ScenariosPanel, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,150 ), 0, NULL, wxLB_SINGLE ); 
	bSizer81->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( ScenariosPanel, ID_NEW_SCENARIO, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	bSizer82->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_delete_scenario = new wxButton( ScenariosPanel, ID_DELETE_SCENARIO, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	id_delete_scenario->SetDefault(); 
	bSizer82->Add( id_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( ScenariosPanel, ID_EDIT_SCENARIO, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	bSizer82->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( ScenariosPanel, ID_MOVEUP_SCENARIO, wxT("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	bSizer82->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( ScenariosPanel, ID_MOVEDOWN_SCENARIO, wxT("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	bSizer82->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer81->Add( bSizer82, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer80->Add( bSizer81, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text40 = new wxStaticText( ScenariosPanel, ID_TEXT, wxT("Start with active scenario:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text40->Wrap( -1 );
	bSizer83->Add( m_text40, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_scenarioChoices[] = { wxT("ChoiceItem") };
	int m_choice_scenarioNChoices = sizeof( m_choice_scenarioChoices ) / sizeof( wxString );
	m_choice_scenario = new wxChoice( ScenariosPanel, ID_CHOICE_SCENARIO, wxDefaultPosition, wxSize( 200,-1 ), m_choice_scenarioNChoices, m_choice_scenarioChoices, 0 );
	m_choice_scenario->SetSelection( 0 );
	bSizer83->Add( m_choice_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer80->Add( bSizer83, 0, wxALIGN_CENTER|wxALL, 0 );
	
	ScenariosPanel->SetSizer( bSizer80 );
	ScenariosPanel->Layout();
	bSizer80->Fit( ScenariosPanel );
	m_notebook->AddPage( ScenariosPanel, wxT("Scenarios"), false );
	
	bSizer13->Add( m_notebook, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer84->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer84->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer13->Add( bSizer84, 0, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	this->SetSizer( bSizer13 );
	this->Layout();
	bSizer13->Fit( this );
	
	this->Centre( wxBOTH );
}

TParamsDlgBase::~TParamsDlgBase()
{
}

TerrManDlgBase::TerrManDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer171;
	bSizer171 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer172;
	bSizer172 = new wxBoxSizer( wxHORIZONTAL );
	
	m_treectrl = new wxTreeCtrl( this, ID_TREECTRL, wxDefaultPosition, wxSize( 380,240 ), wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxSUNKEN_BORDER );
	bSizer172->Add( m_treectrl, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer173;
	bSizer173 = new wxBoxSizer( wxVERTICAL );
	
	m_add_path = new wxButton( this, ID_ADD_PATH, wxT("Add Path"), wxDefaultPosition, wxDefaultSize, 0 );
	m_add_path->SetDefault(); 
	bSizer173->Add( m_add_path, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_add_terrain = new wxButton( this, ID_ADD_TERRAIN, wxT("Add Terrain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_add_terrain->SetDefault(); 
	bSizer173->Add( m_add_terrain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer173->Add( m_line1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_copy = new wxButton( this, ID_COPY, wxT("Add Copy"), wxDefaultPosition, wxDefaultSize, 0 );
	m_copy->SetDefault(); 
	bSizer173->Add( m_copy, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete = new wxButton( this, ID_DELETE, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete->SetDefault(); 
	bSizer173->Add( m_delete, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer173->Add( m_line2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_params = new wxButton( this, ID_EDIT_PARAMS, wxT("Edit Parameters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_params->SetDefault(); 
	bSizer173->Add( m_edit_params, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer173->Add( 20, 10, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer173->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer172->Add( bSizer173, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	bSizer171->Add( bSizer172, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( bSizer171 );
	this->Layout();
	bSizer171->Fit( this );
	
	this->Centre( wxBOTH );
}

TerrManDlgBase::~TerrManDlgBase()
{
}

TimeDlgBase::TimeDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer185;
	bSizer185 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer186;
	bSizer186 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text111 = new wxStaticText( this, ID_TEXT, wxT("Year:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text111->Wrap( 0 );
	bSizer186->Add( m_text111, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_year = new wxSpinCtrl( this, ID_SPIN_YEAR, wxT("2000"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1970, 2038, 2000 );
	bSizer186->Add( m_spin_year, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text112 = new wxStaticText( this, ID_TEXT, wxT("Month"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text112->Wrap( 0 );
	bSizer186->Add( m_text112, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_month = new wxSpinCtrl( this, ID_SPIN_MONTH, wxT("1"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 12, 1 );
	bSizer186->Add( m_spin_month, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text113 = new wxStaticText( this, ID_TEXT, wxT("Day"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text113->Wrap( 0 );
	bSizer186->Add( m_text113, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_day = new wxSpinCtrl( this, ID_SPIN_DAY, wxT("1"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 32, 1 );
	bSizer186->Add( m_spin_day, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer186, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer187;
	bSizer187 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text114 = new wxStaticText( this, ID_TEXT, wxT("Hour:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text114->Wrap( 0 );
	bSizer187->Add( m_text114, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_hour = new wxSpinCtrl( this, ID_SPIN_HOUR, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 24, 0 );
	bSizer187->Add( m_spin_hour, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text115 = new wxStaticText( this, ID_TEXT, wxT("Minute"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text115->Wrap( 0 );
	bSizer187->Add( m_text115, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_minute = new wxSpinCtrl( this, ID_SPIN_MINUTE, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 60, 0 );
	bSizer187->Add( m_spin_minute, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text116 = new wxStaticText( this, ID_TEXT, wxT("Second"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text116->Wrap( 0 );
	bSizer187->Add( m_text116, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_second = new wxSpinCtrl( this, ID_SPIN_SECOND, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 60, 0 );
	bSizer187->Add( m_spin_second, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer187, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer188;
	bSizer188 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text117 = new wxStaticText( this, ID_TEXT, wxT("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text117->Wrap( 0 );
	bSizer188->Add( m_text117, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_speed = new wxTextCtrl( this, ID_TEXT_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer188->Add( m_text_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_speed = new wxSlider( this, ID_SLIDER_SPEED, 0, 0, 100, wxDefaultPosition, wxSize( 140,-1 ), wxSL_HORIZONTAL );
	bSizer188->Add( m_slider_speed, 0, wxALIGN_CENTER, 5 );
	
	m_stop = new wxButton( this, ID_STOP, wxT("Stop"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_stop->SetDefault(); 
	bSizer188->Add( m_stop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer188, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer185 );
	this->Layout();
	bSizer185->Fit( this );
	
	this->Centre( wxBOTH );
}

TimeDlgBase::~TimeDlgBase()
{
}

OptionsDlgBase::OptionsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer189;
	bSizer189 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer190;
	bSizer190 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Display") ), wxVERTICAL );
	
	m_fullscreen = new wxCheckBox( this, ID_FULLSCREEN, wxT("Start in full screen"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fullscreen->SetValue(true); 
	sbSizer36->Add( m_fullscreen, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_stereo = new wxCheckBox( this, ID_STEREO, wxT("Stereo"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stereo->SetValue(true); 
	sbSizer36->Add( m_stereo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer5->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_stereo1 = new wxRadioButton( this, ID_STEREO1, wxT("Anaglyphic (red-blue)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_stereo1->SetValue( true ); 
	fgSizer5->Add( m_stereo1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	fgSizer5->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_stereo2 = new wxRadioButton( this, ID_STEREO2, wxT("Quad buffer (shutter glasses)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stereo2->SetValue( true ); 
	fgSizer5->Add( m_stereo2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer36->Add( fgSizer5, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer190->Add( sbSizer36, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Window size") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text118 = new wxStaticText( this, ID_TEXT, wxT("Window X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text118->Wrap( -1 );
	fgSizer6->Add( m_text118, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_winx = new wxTextCtrl( this, ID_WINX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	fgSizer6->Add( m_winx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text119 = new wxStaticText( this, ID_TEXT, wxT("Window Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text119->Wrap( -1 );
	fgSizer6->Add( m_text119, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_winy = new wxTextCtrl( this, ID_WINY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	fgSizer6->Add( m_winy, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text120 = new wxStaticText( this, ID_TEXT, wxT("Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text120->Wrap( -1 );
	fgSizer6->Add( m_text120, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_win_xsize = new wxTextCtrl( this, ID_WIN_XSIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	fgSizer6->Add( m_win_xsize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text121 = new wxStaticText( this, ID_TEXT, wxT("Height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text121->Wrap( -1 );
	fgSizer6->Add( m_text121, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_win_ysize = new wxTextCtrl( this, ID_WIN_YSIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	fgSizer6->Add( m_win_ysize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer37->Add( fgSizer6, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_size_inside = new wxCheckBox( this, ID_SIZE_INSIDE, wxT("Size is inside (3D view area, not frame)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_size_inside->SetValue(true); 
	sbSizer37->Add( m_size_inside, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer190->Add( sbSizer37, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_texture_compression = new wxCheckBox( this, ID_TEXTURE_COMPRESSION, wxT("Texture compression"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texture_compression->SetValue(true); 
	bSizer190->Add( m_texture_compression, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_disable_mipmaps = new wxCheckBox( this, ID_DISABLE_MIPMAPS, wxT("Disable model mipmaps"), wxDefaultPosition, wxDefaultSize, 0 );
	m_disable_mipmaps->SetValue(true); 
	bSizer190->Add( m_disable_mipmaps, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer38;
	sbSizer38 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Selection") ), wxVERTICAL );
	
	m_direct_picking = new wxCheckBox( this, ID_DIRECT_PICKING, wxT("Use direct picking of object under 2D cursor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_direct_picking->SetValue(true); 
	sbSizer38->Add( m_direct_picking, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text122 = new wxStaticText( this, ID_TEXT, wxT("Selection cutoff distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text122->Wrap( -1 );
	bSizer191->Add( m_text122, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_selection_cutoff = new wxTextCtrl( this, ID_SELECTION_CUTOFF, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer191->Add( m_selection_cutoff, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text123 = new wxStaticText( this, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text123->Wrap( 0 );
	bSizer191->Add( m_text123, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer38->Add( bSizer191, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer192;
	bSizer192 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text124 = new wxStaticText( this, ID_TEXT, wxT("Maximum radius for selecting models:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text124->Wrap( -1 );
	bSizer192->Add( m_text124, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_selection_radius = new wxTextCtrl( this, ID_SELECTION_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer192->Add( m_selection_radius, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text125 = new wxStaticText( this, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text125->Wrap( 0 );
	bSizer192->Add( m_text125, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer38->Add( bSizer192, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer190->Add( sbSizer38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer193;
	bSizer193 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text126 = new wxStaticText( this, ID_TEXT, wxT("Size exaggeration for plants and fences: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text126->Wrap( -1 );
	bSizer193->Add( m_text126, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plantsize = new wxTextCtrl( this, ID_PLANTSIZE, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bSizer193->Add( m_plantsize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer190->Add( bSizer193, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	m_only_available_species = new wxCheckBox( this, ID_ONLY_AVAILABLE_SPECIES, wxT("Show only species with available appearances"), wxDefaultPosition, wxDefaultSize, 0 );
	m_only_available_species->SetValue(true); 
	bSizer190->Add( m_only_available_species, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer194;
	bSizer194 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text127 = new wxStaticText( this, ID_TEXT, wxT("Global content file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text127->Wrap( -1 );
	bSizer194->Add( m_text127, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_contentChoices[] = { wxT("Item") };
	int m_choice_contentNChoices = sizeof( m_choice_contentChoices ) / sizeof( wxString );
	m_choice_content = new wxChoice( this, ID_CHOICE_CONTENT, wxDefaultPosition, wxSize( 200,-1 ), m_choice_contentNChoices, m_choice_contentChoices, 0 );
	m_choice_content->SetSelection( 0 );
	bSizer194->Add( m_choice_content, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer190->Add( bSizer194, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_terrain_progress = new wxCheckBox( this, ID_TERRAIN_PROGRESS, wxT("Show progress dialog during terrain creation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrain_progress->SetValue(true); 
	bSizer190->Add( m_terrain_progress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_fly_in = new wxCheckBox( this, ID_FLY_IN, wxT("Fly in gradually from earth view"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fly_in->SetValue(true); 
	bSizer190->Add( m_fly_in, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer195;
	bSizer195 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer195->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer195->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer190->Add( bSizer195, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer189->Add( bSizer190, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer189 );
	this->Layout();
	bSizer189->Fit( this );
	
	this->Centre( wxBOTH );
}

OptionsDlgBase::~OptionsDlgBase()
{
}

ScenariosPaneBase::ScenariosPaneBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer225;
	bSizer225 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer226;
	bSizer226 = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( this, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,150 ), 0, NULL, wxLB_SINGLE ); 
	bSizer226->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer227;
	bSizer227 = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( this, ID_NEW_SCENARIO, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	bSizer227->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete_scenario = new wxButton( this, ID_DELETE_SCENARIO, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_scenario->SetDefault(); 
	bSizer227->Add( m_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( this, ID_EDIT_SCENARIO, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	bSizer227->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( this, ID_MOVEUP_SCENARIO, wxT("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	bSizer227->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( this, ID_MOVEDOWN_SCENARIO, wxT("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	bSizer227->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer226->Add( bSizer227, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer225->Add( bSizer226, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer228;
	bSizer228 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text147 = new wxStaticText( this, ID_TEXT, wxT("Start with active scenario:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text147->Wrap( -1 );
	bSizer228->Add( m_text147, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_scenarioChoices[] = { wxT("ChoiceItem") };
	int m_choice_scenarioNChoices = sizeof( m_choice_scenarioChoices ) / sizeof( wxString );
	m_choice_scenario = new wxChoice( this, ID_CHOICE_SCENARIO, wxDefaultPosition, wxSize( 200,-1 ), m_choice_scenarioNChoices, m_choice_scenarioChoices, 0 );
	m_choice_scenario->SetSelection( 0 );
	bSizer228->Add( m_choice_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer225->Add( bSizer228, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer225 );
	this->Layout();
	bSizer225->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenariosPaneBase::~ScenariosPaneBase()
{
}

ScenarioVisibleLayersPane::ScenarioVisibleLayersPane( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer229;
	bSizer229 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer230;
	bSizer230 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer43;
	sbSizer43 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Visible Layers") ), wxVERTICAL );
	
	m_scenario_visible_layers = new wxListBox( this, ID_SCENARIO_VISIBLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer43->Add( m_scenario_visible_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer230->Add( sbSizer43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer231;
	bSizer231 = new wxBoxSizer( wxVERTICAL );
	
	m_scenario_add_visible_layer = new wxButton( this, ID_SCENARIO_ADD_VISIBLE_LAYER, wxT("<<"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_add_visible_layer->SetDefault(); 
	bSizer231->Add( m_scenario_add_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_remove_visible_layer = new wxButton( this, ID_SCENARIO_REMOVE_VISIBLE_LAYER, wxT(">>"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_remove_visible_layer->SetDefault(); 
	bSizer231->Add( m_scenario_remove_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer230->Add( bSizer231, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer44;
	sbSizer44 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Available Layers") ), wxVERTICAL );
	
	m_scenario_available_layers = new wxListBox( this, ID_SCENARIO_AVAILABLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer44->Add( m_scenario_available_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer230->Add( sbSizer44, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer229->Add( bSizer230, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer229 );
	this->Layout();
	bSizer229->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenarioVisibleLayersPane::~ScenarioVisibleLayersPane()
{
}

ScenarioParamsDlgBase::ScenarioParamsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer232;
	bSizer232 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer233;
	bSizer233 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text148 = new wxStaticText( this, ID_TEXT, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text148->Wrap( 0 );
	bSizer233->Add( m_text148, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_name = new wxTextCtrl( this, ID_SCENARIO_NAME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer233->Add( m_scenario_name, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer232->Add( bSizer233, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	ScenarioVisibleLayersPane = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer234;
	bSizer234 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer235;
	bSizer235 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer45;
	sbSizer45 = new wxStaticBoxSizer( new wxStaticBox( ScenarioVisibleLayersPane, wxID_ANY, wxT("Visible Layers") ), wxVERTICAL );
	
	m_scenario_visible_layers = new wxListBox( ScenarioVisibleLayersPane, ID_SCENARIO_VISIBLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer45->Add( m_scenario_visible_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer235->Add( sbSizer45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer236;
	bSizer236 = new wxBoxSizer( wxVERTICAL );
	
	m_scenario_add_visible_layer = new wxButton( ScenarioVisibleLayersPane, ID_SCENARIO_ADD_VISIBLE_LAYER, wxT("<<"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_add_visible_layer->SetDefault(); 
	bSizer236->Add( m_scenario_add_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_remove_visible_layer = new wxButton( ScenarioVisibleLayersPane, ID_SCENARIO_REMOVE_VISIBLE_LAYER, wxT(">>"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_remove_visible_layer->SetDefault(); 
	bSizer236->Add( m_scenario_remove_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer235->Add( bSizer236, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer46;
	sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( ScenarioVisibleLayersPane, wxID_ANY, wxT("Available Layers") ), wxVERTICAL );
	
	m_scenario_available_layers = new wxListBox( ScenarioVisibleLayersPane, ID_SCENARIO_AVAILABLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer46->Add( m_scenario_available_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer235->Add( sbSizer46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer234->Add( bSizer235, 0, wxALIGN_CENTER|wxALL, 5 );
	
	ScenarioVisibleLayersPane->SetSizer( bSizer234 );
	ScenarioVisibleLayersPane->Layout();
	bSizer234->Fit( ScenarioVisibleLayersPane );
	m_notebook->AddPage( ScenarioVisibleLayersPane, wxT("Visible Layers"), true );
	
	bSizer232->Add( m_notebook, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer237;
	bSizer237 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer237->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer237->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer232->Add( bSizer237, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer232 );
	this->Layout();
	bSizer232->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenarioParamsDlgBase::~ScenarioParamsDlgBase()
{
}

ScenarioSelectDlgBase::ScenarioSelectDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* ScenarioSizerTop;
	ScenarioSizerTop = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* ScenarioSizerMid;
	ScenarioSizerMid = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( this, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,200 ), 0, NULL, wxLB_SINGLE ); 
	ScenarioSizerMid->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* ScenarioSizerSide;
	ScenarioSizerSide = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( this, ID_NEW_SCENARIO, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete_scenario = new wxButton( this, ID_DELETE_SCENARIO, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( this, ID_EDIT_SCENARIO, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( this, ID_MOVEUP_SCENARIO, wxT("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( this, ID_MOVEDOWN_SCENARIO, wxT("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_previous = new wxButton( this, ID_SCENARIO_PREVIOUS, wxT("Previous"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_previous->SetDefault(); 
	ScenarioSizerSide->Add( m_scenario_previous, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_next = new wxButton( this, ID_SCENARIO_NEXT, wxT("Next"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_next->SetDefault(); 
	ScenarioSizerSide->Add( m_scenario_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	ScenarioSizerMid->Add( ScenarioSizerSide, 0, wxALIGN_CENTER, 5 );
	
	ScenarioSizerTop->Add( ScenarioSizerMid, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer241->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer241->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_apply = new wxButton( this, wxID_APPLY, wxT("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	m_apply->SetDefault(); 
	bSizer241->Add( m_apply, 0, wxALIGN_CENTER|wxALL, 5 );
	
	ScenarioSizerTop->Add( bSizer241, 0, wxALIGN_CENTER, 5 );
	
	this->SetSizer( ScenarioSizerTop );
	this->Layout();
	ScenarioSizerTop->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenarioSelectDlgBase::~ScenarioSelectDlgBase()
{
}

TextureDlgBase::TextureDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer247;
	bSizer247 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer248;
	bSizer248 = new wxBoxSizer( wxHORIZONTAL );
	
	m_single = new wxRadioButton( this, ID_SINGLE, wxT("Single texture "), wxDefaultPosition, wxDefaultSize, 0 );
	m_single->SetValue( true ); 
	bSizer248->Add( m_single, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tfile_single = new wxComboBox( this, ID_TFILE_SINGLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_tfile_single->Append( wxT("Item") );
	bSizer248->Add( m_tfile_single, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer247->Add( bSizer248, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_derived = new wxRadioButton( this, ID_DERIVED, wxT("Derive texture from elevation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_derived->SetValue( true ); 
	bSizer247->Add( m_derived, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer249;
	bSizer249 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer249->Add( 20, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_colorsChoices[] = { wxT("Item") };
	int m_choice_colorsNChoices = sizeof( m_choice_colorsChoices ) / sizeof( wxString );
	m_choice_colors = new wxChoice( this, ID_CHOICE_COLORS, wxDefaultPosition, wxSize( 180,-1 ), m_choice_colorsNChoices, m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	bSizer249->Add( m_choice_colors, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_colors = new wxButton( this, ID_EDIT_COLORS, wxT("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_colors->SetDefault(); 
	bSizer249->Add( m_edit_colors, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer247->Add( bSizer249, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_tiled_4by4 = new wxRadioButton( this, ID_TILED_4BY4, wxT("4x4 Tiled texture"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tiled_4by4->SetValue( true ); 
	bSizer247->Add( m_tiled_4by4, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer7->AddGrowableCol( 1 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text160 = new wxStaticText( this, ID_TEXT, wxT("Tile size: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text160->Wrap( -1 );
	fgSizer7->Add( m_text160, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_choice_tilesizeChoices[] = { wxT("ChoiceItem") };
	int m_choice_tilesizeNChoices = sizeof( m_choice_tilesizeChoices ) / sizeof( wxString );
	m_choice_tilesize = new wxChoice( this, ID_CHOICE_TILESIZE, wxDefaultPosition, wxSize( 100,-1 ), m_choice_tilesizeNChoices, m_choice_tilesizeChoices, 0 );
	m_choice_tilesize->SetSelection( 0 );
	fgSizer7->Add( m_choice_tilesize, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_text161 = new wxStaticText( this, ID_TEXT, wxT("Filename base: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text161->Wrap( -1 );
	fgSizer7->Add( m_text161, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_tfile_baseChoices[] = { wxT("ChoiceItem") };
	int m_tfile_baseNChoices = sizeof( m_tfile_baseChoices ) / sizeof( wxString );
	m_tfile_base = new wxChoice( this, ID_TFILE_BASE, wxDefaultPosition, wxSize( 100,-1 ), m_tfile_baseNChoices, m_tfile_baseChoices, 0 );
	m_tfile_base->SetSelection( 0 );
	fgSizer7->Add( m_tfile_base, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5 );
	
	m_text162 = new wxStaticText( this, ID_TEXT, wxT("Filename: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text162->Wrap( 0 );
	fgSizer7->Add( m_text162, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_tfilename = new wxTextCtrl( this, ID_TFILENAME, wxEmptyString, wxDefaultPosition, wxSize( 220,-1 ), 0 );
	fgSizer7->Add( m_tfilename, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer247->Add( fgSizer7, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer250;
	bSizer250 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer250->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer250->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer247->Add( bSizer250, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	this->SetSizer( bSizer247 );
	this->Layout();
	bSizer247->Fit( this );
	
	this->Centre( wxBOTH );
}

TextureDlgBase::~TextureDlgBase()
{
}

LayerAnimDlgBase::LayerAnimDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer251;
	bSizer251 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer252;
	bSizer252 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer253;
	bSizer253 = new wxBoxSizer( wxVERTICAL );
	
	m_anim_pos = new wxSlider( this, ID_ANIM_POS, 0, 0, 1000, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer253->Add( m_anim_pos, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer254;
	bSizer254 = new wxBoxSizer( wxHORIZONTAL );
	
	m_reset = new wxBitmapButton( this, ID_RESET, wxBitmap( wxT("bitmap/play_back.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_reset->SetDefault(); 
	bSizer254->Add( m_reset, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_stop = new wxBitmapButton( this, ID_STOP, wxBitmap( wxT("bitmap/play_stop.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_stop->SetDefault(); 
	bSizer254->Add( m_stop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_play = new wxBitmapButton( this, ID_PLAY, wxBitmap( wxT("bitmap/play_play.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_play->SetDefault(); 
	bSizer254->Add( m_play, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text163 = new wxStaticText( this, ID_TEXT, wxT("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text163->Wrap( 0 );
	bSizer254->Add( m_text163, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, wxT("2.0"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer254->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text164 = new wxStaticText( this, ID_TEXT, wxT("fps"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text164->Wrap( 0 );
	bSizer254->Add( m_text164, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer253->Add( bSizer254, 0, wxALIGN_CENTER, 5 );
	
	bSizer252->Add( bSizer253, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer251->Add( bSizer252, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	this->SetSizer( bSizer251 );
	this->Layout();
	bSizer251->Fit( this );
	
	this->Centre( wxBOTH );
}

LayerAnimDlgBase::~LayerAnimDlgBase()
{
}

VehicleDlgBase::VehicleDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer255;
	bSizer255 = new wxBoxSizer( wxVERTICAL );
	
	wxString m_choice_vehiclesChoices[] = { wxT("ChoiceItem") };
	int m_choice_vehiclesNChoices = sizeof( m_choice_vehiclesChoices ) / sizeof( wxString );
	m_choice_vehicles = new wxChoice( this, ID_CHOICE_VEHICLES, wxDefaultPosition, wxSize( 100,-1 ), m_choice_vehiclesNChoices, m_choice_vehiclesChoices, 0 );
	m_choice_vehicles->SetSelection( 0 );
	bSizer255->Add( m_choice_vehicles, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer256;
	bSizer256 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text165 = new wxStaticText( this, ID_TEXT, wxT("Body color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text165->Wrap( -1 );
	bSizer256->Add( m_text165, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( this, ID_COLOR3, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer256->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_vehicle_color = new wxButton( this, ID_SET_VEHICLE_COLOR, wxT("Set"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_set_vehicle_color->SetDefault(); 
	bSizer256->Add( m_set_vehicle_color, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer255->Add( bSizer256, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	this->SetSizer( bSizer255 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

VehicleDlgBase::~VehicleDlgBase()
{
}

StyleDlgBase::StyleDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer196;
	bSizer196 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer197;
	bSizer197 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text128 = new wxStaticText( this, ID_TEXT, wxT("Feature type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text128->Wrap( -1 );
	bSizer197->Add( m_text128, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_feature_type = new wxTextCtrl( this, ID_FEATURE_TYPE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	bSizer197->Add( m_feature_type, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer196->Add( bSizer197, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	StylePanel1 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer198;
	bSizer198 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_object_geom = new wxCheckBox( StylePanel1, ID_ENABLE_OBJECT_GEOM, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_object_geom->SetValue(true); 
	bSizer198->Add( m_enable_object_geom, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer39;
	sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( StylePanel1, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer199;
	bSizer199 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text129 = new wxStaticText( StylePanel1, ID_TEXT, wxT("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text129->Wrap( 0 );
	bSizer199->Add( m_text129, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer200;
	bSizer200 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio1 = new wxRadioButton( StylePanel1, ID_RADIO1, wxT("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio1->SetValue( true ); 
	bSizer201->Add( m_radio1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_color = new wxBitmapButton( StylePanel1, ID_OBJECT_GEOM_COLOR, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_object_geom_color->SetDefault(); 
	bSizer201->Add( m_object_geom_color, 0, wxALIGN_CENTER, 5 );
	
	bSizer200->Add( bSizer201, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer202;
	bSizer202 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_object_color_field = new wxRadioButton( StylePanel1, ID_RADIO_USE_OBJECT_COLOR_FIELD, wxT("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_use_object_color_field->SetValue( true ); 
	bSizer202->Add( m_radio_use_object_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_object_color_fieldChoices[] = { wxT("Item") };
	int m_object_color_fieldNChoices = sizeof( m_object_color_fieldChoices ) / sizeof( wxString );
	m_object_color_field = new wxChoice( StylePanel1, ID_OBJECT_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_object_color_fieldNChoices, m_object_color_fieldChoices, 0 );
	m_object_color_field->SetSelection( 0 );
	bSizer202->Add( m_object_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer200->Add( bSizer202, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer199->Add( bSizer200, 1, wxALIGN_CENTER, 5 );
	
	sbSizer39->Add( bSizer199, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer203;
	bSizer203 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text130 = new wxStaticText( StylePanel1, ID_TEXT, wxT("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text130->Wrap( -1 );
	bSizer203->Add( m_text130, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_height = new wxTextCtrl( StylePanel1, ID_OBJECT_GEOM_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer203->Add( m_object_geom_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text131 = new wxStaticText( StylePanel1, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text131->Wrap( 0 );
	bSizer203->Add( m_text131, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer39->Add( bSizer203, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer204;
	bSizer204 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text132 = new wxStaticText( StylePanel1, ID_TEXT, wxT("Size/scale"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text132->Wrap( 0 );
	bSizer204->Add( m_text132, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_size = new wxTextCtrl( StylePanel1, ID_OBJECT_GEOM_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer204->Add( m_object_geom_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text133 = new wxStaticText( StylePanel1, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text133->Wrap( 0 );
	bSizer204->Add( m_text133, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer39->Add( bSizer204, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer198->Add( sbSizer39, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	StylePanel1->SetSizer( bSizer198 );
	StylePanel1->Layout();
	bSizer198->Fit( StylePanel1 );
	m_notebook->AddPage( StylePanel1, wxT("Object Geometry"), false );
	StylePanel2 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer205;
	bSizer205 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_line_geom = new wxCheckBox( StylePanel2, ID_ENABLE_LINE_GEOM, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_line_geom->SetValue(true); 
	bSizer205->Add( m_enable_line_geom, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer40;
	sbSizer40 = new wxStaticBoxSizer( new wxStaticBox( StylePanel2, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer206;
	bSizer206 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text134 = new wxStaticText( StylePanel2, ID_TEXT, wxT("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text134->Wrap( 0 );
	bSizer206->Add( m_text134, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer207;
	bSizer207 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer208;
	bSizer208 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio2 = new wxRadioButton( StylePanel2, ID_RADIO2, wxT("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio2->SetValue( true ); 
	bSizer208->Add( m_radio2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_geom_color = new wxBitmapButton( StylePanel2, ID_LINE_GEOM_COLOR, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_line_geom_color->SetDefault(); 
	bSizer208->Add( m_line_geom_color, 0, wxALIGN_CENTER, 5 );
	
	bSizer207->Add( bSizer208, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer209;
	bSizer209 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_line_color_field = new wxRadioButton( StylePanel2, ID_RADIO_USE_LINE_COLOR_FIELD, wxT("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_use_line_color_field->SetValue( true ); 
	bSizer209->Add( m_radio_use_line_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_line_color_fieldChoices[] = { wxT("Item") };
	int m_line_color_fieldNChoices = sizeof( m_line_color_fieldChoices ) / sizeof( wxString );
	m_line_color_field = new wxChoice( StylePanel2, ID_LINE_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_line_color_fieldNChoices, m_line_color_fieldChoices, 0 );
	m_line_color_field->SetSelection( 0 );
	bSizer209->Add( m_line_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer207->Add( bSizer209, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer206->Add( bSizer207, 1, wxALIGN_CENTER, 5 );
	
	sbSizer40->Add( bSizer206, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer210;
	bSizer210 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text135 = new wxStaticText( StylePanel2, ID_TEXT, wxT("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text135->Wrap( -1 );
	bSizer210->Add( m_text135, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_geom_height = new wxTextCtrl( StylePanel2, ID_LINE_GEOM_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer210->Add( m_line_geom_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text136 = new wxStaticText( StylePanel2, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text136->Wrap( 0 );
	bSizer210->Add( m_text136, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer40->Add( bSizer210, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text137 = new wxStaticText( StylePanel2, ID_TEXT, wxT("Line width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text137->Wrap( -1 );
	bSizer211->Add( m_text137, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_width = new wxTextCtrl( StylePanel2, ID_LINE_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer211->Add( m_line_width, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text138 = new wxStaticText( StylePanel2, ID_TEXT, wxT("pixels"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text138->Wrap( 0 );
	bSizer211->Add( m_text138, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer40->Add( bSizer211, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer212;
	bSizer212 = new wxBoxSizer( wxHORIZONTAL );
	
	m_tessellate = new wxCheckBox( StylePanel2, ID_TESSELLATE, wxT("Tessellate edges"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tessellate->SetValue(true); 
	bSizer212->Add( m_tessellate, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer40->Add( bSizer212, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer205->Add( sbSizer40, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	StylePanel2->SetSizer( bSizer205 );
	StylePanel2->Layout();
	bSizer205->Fit( StylePanel2 );
	m_notebook->AddPage( StylePanel2, wxT("Line Geometry"), true );
	StylePanel3 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer213;
	bSizer213 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_text_labels = new wxCheckBox( StylePanel3, ID_ENABLE_TEXT_LABELS, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_text_labels->SetValue(true); 
	bSizer213->Add( m_enable_text_labels, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( StylePanel3, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer214;
	bSizer214 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text139 = new wxStaticText( StylePanel3, ID_TEXT, wxT("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text139->Wrap( 0 );
	bSizer214->Add( m_text139, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer215;
	bSizer215 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer216;
	bSizer216 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio3 = new wxRadioButton( StylePanel3, ID_RADIO3, wxT("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio3->SetValue( true ); 
	bSizer216->Add( m_radio3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_color = new wxBitmapButton( StylePanel3, ID_TEXT_COLOR, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_text_color->SetDefault(); 
	bSizer216->Add( m_text_color, 0, wxALIGN_CENTER, 5 );
	
	bSizer215->Add( bSizer216, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer217;
	bSizer217 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_text_color_field = new wxRadioButton( StylePanel3, ID_RADIO_USE_TEXT_COLOR_FIELD, wxT("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_use_text_color_field->SetValue( true ); 
	bSizer217->Add( m_radio_use_text_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_text_color_fieldChoices[] = { wxT("Item") };
	int m_text_color_fieldNChoices = sizeof( m_text_color_fieldChoices ) / sizeof( wxString );
	m_text_color_field = new wxChoice( StylePanel3, ID_TEXT_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_text_color_fieldNChoices, m_text_color_fieldChoices, 0 );
	m_text_color_field->SetSelection( 0 );
	bSizer217->Add( m_text_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer215->Add( bSizer217, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer214->Add( bSizer215, 1, wxALIGN_CENTER, 5 );
	
	sbSizer41->Add( bSizer214, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer218;
	bSizer218 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text140 = new wxStaticText( StylePanel3, ID_TEXT, wxT("Text field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text140->Wrap( -1 );
	bSizer218->Add( m_text140, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_text_fieldChoices[] = { wxT("Item") };
	int m_text_fieldNChoices = sizeof( m_text_fieldChoices ) / sizeof( wxString );
	m_text_field = new wxChoice( StylePanel3, ID_TEXT_FIELD, wxDefaultPosition, wxSize( 160,-1 ), m_text_fieldNChoices, m_text_fieldChoices, 0 );
	m_text_field->SetSelection( 0 );
	bSizer218->Add( m_text_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer41->Add( bSizer218, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer219;
	bSizer219 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text141 = new wxStaticText( StylePanel3, ID_TEXT, wxT("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text141->Wrap( -1 );
	bSizer219->Add( m_text141, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_label_height = new wxTextCtrl( StylePanel3, ID_LABEL_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer219->Add( m_label_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text142 = new wxStaticText( StylePanel3, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text142->Wrap( 0 );
	bSizer219->Add( m_text142, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer41->Add( bSizer219, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer220;
	bSizer220 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text143 = new wxStaticText( StylePanel3, ID_TEXT, wxT("Label Size (vertical)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text143->Wrap( -1 );
	bSizer220->Add( m_text143, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_label_size = new wxTextCtrl( StylePanel3, ID_LABEL_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer220->Add( m_label_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text144 = new wxStaticText( StylePanel3, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text144->Wrap( 0 );
	bSizer220->Add( m_text144, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer41->Add( bSizer220, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text145 = new wxStaticText( StylePanel3, ID_TEXT, wxT("Font:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text145->Wrap( 0 );
	bSizer221->Add( m_text145, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_font = new wxTextCtrl( StylePanel3, ID_FONT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer221->Add( m_font, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer41->Add( bSizer221, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer213->Add( sbSizer41, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	StylePanel3->SetSizer( bSizer213 );
	StylePanel3->Layout();
	bSizer213->Fit( StylePanel3 );
	m_notebook->AddPage( StylePanel3, wxT("Text Labels"), false );
	StylePanel4 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer222;
	bSizer222 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_texture_overlay = new wxCheckBox( StylePanel4, ID_ENABLE_TEXTURE_OVERLAY, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_texture_overlay->SetValue(true); 
	bSizer222->Add( m_enable_texture_overlay, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer42;
	sbSizer42 = new wxStaticBoxSizer( new wxStaticBox( StylePanel4, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer223;
	bSizer223 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer223->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text146 = new wxStaticText( StylePanel4, ID_TEXT, wxT("Mode:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text146->Wrap( 0 );
	bSizer223->Add( m_text146, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_texture_modeChoices[] = { wxT("Item") };
	int m_texture_modeNChoices = sizeof( m_texture_modeChoices ) / sizeof( wxString );
	m_texture_mode = new wxChoice( StylePanel4, ID_TEXTURE_MODE, wxDefaultPosition, wxSize( 160,-1 ), m_texture_modeNChoices, m_texture_modeChoices, 0 );
	m_texture_mode->SetSelection( 0 );
	bSizer223->Add( m_texture_mode, 1, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer42->Add( bSizer223, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer222->Add( sbSizer42, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5 );
	
	StylePanel4->SetSizer( bSizer222 );
	StylePanel4->Layout();
	bSizer222->Fit( StylePanel4 );
	m_notebook->AddPage( StylePanel4, wxT("Texture Overlay"), false );
	
	bSizer196->Add( m_notebook, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer224;
	bSizer224 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer224->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancell = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancell->SetDefault(); 
	bSizer224->Add( m_cancell, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer196->Add( bSizer224, 0, wxALIGN_CENTER, 5 );
	
	this->SetSizer( bSizer196 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

StyleDlgBase::~StyleDlgBase()
{
}

PagingDlgBase::PagingDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer285;
	bSizer285 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	SurfaceLODPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer286;
	bSizer286 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( SurfaceLODPanel, wxID_ANY, wxT("Detail levels chart") ), wxVERTICAL );
	
	m_panel1 = new wxPanel( SurfaceLODPanel, ID_PANEL1, wxDefaultPosition, wxSize( 300,40 ), wxTAB_TRAVERSAL );
	sbSizer54->Add( m_panel1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer286->Add( sbSizer54, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer287;
	bSizer287 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text187 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("Target Count"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text187->Wrap( -1 );
	bSizer287->Add( m_text187, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_target = new wxSpinCtrl( SurfaceLODPanel, ID_TARGET, wxT("1000"), wxDefaultPosition, wxSize( 100,-1 ), wxSP_ARROW_KEYS, 1000, 150000, 1000 );
	bSizer287->Add( m_target, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text188 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text188->Wrap( 0 );
	bSizer287->Add( m_text188, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_current = new wxTextCtrl( SurfaceLODPanel, ID_CURRENT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer287->Add( m_current, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer286->Add( bSizer287, 0, wxALIGN_CENTER, 5 );
	
	wxStaticBoxSizer* sbSizer55;
	sbSizer55 = new wxStaticBoxSizer( new wxStaticBox( SurfaceLODPanel, wxID_ANY, wxT("Tileset Texture Status") ), wxVERTICAL );
	
	wxBoxSizer* bSizer288;
	bSizer288 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer289;
	bSizer289 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text189 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("prange"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text189->Wrap( 0 );
	bSizer289->Add( m_text189, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_prange = new wxSlider( SurfaceLODPanel, ID_SLIDER_PRANGE, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer289->Add( m_slider_prange, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_text_prange = new wxTextCtrl( SurfaceLODPanel, ID_TEXT_PRANGE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer289->Add( m_text_prange, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer288->Add( bSizer289, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_panel2 = new wxPanel( SurfaceLODPanel, ID_PANEL2, wxDefaultPosition, wxSize( 300,300 ), wxTAB_TRAVERSAL );
	bSizer288->Add( m_panel2, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer290;
	bSizer290 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticbitmap1 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap1, 0, wxALIGN_CENTER, 0 );
	
	m_text190 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("16"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text190->Wrap( 0 );
	bSizer290->Add( m_text190, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap2 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap2, 0, wxALIGN_CENTER, 5 );
	
	m_text191 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("32"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text191->Wrap( 0 );
	bSizer290->Add( m_text191, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap3 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap3, 0, wxALIGN_CENTER, 5 );
	
	m_text192 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("64"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text192->Wrap( 0 );
	bSizer290->Add( m_text192, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap4 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap4, 0, wxALIGN_CENTER, 5 );
	
	m_text193 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("128"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text193->Wrap( 0 );
	bSizer290->Add( m_text193, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap5 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap5, 0, wxALIGN_CENTER, 5 );
	
	m_text194 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("256"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text194->Wrap( 0 );
	bSizer290->Add( m_text194, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap6 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap6, 0, wxALIGN_CENTER, 5 );
	
	m_text195 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("512"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text195->Wrap( 0 );
	bSizer290->Add( m_text195, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap7 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap7, 0, wxALIGN_CENTER, 5 );
	
	m_text196 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("1k"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text196->Wrap( 0 );
	bSizer290->Add( m_text196, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap8 = new wxStaticBitmap( SurfaceLODPanel, wxID_ANY, wxBitmap( wxT("bitmap/dummy14.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap8, 0, wxALIGN_CENTER, 5 );
	
	m_text197 = new wxStaticText( SurfaceLODPanel, ID_TEXT, wxT("2k"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text197->Wrap( 0 );
	bSizer290->Add( m_text197, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	bSizer288->Add( bSizer290, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tile_status = new wxTextCtrl( SurfaceLODPanel, ID_TILE_STATUS, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer288->Add( m_tile_status, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	sbSizer55->Add( bSizer288, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer286->Add( sbSizer55, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SurfaceLODPanel->SetSizer( bSizer286 );
	SurfaceLODPanel->Layout();
	bSizer286->Fit( SurfaceLODPanel );
	m_notebook->AddPage( SurfaceLODPanel, wxT("Surface and Textures"), true );
	StructureLODPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer56;
	sbSizer56 = new wxStaticBoxSizer( new wxStaticBox( StructureLODPanel, wxID_ANY, wxT("Structure Paging Status") ), wxVERTICAL );
	
	wxBoxSizer* bSizer291;
	bSizer291 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text198 = new wxStaticText( StructureLODPanel, ID_TEXT, wxT("Page-out distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text198->Wrap( -1 );
	bSizer291->Add( m_text198, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_pageout = new wxTextCtrl( StructureLODPanel, ID_TEXT_PAGEOUT, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	bSizer291->Add( m_text_pageout, 0, wxALIGN_CENTER, 5 );
	
	m_slider_pageout = new wxSlider( StructureLODPanel, ID_SLIDER_PAGEOUT, 0, 0, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer291->Add( m_slider_pageout, 1, wxALIGN_CENTER, 0 );
	
	sbSizer56->Add( bSizer291, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer292;
	bSizer292 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text199 = new wxStaticText( StructureLODPanel, ID_TEXT, wxT("Currently constructed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text199->Wrap( -1 );
	bSizer292->Add( m_text199, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_count_current = new wxTextCtrl( StructureLODPanel, ID_COUNT_CURRENT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer292->Add( m_count_current, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text200 = new wxStaticText( StructureLODPanel, ID_TEXT, wxT("/"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text200->Wrap( 0 );
	bSizer292->Add( m_text200, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_count_maximum = new wxTextCtrl( StructureLODPanel, ID_COUNT_MAXIMUM, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer292->Add( m_count_maximum, 0, wxALIGN_CENTER|wxALL, 5 );
	
	sbSizer56->Add( bSizer292, 0, wxALIGN_CENTER, 5 );
	
	m_panel3 = new wxPanel( StructureLODPanel, ID_PANEL3, wxDefaultPosition, wxSize( 300,300 ), wxTAB_TRAVERSAL );
	sbSizer56->Add( m_panel3, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	StructureLODPanel->SetSizer( sbSizer56 );
	StructureLODPanel->Layout();
	sbSizer56->Fit( StructureLODPanel );
	m_notebook->AddPage( StructureLODPanel, wxT("Structures"), false );
	
	bSizer285->Add( m_notebook, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( bSizer285 );
	this->Layout();
	bSizer285->Fit( this );
	
	this->Centre( wxBOTH );
}

PagingDlgBase::~PagingDlgBase()
{
}

EphemDlgBase::EphemDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer297;
	bSizer297 = new wxBoxSizer( wxVERTICAL );
	
	m_oceanplane = new wxCheckBox( this, ID_OCEANPLANE, wxT("Ocean plane"), wxDefaultPosition, wxDefaultSize, 0 );
	m_oceanplane->SetValue(true); 
	bSizer297->Add( m_oceanplane, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer298;
	bSizer298 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer298->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text204 = new wxStaticText( this, ID_TEXT, wxT("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text204->Wrap( -1 );
	bSizer298->Add( m_text204, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_oceanplaneoffset = new wxTextCtrl( this, ID_OCEANPLANEOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer298->Add( m_oceanplaneoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer297->Add( bSizer298, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_sky = new wxCheckBox( this, ID_SKY, wxT("Sky dome"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sky->SetValue(true); 
	bSizer297->Add( m_sky, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer299;
	bSizer299 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer299->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_skytexture = new wxComboBox( this, ID_SKYTEXTURE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer299->Add( m_skytexture, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	bSizer297->Add( bSizer299, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	m_horizon = new wxCheckBox( this, ID_HORIZON, wxT("Artificial Horizon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_horizon->SetValue(true); 
	bSizer297->Add( m_horizon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer300;
	bSizer300 = new wxBoxSizer( wxHORIZONTAL );
	
	m_fog = new wxCheckBox( this, ID_FOG, wxT("Fog"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fog->SetValue(true); 
	bSizer300->Add( m_fog, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text205 = new wxStaticText( this, ID_TEXT, wxT("Distance (m)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text205->Wrap( -1 );
	bSizer300->Add( m_text205, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fog_distance = new wxTextCtrl( this, ID_FOG_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer300->Add( m_fog_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_fog_distance = new wxSlider( this, ID_SLIDER_FOG_DISTANCE, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer300->Add( m_slider_fog_distance, 0, wxALIGN_CENTER|wxTOP, 5 );
	
	bSizer297->Add( bSizer300, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer301;
	bSizer301 = new wxBoxSizer( wxHORIZONTAL );
	
	m_shadows = new wxCheckBox( this, ID_SHADOWS, wxT("Shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows->SetValue(true); 
	bSizer301->Add( m_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer297->Add( bSizer301, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer302;
	bSizer302 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer302->Add( 20, 20, 0, wxALIGN_CENTER|wxLEFT, 5 );
	
	wxBoxSizer* bSizer303;
	bSizer303 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer304;
	bSizer304 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text206 = new wxStaticText( this, ID_TEXT, wxT("Darkness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text206->Wrap( 0 );
	bSizer304->Add( m_text206, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_ambient_bias = new wxTextCtrl( this, ID_AMBIENT_BIAS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer304->Add( m_ambient_bias, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_slider_ambient_bias = new wxSlider( this, ID_SLIDER_AMBIENT_BIAS, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer304->Add( m_slider_ambient_bias, 0, wxALIGN_CENTER, 5 );
	
	bSizer303->Add( bSizer304, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_shadows_every_frame = new wxCheckBox( this, ID_SHADOWS_EVERY_FRAME, wxT("Recompute shadows every frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_every_frame->SetValue(true); 
	bSizer303->Add( m_shadows_every_frame, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer305;
	bSizer305 = new wxBoxSizer( wxHORIZONTAL );
	
	m_shadow_limit = new wxCheckBox( this, ID_SHADOW_LIMIT, wxT("Limit shadow area:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadow_limit->SetValue(true); 
	bSizer305->Add( m_shadow_limit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_shadow_limit_radius = new wxTextCtrl( this, ID_SHADOW_LIMIT_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	bSizer305->Add( m_shadow_limit_radius, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text222 = new wxStaticText( this, ID_TEXT, wxT("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text222->Wrap( 0 );
	bSizer305->Add( m_text222, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer303->Add( bSizer305, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer302->Add( bSizer303, 0, wxALIGN_CENTER, 5 );
	
	bSizer297->Add( bSizer302, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer306;
	bSizer306 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text207 = new wxStaticText( this, ID_TEXT, wxT("Scene background color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text207->Wrap( -1 );
	bSizer306->Add( m_text207, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( this, ID_COLOR3, wxBitmap( wxT("bitmap/dummy_32x18.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer306->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_bgcolor = new wxButton( this, ID_BGCOLOR, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bgcolor->SetDefault(); 
	bSizer306->Add( m_bgcolor, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer297->Add( bSizer306, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text208 = new wxStaticText( this, ID_TEXT, wxT("Wind Direction:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text208->Wrap( -1 );
	fgSizer8->Add( m_text208, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_wind_direction = new wxTextCtrl( this, ID_TEXT_WIND_DIRECTION, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	fgSizer8->Add( m_text_wind_direction, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_slider_wind_direction = new wxSlider( this, ID_SLIDER_WIND_DIRECTION, 0, 0, 150, wxDefaultPosition, wxSize( 150,-1 ), wxSL_HORIZONTAL );
	fgSizer8->Add( m_slider_wind_direction, 0, wxALIGN_CENTER|wxTOP, 5 );
	
	m_text209 = new wxStaticText( this, ID_TEXT, wxT("Wind Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text209->Wrap( -1 );
	fgSizer8->Add( m_text209, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_wind_speed = new wxTextCtrl( this, ID_TEXT_WIND_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	fgSizer8->Add( m_text_wind_speed, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_slider_wind_speed = new wxSlider( this, ID_SLIDER_WIND_SPEED, 0, 0, 150, wxDefaultPosition, wxSize( 150,-1 ), wxSL_HORIZONTAL );
	fgSizer8->Add( m_slider_wind_speed, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	bSizer297->Add( fgSizer8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer297 );
	this->Layout();
	bSizer297->Fit( this );
	
	this->Centre( wxBOTH );
}

EphemDlgBase::~EphemDlgBase()
{
}

ContourDlgBase::ContourDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer307;
	bSizer307 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer308;
	bSizer308 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text210 = new wxStaticText( this, ID_TEXT, wxT("Contour at elevation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text210->Wrap( -1 );
	bSizer308->Add( m_text210, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_elev = new wxTextCtrl( this, ID_ELEV, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer308->Add( m_elev, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text211 = new wxStaticText( this, ID_TEXT, wxT("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text211->Wrap( 0 );
	bSizer308->Add( m_text211, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer307->Add( bSizer308, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer309;
	bSizer309 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_create = new wxRadioButton( this, ID_RADIO_CREATE, wxT("Create new layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create->SetValue( true ); 
	bSizer309->Add( m_radio_create, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer307->Add( bSizer309, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer310;
	bSizer310 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_add = new wxRadioButton( this, ID_RADIO_ADD, wxT("Add to layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_add->SetValue( true ); 
	bSizer310->Add( m_radio_add, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_layerChoices[] = { wxT("ChoiceItem") };
	int m_choice_layerNChoices = sizeof( m_choice_layerChoices ) / sizeof( wxString );
	m_choice_layer = new wxChoice( this, ID_CHOICE_LAYER, wxDefaultPosition, wxSize( 100,-1 ), m_choice_layerNChoices, m_choice_layerChoices, 0 );
	m_choice_layer->SetSelection( 0 );
	bSizer310->Add( m_choice_layer, 1, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer307->Add( bSizer310, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer311;
	bSizer311 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer311->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer311->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer307->Add( bSizer311, 0, wxALIGN_CENTER, 5 );
	
	this->SetSizer( bSizer307 );
	this->Layout();
	bSizer307->Fit( this );
	
	this->Centre( wxBOTH );
}

ContourDlgBase::~ContourDlgBase()
{
}

PerformanceMonitorDlgBase::PerformanceMonitorDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer312;
	bSizer312 = new wxBoxSizer( wxVERTICAL );
	
	m_pm_listctrl = new wxListCtrl( this, ID_PM_LISTCTRL, wxDefaultPosition, wxSize( 800,400 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer312->Add( m_pm_listctrl, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text212 = new wxStaticText( this, ID_TEXT, wxT("Right click on counter to enable/disable"), wxDefaultPosition, wxSize( 300,-1 ), 0 );
	m_text212->Wrap( -1 );
	bSizer312->Add( m_text212, 0, wxALIGN_CENTER|wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	this->SetSizer( bSizer312 );
	this->Layout();
	bSizer312->Fit( this );
	
	// Connect Events
	m_pm_listctrl->Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( PerformanceMonitorDlgBase::OnListItemRightClick ), NULL, this );
}

PerformanceMonitorDlgBase::~PerformanceMonitorDlgBase()
{
	// Disconnect Events
	m_pm_listctrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( PerformanceMonitorDlgBase::OnListItemRightClick ), NULL, this );
	
}
