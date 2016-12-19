/////////////////////////////////////////////////////////////////////////////
// Name:        expresseur.cpp
// Purpose:     expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     19/03/2015
// update : 06/12/2016 18:00
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/toolbar.h"
#include "wx/log.h"
#include "wx/image.h"
#include "wx/filedlg.h"
#include "wx/colordlg.h"
#include "wx/srchctrl.h"
#include "wx/textfile.h"
#include "wx/ffile.h"
#include "wx/filename.h"
#include "wx/filehistory.h"
#include "wx/tglbtn.h"
#include "wx/config.h"
#include "wx/listctrl.h"
#include "wx/filepicker.h"
#include "wx/msgdlg.h"
#include "wx/scrolbar.h"
#include "wx/choicdlg.h"
#include "wx/xml/xml.h"
#include "wx/dynarray.h"
#include "wx/sstream.h"
#include "wx/protocol/http.h"
#include "wx/wizard.h"
#include "wx/clipbrd.h"
#include "wx/stdpaths.h"
#include "wx/dir.h"
#include "wx/dynlib.h"
#include "MNL.h"

#include "global.h"
#include "luabass.h"
#include "basslua.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "mixer.h"
#include "editshortcut.h"
#include "midishortcut.h"
#include "expression.h"
#include "logerror.h"
#include "emptyscore.h"
#include "bitmapscore.h"
#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"
#include "textscore.h"
#include "luafile.h"
#include "expresseur.h"

// define this to use XPMs everywhere (by default, BMPs are used under Win)
// BMPs use less space, but aren't compiled into the executable on other platforms
#ifdef __WINDOWS__
    #define USE_XPM_BITMAPS 1
#else
    #define USE_XPM_BITMAPS 1
#endif


#ifndef wxHAS_IMAGES_IN_RESOURCES
#endif

/*
#if USE_XPM_BITMAPS
    #include "bitmaps/new.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/preview.xpm"  // paste XPM
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS
*/

// Define a new application
class MyApp : public wxApp
{
public:
    bool OnInit();
};

#define periodTimer 50
#define tLongWait 500

enum
{
    // toolbar menu items
	ID_MAIN_LIST_NEW = ID_MAIN,
	ID_MAIN_LIST_OPEN,
	ID_MAIN_LIST_SAVE,
	ID_MAIN_LIST_SAVEAS,
	ID_MAIN_LIST_ADD,
	ID_MAIN_LIST_REMOVE,
	ID_MAIN_LIST_UP,
	ID_MAIN_LIST_DOWN,

	ID_MAIN_ORNAMENT_ADD_ABSOLUTE,
	ID_MAIN_ORNAMENT_ADD_RELATIVE,
	ID_MAIN_ZOOM,
	ID_MAIN_UNZOOM,

	ID_MAIN_LIST_PREVIOUS_FILE,
	ID_MAIN_LIST_NEXT_FILE,

	ID_MAIN_TEXT_SONG ,

	ID_MAIN_SCROLL_HORIZONTAL,
	ID_MAIN_SCROLL_VERTICAL,

	ID_MAIN_MIXER,
	ID_MAIN_GOTO,
	ID_MAIN_MIDISHORTCUT,
	ID_MAIN_EXPRESSION,
	ID_MAIN_LUAFILE,
	ID_MAIN_SETTING_OPEN,
	ID_MAIN_SETTING_SAVE,
	ID_MAIN_SETTING_SAVEAS,
	ID_MAIN_RESET,
	ID_MAIN_LOG,
	ID_MAIN_UPDATE,

	ID_MAIN_TIMER,

	ID_MAIN_TEST,

	ID_MAIN_LOCAL_OFF,
	ID_MAIN_AUDIO_SETTING,

	ID_MAIN_LIST_FILE = ID_MAIN + 100,  // large range for the files in the list
	ID_MAIN_LIST_FILE_END = ID_MAIN_LIST_FILE + 99,  // large range for the files in the list
	ID_MAIN_ACTION = ID_MAIN_LIST_FILE_END + 1, // large range for actions
	ID_MAIN_ACTION_END = ID_MAIN_ACTION + 99 // large range for actions
};

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(Expresseur, wxFrame)
EVT_SIZE(Expresseur::OnSize)

EVT_MENU(ID_MAIN_TEST, Expresseur::OnTest)

EVT_MENU(wxID_NEW, Expresseur::OnNew)
EVT_MENU(wxID_OPEN, Expresseur::OnOpen)
EVT_MENU(wxID_SAVE, Expresseur::OnSave)
EVT_MENU(wxID_SAVEAS, Expresseur::OnSaveas)
EVT_MENU(wxID_EXIT, Expresseur::OnExit)
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, Expresseur::OnRecentFile)

EVT_MENU(wxID_UNDO, Expresseur::OnUndo)
EVT_MENU(wxID_REDO, Expresseur::OnRedo)
EVT_MENU(wxID_COPY, Expresseur::OnCopy)
EVT_MENU(wxID_CUT, Expresseur::OnCut)
EVT_MENU(wxID_PASTE, Expresseur::OnPaste)
EVT_MENU(ID_MAIN_ZOOM, Expresseur::OnZoom)
EVT_MENU(ID_MAIN_UNZOOM, Expresseur::OnUnzoom)
EVT_MENU(ID_MAIN_ORNAMENT_ADD_RELATIVE, Expresseur::OnOrnamentAddRelative)
EVT_MENU(ID_MAIN_ORNAMENT_ADD_ABSOLUTE, Expresseur::OnOrnamentAddAbsolute)

EVT_MENU(wxID_EDIT, Expresseur::OnEdit)
EVT_MENU(ID_MAIN_LOCAL_OFF, Expresseur::OnLocaloff)
EVT_MENU(ID_MAIN_AUDIO_SETTING, Expresseur::OnAudioSetting)

EVT_MENU(ID_MAIN_LIST_PREVIOUS_FILE, Expresseur::OnListPreviousFile)
EVT_MENU(ID_MAIN_LIST_NEXT_FILE, Expresseur::OnListNextFile)
EVT_MENU(ID_MAIN_LIST_NEW, Expresseur::OnListNew)
EVT_MENU(ID_MAIN_LIST_OPEN, Expresseur::OnListOpen)
EVT_MENU(ID_MAIN_LIST_SAVE, Expresseur::OnListSave)
EVT_MENU(ID_MAIN_LIST_SAVEAS, Expresseur::OnListSaveas)
EVT_MENU(ID_MAIN_LIST_ADD, Expresseur::OnListAdd)
EVT_MENU(ID_MAIN_LIST_REMOVE, Expresseur::OnListRemove)
EVT_MENU(ID_MAIN_LIST_UP, Expresseur::OnListUp)
EVT_MENU(ID_MAIN_LIST_DOWN, Expresseur::OnListDown)
EVT_MENU_RANGE(ID_MAIN_LIST_FILE, ID_MAIN_LIST_FILE_END, Expresseur::OnListFile)

EVT_MENU_RANGE(ID_MAIN_ACTION, ID_MAIN_ACTION_END, Expresseur::OnMenuAction)

EVT_MENU(ID_MAIN_MIXER, Expresseur::OnMixer)
EVT_MENU(ID_MAIN_GOTO, Expresseur::OnGoto)
EVT_MENU(ID_MAIN_MIDISHORTCUT, Expresseur::OnMidishortcut)
EVT_MENU(ID_MAIN_EXPRESSION, Expresseur::OnExpression)
EVT_MENU(ID_MAIN_LUAFILE, Expresseur::OnLuafile)
EVT_MENU(ID_MAIN_RESET, Expresseur::OnReset)
EVT_MENU(ID_MAIN_LOG, Expresseur::OnLog)
EVT_MENU(ID_MAIN_SETTING_OPEN, Expresseur::OnSettingOpen)
EVT_MENU(ID_MAIN_SETTING_SAVE, Expresseur::OnSettingSave)
EVT_MENU(ID_MAIN_SETTING_SAVEAS, Expresseur::OnSettingSaveas)

EVT_MENU(wxID_ABOUT, Expresseur::OnAbout)
EVT_MENU(wxID_HELP, Expresseur::OnHelp)
EVT_MENU(ID_MAIN_UPDATE, Expresseur::OnUpdate)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MAIN_SCROLL_HORIZONTAL, Expresseur::OnHorizontalScroll)
EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MAIN_SCROLL_VERTICAL, Expresseur::OnVerticalScroll)

EVT_TEXT(ID_MAIN_TEXT_SONG, Expresseur::OnTextChange)

EVT_TIMER(ID_MAIN_TIMER, Expresseur::OnTimer)


wxEND_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------

IMPLEMENT_APP(MyApp)

Expresseur *frame = NULL;
wxRect rectFrame;

// The `main program' equivalent, creating the windows and returning the
// main frame
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	SetAppName(APP_NAME);

    // Create the main frame window
	frame = new Expresseur(NULL, wxID_ANY, APP_NAME, wxPoint(0, 0), wxSize(500, 400), wxDEFAULT_FRAME_STYLE);

	// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
	frame->SetIcon(wxIcon("MAINICON"));
#endif

	frame->timerTask(true,true);
	if (rectFrame.GetWidth() < 600)
		rectFrame.SetWidth(600);
	if (rectFrame.GetHeight() < 400)
		rectFrame.SetHeight(400);
	frame->SetSize(rectFrame);
	frame->CenterOnScreen();
	frame->Show(true);

#if wxUSE_STATUSBAR
    frame->SetStatusText(wxT(""));
#endif

    wxInitAllImageHandlers();

    return true;
}


Expresseur::Expresseur(wxFrame* parent,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style)
 :wxFrame(parent, id, title, pos, size, style)
{
#if wxUSE_STATUSBAR
    // Give it a status line
	CreateStatusBar(2);
#endif

	onstart = true;
	mode = modeNil;
	listChanged = false;
	listName.Clear();
	typeViewer = EMPTYVIEWER;
	listSelection = -1;
	leftDown = false;
	waitBeforeToCompile = 2000 / periodTimer;
	fileName.Clear();
	mViewerscore = NULL;
	mTextscore = NULL;
	mMixer = NULL;
	mExpression = NULL;
	mLog = NULL;
	mMidishortcut = NULL;
	for (int i = 0; i < MAX_KEYS; i++)
		ckeys[i] = 0;

	// configuration object ( with memory of all parameters
	mConf = new mxconf();

	// check if it the first use ( for intialization, wizard, .. )
	initFirstUse();

	// check if the last "close expreseur" exits normally
	end_ok = mConf->get(CONFIG_END_OK, true);
	if (!end_ok)
	{
		wxMessageBox("\
					 Expresseur has not stopped correctly...\n\
					 Please try to correct the tuning ( score, mixer devices, ... ).\n\
					 Then select menu 'Setting/reset' to restart the system.\n\
					 Sorry for this issue ...\n", "Bug...");
	}
	mConf->set(CONFIG_END_OK, false);

	// load the DLL for the score display
	wxFileName fdll;
	fdll.AssignCwd();
	fdll.AppendDir("MNL folder");
	fdll.SetFullName("MusicNotationLibrary I.dll");
	wxString sdll = fdll.GetFullPath();
	musicxmlDll.Load(sdll);
	if (musicxmlDll.IsLoaded() == false)
	{
		wxString serr;
		serr.Printf("error LoadLibrary '%s'", sdll);
		wxMessageBox(serr, "error library for score notation");
	}
	else
	{
		bool retSuccess = false ;
		MNLLoad = (MNLLoadProc *)(musicxmlDll.GetSymbol( "MNLLoad" , &retSuccess )); 
		//MNLLoad = (MNLLoadProc *)GetProcAddress(musicxmlDll, "MNLLoad");
		if ((MNLLoad == NULL) || (retSuccess == false ))
		{
			wxMessageBox("error GetProcAddress MNLLoad", "error library for score notation");
			musicxmlDll.Unload();
		}
		else
		{
			char chmnlfolder[] = "MNL folder" ;
			char mnlCode[] = "8FF2B7A1BD5ACCA4" ;
			long retcode = MNLLoad(chmnlfolder, mnlCode);
			if (retcode != 0)
			{
				wxMessageBox("error MNLLoad MNL folder", "error library for score notation");
				MNLRelease();
				musicxmlDll.Unload();
			}
			else
			{
				MNLRelease = (MNLReleaseProc *)(musicxmlDll.GetSymbol( "MNLRelease"));
			}
		}
	}
	
	// set the menus
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_NEW, _("New"));
	fileMenu->Append(wxID_OPEN, _("Open..."));
	fileMenu->Append(wxID_SAVE, _("Save\tCtrl+S"));
	fileMenu->Append(wxID_SAVEAS, _("Save as..."));
	fileMenu->AppendSeparator();
	wxMenu* menuRecent = new wxMenu;
	fileMenu->AppendSubMenu(menuRecent, "Open Recent");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, wxT("Exit\tCtrl+Q"));
	fileHistory = new wxFileHistory();
	fileHistory->UseMenu(menuRecent);
	fileHistory->AddFilesToMenu(menuRecent);
	fileHistory->Load(*mConf->getConfig());

	editMenu = new wxMenu;
	editMenu->Append(wxID_UNDO, _("Undo\tCtrl+Z"));
	editMenu->Append(wxID_REDO, _("Redo"));
	editMenu->AppendSeparator();
	editMenu->Append(wxID_COPY, _("Copy\tCtrl+C"));
	editMenu->Append(wxID_CUT, _("Cut\tCtrl+X"));
	editMenu->Append(wxID_PASTE, _("Paste\tCtrl+V"));
	editMenu->AppendSeparator();
	menuEditMode = editMenu->AppendCheckItem(wxID_EDIT, _("Edit\tCtrl+E"), _("Allow text change. Else, keystrokes are used for shortcuts"));
	editMenu->Append(ID_MAIN_GOTO, _("Goto...\tCTRL+G"),_("Goto a meausure in the score"));
	editMenu->Append(ID_MAIN_ORNAMENT_ADD_ABSOLUTE, _("Add Ornament absolute...\tCTRL+A"), _("Add an ornament in the score, at the absolute current position"));
	editMenu->Append(ID_MAIN_ORNAMENT_ADD_RELATIVE, _("Add Ornament relative...\tCTRL+R"), _("Add an ornament in the score, at the relative current position"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_MAIN_ZOOM, _("Zoom\tCtrl++"));
	editMenu->Append(ID_MAIN_UNZOOM, _("Unzoom\tCtrl+-"));

	actionMenu = new wxMenu;

	listMenu = new wxMenu;
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_NEW, _("New List"));
	listMenu->Append(ID_MAIN_LIST_OPEN, _("Open List..."));
	listMenu->Append(ID_MAIN_LIST_SAVE, _("Save List"));
	listMenu->Append(ID_MAIN_LIST_SAVEAS, _("Save List as..."));
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_ADD, _("Add current file in list"));
	listMenu->Append(ID_MAIN_LIST_REMOVE, _("Remove current file from list"));
	listMenu->Append(ID_MAIN_LIST_UP, _("Up current file in list"));
	listMenu->Append(ID_MAIN_LIST_DOWN, _("Down current file in list"));
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_PREVIOUS_FILE, _("Previous file\tCTRL+LEFT"));
	listMenu->Append(ID_MAIN_LIST_NEXT_FILE, _("Next file\tCTRL+RIGHT"));

	wxMenu *settingMenu = new wxMenu;
	settingMenu->Append(ID_MAIN_MIXER, _("Mixer\tCTRL+M"));
	settingMenu->Append(ID_MAIN_MIDISHORTCUT, _("Shortcut"));
	settingMenu->Append(ID_MAIN_EXPRESSION, _("LUA setting"));
	settingMenu->AppendSeparator();
	settingMenu->Append(ID_MAIN_RESET, _("Reset"),_("Reset the configuration"));
#ifdef __WINDOWS__
	settingMenu->Append(ID_MAIN_AUDIO_SETTING, _("Audio..."), _("Audio ASIO settings, to decrease latency, and to select the default audio output"));
#else
	settingMenu->Append(ID_MAIN_AUDIO_SETTING, _("Audio output..."), _("select the default audio output"));
#endif
	settingMenu->Append(ID_MAIN_LUAFILE, _("LUA Files..."));
	settingMenu->Append(ID_MAIN_LOG, _("Log"));
	//settingMenu->Append(ID_MAIN_TEST, "Test\tCTRL+T");
	settingMenu->AppendSeparator();
	settingMenu->AppendCheckItem(ID_MAIN_LOCAL_OFF, _("Send MIDI local-off"), _("Send local-off on MIDI-out opening, i.e. to unlink keyboard and soud-generator on electronic piano"));
	settingMenu->AppendSeparator();
	settingMenu->Append(ID_MAIN_SETTING_OPEN, _("Load setting..."));
	settingMenu->Append(ID_MAIN_SETTING_SAVE, _("Save setting"));
	settingMenu->Append(ID_MAIN_SETTING_SAVEAS, _("Save setting as..."));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(wxID_HELP, _("Web help"));
	helpMenu->Append(wxID_ABOUT, _("About"));
	helpMenu->Append(ID_MAIN_UPDATE, _("Check update"));

    mMenuBar = new wxMenuBar( wxMB_DOCKABLE );

	mMenuBar->Append(fileMenu, _("File"));
	mMenuBar->Append(editMenu, _("Edit"));
	mMenuBar->Append(actionMenu, _("Action"));
	mMenuBar->Append(listMenu, _("List"));
	mMenuBar->Append(settingMenu, _("Setting"));
	mMenuBar->Append(helpMenu, _("Help"));

    // Associate the menu bar with the frame
	SetMenuBar(mMenuBar);

	toolBar = CreateToolBar();

	if (end_ok)
	{
		// read the list of scores
		listName.Assign(mConf->get(CONFIG_LISTNAME, ""));
		if (listName.IsFileReadable())
			ListOpen();

		fileName.Assign(mConf->get(CONFIG_FILENAME, ""));
	}

	editMode = false;
	editMenu->Check(wxID_EDIT, false);

	localoff = (bool)(mConf->get(CONFIG_LOCALOFF, true));
	settingMenu->Check(ID_MAIN_LOCAL_OFF, localoff);

	mtimer = new wxTimer(this, ID_MAIN_TIMER);

	// text for the score
	mTextscore = new textscore(this, ID_MAIN_TEXT_SONG, mConf);
	mTextscore->SetMinSize(wxSize(0, 0));
	mTextscore->Bind(wxEVT_CHAR, &Expresseur::OnChar, this);
	mTextscore->Bind(wxEVT_KEY_UP, &Expresseur::OnKeyUp, this);
	mTextscore->Bind(wxEVT_KEY_DOWN, &Expresseur::OnKeyDown, this);
	mTextscore->Bind(wxEVT_LEFT_DOWN, &Expresseur::OnLeftDown, this);

	// empty viewer for the score
	mViewerscore = new emptyscore(this, wxID_ANY, mConf);
	

	// scroll horizontal
	posScrollHorizontal = mConf->get(CONFIG_MAIN_SCROLLHORIZONTAL, 20);
	mScrollHorizontal = new	wxScrollBar(this, ID_MAIN_SCROLL_HORIZONTAL, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);
	mScrollHorizontal->SetToolTip(_("split horizontally the text and the image of the Score"));
	mScrollHorizontal->SetScrollbar(posScrollHorizontal, 1, 100, 1, false);
	posScrollVertical = mConf->get(CONFIG_MAIN_SCROLLVERTICAL, 50);
	mScrollVertical = new	wxScrollBar(this, ID_MAIN_SCROLL_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	mScrollVertical->SetScrollbar(posScrollVertical, 1, 100, 1, false);
	mScrollVertical->SetToolTip(_("split vertically the text and the image of the Score"));


	rectFrame.SetX(mConf->get(CONFIG_MAINX, 10));
	rectFrame.SetY(mConf->get(CONFIG_MAINY, 10));
	rectFrame.SetWidth(mConf->get(CONFIG_MAINWIDTH, 1010));
	rectFrame.SetHeight(mConf->get(CONFIG_MAINHEIGHT, 780));

	buildSizer();

	FileOpen();
}
Expresseur::~Expresseur()
{
	checkUpdate();

	mtimer->Stop();
	delete mtimer;

	fileHistory->Save(*mConf->getConfig());

	if (listName.IsFileReadable() == true)
		mConf->set(CONFIG_LISTNAME, listName.GetFullPath());
	else
		mConf->remove(CONFIG_LISTNAME);

	if (fileName.IsFileReadable() == true)
		mConf->set(CONFIG_FILENAME, fileName.GetFullPath());
	else
		mConf->remove(CONFIG_FILENAME);

	wxRect mrect = GetRect();
	mConf->set(CONFIG_MAINWIDTH, mrect.GetWidth());
	mConf->set(CONFIG_MAINHEIGHT, mrect.GetHeight());
	mConf->set(CONFIG_MAINX, mrect.GetX());
	mConf->set(CONFIG_MAINY, mrect.GetY());

	mConf->set(CONFIG_MAIN_SCROLLHORIZONTAL, posScrollHorizontal);
	mConf->set(CONFIG_MAIN_SCROLLVERTICAL, posScrollVertical);

	mConf->set(CONFIG_LOCALOFF, localoff );

	if (mExpression)
	{
		mExpression->savePos();
		mConf->set(CONFIG_EXPRESSIONVISIBLE, (mExpression->IsVisible()));
	}
	if (mMidishortcut)
	{
		mMidishortcut->savePos();
	}
	if (mMixer)
	{
		mMixer->savePos();
		mConf->set(CONFIG_MIXERVISIBLE, (mMixer->IsVisible()));
	}

	delete mLog;
	delete mTextscore;
	delete mViewerscore;
	delete mMixer;
	delete mMidishortcut;
	delete mExpression;
	delete fileHistory;

	if (musicxmlDll.IsLoaded())
	{
		if (MNLRelease)
			MNLRelease();
		musicxmlDll.Unload() ;
	}

	basslua_close();

	mConf->set(CONFIG_END_OK, true);
	delete mConf;
}

void Expresseur::buildSizer()
{
	sizer_text_viewer = new wxBoxSizer(wxVERTICAL);
	sizer_Text = sizer_text_viewer->Add(mTextscore, wxSizerFlags().Expand().Proportion(50));
	sizer_viewer = sizer_text_viewer->Add(mViewerscore, wxSizerFlags().Expand().Proportion(50));
	wxBoxSizer *sizer_scroll_horizontal = new wxBoxSizer(wxHORIZONTAL);
	sizer_scroll_horizontal->AddSpacer(mScrollVertical->GetSize().GetWidth());
	sizer_scroll_horizontal->Add(mScrollHorizontal, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer *sizer_scroll_vertical = new wxBoxSizer(wxHORIZONTAL);
	sizer_scroll_vertical->Add(mScrollVertical, wxSizerFlags().Expand());
	sizer_scroll_vertical->Add(sizer_text_viewer, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(sizer_scroll_horizontal, wxSizerFlags().Expand());
	topSizer->Add(sizer_scroll_vertical, wxSizerFlags().Expand().Proportion(1));
	setOrientation(posScrollVertical, posScrollHorizontal);
	SetSizerAndFit(topSizer);

}
void Expresseur::setOrientation(int v, int h )
{
	if (v == 50)
	{
		sizer_text_viewer->SetOrientation(wxHORIZONTAL);
		sizer_Text->SetProportion(h);
		sizer_viewer->SetProportion(100 - h);
	}
	else
	{
		sizer_text_viewer->SetOrientation(wxVERTICAL);
		sizer_Text->SetProportion(v);
		sizer_viewer->SetProportion(100 - v);
	}
}
void Expresseur::OnHorizontalScroll(wxScrollEvent& event)
{
	posScrollHorizontal = event.GetPosition();
	if (posScrollHorizontal == 50) posScrollHorizontal = 51;
	posScrollVertical = 50;
	mScrollVertical->SetThumbPosition(50);
	setOrientation(posScrollVertical, posScrollHorizontal);
	layoutWaiting = true;
	mTextscore->SetFocus();
}
void Expresseur::OnVerticalScroll(wxScrollEvent& event)
{
	posScrollVertical = event.GetPosition();
	if (posScrollVertical == 50) posScrollVertical = 51;
	posScrollHorizontal = 50;
	mScrollHorizontal->SetThumbPosition(50);
	setOrientation(posScrollVertical, posScrollHorizontal);
	layoutWaiting = true ;
	mTextscore->SetFocus();
}
void Expresseur::OnSize(wxSizeEvent& WXUNUSED(event))
{
	layoutWaiting = true;
}
void Expresseur::OnTextChange(wxCommandEvent& WXUNUSED(event))
{
	// SetStatusText(wxT("change"));
}
void Expresseur::OnChar(wxKeyEvent& event)
{
	if (editMode)
	{
		event.Skip(true);
		waitBeforeToCompile = 2000 / periodTimer;
	}
	else
	{
		wxChar ukeychar = event.GetUnicodeKey();
		if ((ukeychar != WXK_NONE) && (ukeychar >= 32))
		{
			int ikeychar = ckeychar; // use the keycode generated during just previous OnKeyDown event
			ckeys[ikeychar] = ukeychar;
			mMidishortcut->hitkey(ckeys[ikeychar], true , this);
			event.Skip(false);
		}
		else
			event.Skip(true);
	}
}
void Expresseur::OnKeyUp(wxKeyEvent& event)
{
	if (editMode)
	{
		waitBeforeToCompile = 2000 / periodTimer;
		event.Skip(true);
	}
	else
	{
		wxChar upkeychar = event.GetKeyCode(); // compliant with the keycode generated during OnKeyDown event
		int ikeychar = upkeychar;
		if (ckeys[ikeychar] != 0)
		{
			mMidishortcut->hitkey(ckeys[ikeychar], false , this);
			ckeys[ikeychar] = 0;
		}
		event.Skip(false);
	}
}
void Expresseur::OnKeyDown(wxKeyEvent& event)
{
	if (editMode)
	{
		waitBeforeToCompile = 2000 / periodTimer;
		event.Skip(true);
	}
	else
	{
		ckeychar = event.GetKeyCode();
		event.Skip(true);
	}
}
void Expresseur::OnLeftDown(wxMouseEvent& event)
{
	if (! editMode)
	{
		leftDown = true;
	}
	event.Skip(true);
}
bool Expresseur::timerTask(bool compile, bool longWait)
{
	bool quick = luafile::isCalledback();
	switch (mode)
	{
	case modeChord:
	{
		// compile the text of chords
		if (compile)
			mTextscore->compileText();

		if ((quick) || (longWait))
		{
			// scan the current position given by LUA module, according to MID events
			int nrChord = mTextscore->scanPosition(editMode);
			mViewerscore->setPosition(nrChord, true, quick);
		}

		// Manage a mouse-left-down on the text screen , to change the position in the text of chords
		if (leftDown == true)
		{
			leftDown = false;
			int pos = mTextscore->getInsertionPoint();
			basslua_call(moduleChord, functionChordSetPosition, "i", pos);
		}
		break;
	}
	case modeScore:
	{
		if ((quick) || (longWait))
		{
			int nrEvent, playing;
			basslua_call(moduleScore, functionScoreGetPosition, ">ii", &nrEvent, &playing);
			mViewerscore->setPosition(nrEvent, (playing>0) , quick);
		}
		break;
	}
	default:
		break;
	}

	if ((quick) || (longWait))
	{
		// scan pendings useful events from LUA
		if (mLog && (mLog->IsVisible()))
			mLog->scanLog(); // updates any log from LUA to the window of this GUI
		if (mMixer && (mMixer->IsVisible()))
			mMixer->scanVolume(); // updates the volume of the mixer of thuis GUI, with the values in LUA
		if (mExpression && (mExpression->IsVisible()))
			mExpression->scanValue(); // updates the values of the "expression" of thuis GUI, with the values in LUA
		if (mMidishortcut)
			mMidishortcut->scanMidi(); // scan midi-in events for the potential shortcuts needed in the definition of the GUI

		// scan from LUA, if a MIDI event asked to go to next file in the list of file
		int n;
		if ((basslua_table(moduleGlobal, tableInfo, -1, fieldNext, NULL, &n, tableGetKeyValue | tableNilKeyValue) & tableGetKeyValue) == tableGetKeyValue)
			ListSelectNext(n);

		// scan if the LUA status text has been changed
		char ch[1024];
		if ((basslua_table(moduleGlobal, tableInfo, -1, fieldValue, ch, NULL, tableGetKeyValue | tableNilKeyValue) & tableGetKeyValue) == tableGetKeyValue)
			SetStatusText(ch, 1);

		// scan pending layout
		if (layoutWaiting)
		{
			Layout();
			mViewerscore->newLayout();
			mViewerscore->Refresh();
		}
		layoutWaiting = false;
	}

	// restart the timer
	mtimer->Start(periodTimer);
	return quick;
}
void Expresseur::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	bool quick = timerTask(waitBeforeToCompile == 1, waitlong == 1);
	if (waitBeforeToCompile > 0)
		waitBeforeToCompile--;
	if ((waitlong > 0) && ( !quick ))
		waitlong--;
	else
		waitlong = tLongWait / periodTimer;
}
void Expresseur::setWindowsTitle()
{
	SetTitle(APP_NAME << wxString(" - ") << fileName.GetFullName()); // Set the Title to reflect the file open

}
void Expresseur::FileOpen()
{
	if (settingReset(false))
	{
		if (fileName.IsFileReadable())
		{
			fileHistory->AddFileToHistory(fileName.GetFullPath());
			ListCheck();
		}
	}
	setWindowsTitle();
}
void Expresseur::FileSave()
{
	fileName.SetExt(SUFFIXE_TEXT);
	mTextscore->saveFile(fileName);
	fileHistory->AddFileToHistory(fileName.GetFullPath());
	setWindowsTitle();
	editMode = false;
	menuEditMode->Check(false);
	switch (mode)
	{
	case modeScore:
	  settingReset(false);
	  break;
	default:
		break;
	}
}
void Expresseur::getLuaAction(bool all, wxMenu *newActionMenu)
{
	wxBitmap b;
	wxFileName fb;
	fb.AssignCwd();
	fb.AppendDir("lua");
	fb.SetExt("bmp");

	wxString s;
	nameAction.Clear();
	char name[512];
	int nrAction = 0;
	while (basslua_table(moduleGlobal, tableActions, nrAction, fieldName, name, NULL, tableGetKeyValue) == tableGetKeyValue)
	{
		nameAction.Add(name);
		char shortcut[64] = "";
		char help[512] = "";
		basslua_table(moduleGlobal, tableActions, nrAction, fieldShortcut, shortcut, NULL, tableGetKeyValue);
		basslua_table(moduleGlobal, tableActions, nrAction, fieldHelp, help, NULL, tableGetKeyValue);
		if (strlen(shortcut) != 0)
		{
			if ((editMode) || (strlen(shortcut) == 0))
				s.Printf("%s", _(name));
			else
				s.Printf("%s\t%s", _(name), shortcut);
			if (newActionMenu != NULL)
				newActionMenu->Append(ID_MAIN_ACTION + nrAction, s, _(help));
		}
		if (all)
		{
			char icone[64] = "";
			basslua_table(moduleGlobal, tableActions, nrAction, fieldIcone, icone, NULL, tableGetKeyValue);
			fb.SetName(icone);
			s.Printf("%s %s", _(name), shortcut);
			if (fb.IsFileReadable())
			{
				if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				{
					toolBar->AddTool(ID_MAIN_ACTION + nrAction, _(name), b, _(help));
				}
			}
		}
		nrAction++;
	}
}
void Expresseur::SetMenuAction(bool all)
{
	// create the actionMenu and the associated icones in the toolbar, according to actions known in the LUA script
	mTextscore->setEditMode(editMode);
	wxString s;
	wxMenu *newActionMenu = new wxMenu;
	wxBitmap b;
	wxFileName fb;
	fb.AssignCwd();
	fb.SetExt("bmp");
	if (all)
	{
		fb.SetName("exit");
		int w = 15;
		int h = 15;
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
			{
			w = b.GetWidth();
			h = b.GetHeight();
			}

		toolBar->ClearTools();
		toolBar->SetToolBitmapSize(wxSize(w, h));
		fb.SetName("open");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(wxID_OPEN, _("Open..."), b, _("Open..."));
		fb.SetName("save");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(wxID_SAVE, _("Save"), b, _("Save"));
		fb.SetName("edit");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(wxID_EDIT, _("Edit"), b, _("edit mode"), wxITEM_CHECK);
		fb.SetName("mixer");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(ID_MAIN_MIXER, _("Mixer"), b, _("Mixer"));
		fb.SetName("help");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(wxID_HELP, _("Help"), b, _("Help"));
		fb.SetName("exit");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_BMP))
				toolBar->AddTool(wxID_EXIT, _("Exit"), b, _("Exit"));
		toolBar->AddSeparator();
	}

	if (toolBar->FindById(wxID_EDIT))
		toolBar->ToggleTool(wxID_EDIT, editMode);
	editMenu->Check(wxID_EDIT, editMode);

	getLuaAction(all, newActionMenu);

	if ( all )
		toolBar->Realize();

	wxMenu *oldactionMenu = mMenuBar->Replace(2, newActionMenu, _("Action"));
	delete oldactionMenu;

	mTextscore->SetEditable(editMode);
	if (editMode == false)
	{
		mTextscore->SelectNone();
		//mTextscore->HideNativeCaret();;
	}
	/*
	else
	{
		mTextscore->ShowNativeCaret(true);;
	}
	*/
}
void Expresseur::OnMenuAction(wxCommandEvent& event)
{
	int nrAction = event.GetId() - ID_MAIN_ACTION;
	if (basslua_table(moduleGlobal, tableActions, nrAction, fieldCallFunction, NULL, NULL, tableCallKeyFunction) != tableCallKeyFunction)
	{
		if (basslua_table(moduleGlobal, tableActions, nrAction, (mode == modeChord) ? fieldCallChord : fieldCallScore, NULL, NULL, tableCallKeyFunction) == tableCallKeyFunction)
			luafile::functioncallback();
		else
		{
			wxString s;
			s.Printf(_("Error calling LUA action %d : %s "), nrAction + 1, nameAction[nrAction]);
			SetStatusText(s, 1);
		}
	}
}
void Expresseur::OnNew(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	mTextscore->SetValue("");
	fileName.SetName("newsong");
	fileName.SetExt(SUFFIXE_TEXT);
	wxTextFile tfile;
	if (fileName.IsFileWritable() == false)
		tfile.Create(fileName.GetFullPath());
	tfile.Write();
	tfile.Close();
	Open(fileName.GetFullPath());
	mTextscore->noNeedToSave();
}
void Expresseur::OnOpen(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	wxFileDialog
		openFileDialog(this, _("Open file"), "", "",
		"music file (*.txt;*.bmp;*.xml;*.mxl)|*.txt;*.bmp;*.xml;*.mxl", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	Open(openFileDialog.GetPath());
	mTextscore->noNeedToSave();
}
void Expresseur::OnSave(wxCommandEvent& WXUNUSED(event)) 
{
	if (fileName.IsFileWritable() == false)
	{
		wxFileDialog
			openFileDialog(this, _("Save file"), "", "",
			"song file (*.txt)|*.txt", wxFD_SAVE);
		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return; // the user changed idea...
		fileName.Assign(openFileDialog.GetPath());
	}
	FileSave();
}
void Expresseur::OnSaveas(wxCommandEvent& WXUNUSED(event)) 
{
	wxFileDialog
		openFileDialog(this, _("Save file"), "", "",
		"song file (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	fileName.Assign(openFileDialog.GetPath());
	FileSave();
}
void Expresseur::OnExit(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	Close(true);
}

void Expresseur::OnUndo(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Undo();
}
void Expresseur::OnRedo(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Redo();
}
void Expresseur::OnCopy(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Copy();
}
void Expresseur::OnCut(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Cut();
}
void Expresseur::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	mTextscore->Paste();
}
void Expresseur::OnZoom(wxCommandEvent& WXUNUSED(event))
{
	mTextscore->zoom(1);
	mViewerscore->zoom(1);
}
void Expresseur::OnUnzoom(wxCommandEvent& WXUNUSED(event))
{
	mTextscore->zoom(-1);
	mViewerscore->zoom(-1);
}
void Expresseur::OnOrnamentAddAbsolute(wxCommandEvent& WXUNUSED(event))
{
	ornamentAdd(true);
}
void Expresseur::OnOrnamentAddRelative(wxCommandEvent& WXUNUSED(event))
{
	ornamentAdd(false);
}
void Expresseur::ornamentAdd(bool absolute)
{
	if (mode != modeScore)
		return;
	int absolute_measure_nr, measure_nr, beat, t;
	bool ret = ((musicxmlscore *)(mViewerscore))->getScorePosition(&absolute_measure_nr, &measure_nr,&beat, &t);
	if (!ret)
		return;
	wxArrayString list_ornament = musicxmlcompile::getListOrnament();
	wxString ornament = wxGetSingleChoice("Select ornament", "Add ornament", list_ornament, this);
	if (ornament.IsEmpty())
		return;
	wxString line;
	wxString measure;
	wxString repeat;
	wxString ti;
	if (absolute)
		measure.Printf("!%d", absolute_measure_nr);
	else
	{
		measure.Printf("%d", measure_nr);
	}
	if (t > 0)
		ti.Printf(".%d", t);
	line.Printf("%s.%d%s%s:%s", measure, beat + 1, ti, repeat, ornament);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(line));
		wxTheClipboard->Close();
		wxMessageBox("You can 'paste' the line in the Score Description", "Add ornament");
	}
	else
	{
		wxMessageBox("Error clipboard ...");
	}

}
void Expresseur::OnEdit(wxCommandEvent& WXUNUSED(event))
{
	editMode = !editMode;
	SetMenuAction(false);
}
void Expresseur::OnLocaloff(wxCommandEvent& WXUNUSED(event))
{
	localoff = !localoff;
}

void Expresseur::ListClearMenu()
{
	for (unsigned int i = ID_MAIN_LIST_FILE; i < ID_MAIN_LIST_FILE_END; i++)
	{
		if (listMenu->FindItem(i) != NULL)
			listMenu->Delete(i);
	}
}
void Expresseur::ListUpdateMenu()
{
	ListClearMenu();
	wxFileName f;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		f.Assign(listFiles.Item(i));
		listMenu->PrependCheckItem(ID_MAIN_LIST_FILE + i, f.GetFullName() , f.GetFullPath() );
		listMenu->Check(ID_MAIN_LIST_FILE + i, false);
	}
}
void Expresseur::ListSave()
{
	wxString s;
	wxFileName f;
	wxTextFile tfile;
	if (listName.IsFileWritable() == false)
		tfile.Create(listName.GetFullPath());
	fileHistory->AddFileToHistory(listName.GetFullPath());
	tfile.Open(listName.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	tfile.Clear();
	tfile.AddLine(LIST_FILE);
	for (unsigned int i = 0; i < listFiles.Count(); i++)
	{
		f.Assign(listFiles.Item(i));
		f.MakeRelativeTo(listName.GetPath());
		tfile.AddLine(f.GetFullPath());
	}
	tfile.Write();
	tfile.Close();
	listChanged = false;
	fileHistory->AddFileToHistory(listName.GetFullPath());
}
void Expresseur::ListOpen()
{
	wxString        str;
	wxFileName f;
	// open the file
	wxTextFile      tfile;
	if (listName.IsFileReadable() == false)
		return;
	fileHistory->AddFileToHistory(listName.GetFullPath());
	tfile.Open(listName.GetFullPath());
	if (tfile.IsOpened() == false)
	{
		ListNew();
		return;
	}
	str = tfile.GetFirstLine();
	if (str == LIST_FILE)
	{
		str = tfile.GetNextLine();
	}
	while (!tfile.Eof())
	{
		if (str.IsEmpty() == false)
		{
			f.Assign(str);
			f.MakeAbsolute(listName.GetPath());
			listFiles.Add(f.GetFullPath());
		}
		str = tfile.GetNextLine();
	}
	tfile.Close();
	ListUpdateMenu();
	listSelection = -1;
	ListCheck();
}
void Expresseur::ListNew()
{
	ListClearMenu();
	listFiles.Clear();
	listName.Clear();

}
void Expresseur::ListCheck()
{
	wxFileName f;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		f.Assign(listFiles.Item(i));
		if (f.GetFullPath() == fileName.GetFullPath())
		{
			listMenu->Check(ID_MAIN_LIST_FILE + i, true);
			listSelection = i;
			return;
		}
	}

}
void Expresseur::ListSelectNext(int df)
{
	if (listSelection == -1)
	{
		if (df > 0)
			ListSelect(0);
		else
			ListSelect(listFiles.Count() - 1);
		return;
	}
	ListSelect(listSelection + df);
}
void Expresseur::ListSelect(int id)
{
	int nrfile;
	if ((int)(listFiles.Count()) == 0)
		return;
	nrfile = id;
	if (id < 0)
		nrfile = (int)(listFiles.Count()) - 1;
	if (id >= (int)(listFiles.Count()))
		nrfile = 0;
	Open(listFiles.Item(nrfile));
	listSelection = nrfile;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		listMenu->Check(ID_MAIN_LIST_FILE + i, (i == nrfile));
	}
}
void Expresseur::OnListNew(wxCommandEvent& WXUNUSED(event)) 
{ 
	ListNew();
}
void Expresseur::OnListOpen(wxCommandEvent& WXUNUSED(event)) 
{
	if (listChanged)
	{
		if (wxMessageBox(_("Current list has not been saved! Proceed?"), _("Please confirm"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	wxFileDialog
		openFileDialog(this, _("Open list file"), "", "",
		"list files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...

	listFiles.Clear();
	listName.Assign(openFileDialog.GetPath());
	ListOpen();
}
void Expresseur::OnListSave(wxCommandEvent& WXUNUSED(event)) 
{
	if (listName.IsFileWritable() == false)
	{
		wxFileDialog
			openFileDialog(this, _("Save list file"), "", "",
			"list files (*.txt)|*.txt", wxFD_SAVE );
		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return; // the user changed idea...
		listName.Assign(openFileDialog.GetPath());
	}
	ListSave();
}
void Expresseur::OnListSaveas(wxCommandEvent& WXUNUSED(event)) 
{
	wxFileDialog
		openFileDialog(this, _("Save list file"), "", "",
		"list files (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	listName.Assign(openFileDialog.GetPath());
	ListSave();
}
void Expresseur::OnListAdd(wxCommandEvent& WXUNUSED(event)) 
{
	if (fileName.IsOk() == false)
		return;
	listFiles.Add(fileName.GetFullPath());
	ListUpdateMenu();
	listChanged = true;
	ListCheck();
}
void Expresseur::OnListRemove(wxCommandEvent& WXUNUSED(event)) 
{
	if (listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
	{
		listFiles.Remove(fileName.GetFullPath());
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListUp(wxCommandEvent& WXUNUSED(event)) 
{
	if ((listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
		&& (listFiles.Index(fileName.GetFullPath()) != 0 ))
	{
		int p = listFiles.Index(fileName.GetFullPath());
		listFiles.Insert(fileName.GetFullPath(), p-1);
		listFiles.RemoveAt(p+1);
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListDown(wxCommandEvent& WXUNUSED(event)) 
{
	if ((listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
		&& (listFiles.Index(fileName.GetFullPath()) != ((int)(listFiles.Count()) - 1 )))
	{
		int p = listFiles.Index(fileName.GetFullPath());
		listFiles.Insert(fileName.GetFullPath(), p + 2);
		listFiles.RemoveAt(p);
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListFile(wxCommandEvent& event) 
{
	ListSelect(event.GetId() - ID_MAIN_LIST_FILE);
}
void Expresseur::OnListPreviousFile(wxCommandEvent& WXUNUSED(event))
{
	ListSelectNext(-1);
}
void Expresseur::OnListNextFile(wxCommandEvent& WXUNUSED(event)) 
{
	ListSelectNext(1);
}

void Expresseur::OnMixer(wxCommandEvent& WXUNUSED(event))
{
	mMixer->Show();
	mMixer->Raise();
}
void Expresseur::OnGoto(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeScore)
		return;
	mViewerscore->gotoPosition();
}
void Expresseur::OnMidishortcut(wxCommandEvent& WXUNUSED(event))
{
	mMidishortcut->Show();
	mMidishortcut->Raise();
}
void Expresseur::OnExpression(wxCommandEvent& WXUNUSED(event))
{
	mExpression->Show();
	mExpression->Raise();
}
void Expresseur::OnLuafile(wxCommandEvent& WXUNUSED(event))
{
	luafile mLuafile(this, wxID_ANY, _("Lua script"), mConf);
	if ( mLuafile.ShowModal() == 1 )
		settingReset(true);
}
void Expresseur::settingSave()
{
	wxArrayString lChoice;
	lChoice.Add(_("Mixer"));
	lChoice.Add(_("Expression"));
	lChoice.Add(_("Shortcuts"));
	lChoice.Add(_("Lua Files"));
	wxMultiChoiceDialog mChoice(this, _("Select the settings to save"), _("Savec setting"), lChoice, wxOK);
	if (mChoice.ShowModal() != wxID_OK)
		return;
	wxArrayInt listToSave = mChoice.GetSelections();

	wxString s;
	wxFileName f;
	wxTextFile tfile;
	if (settingName.IsFileWritable() == false)
		tfile.Create(settingName.GetFullPath());
	fileHistory->AddFileToHistory(settingName.GetFullPath());
	tfile.Open(settingName.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	
	tfile.Clear();

	tfile.AddLine(CONFIG_FILE);

	for (unsigned int i = 0; i < listToSave.GetCount(); i++)
	{
		switch (listToSave[i])
		{
		case 0: mMixer->write(&tfile); break;
		case 1: mExpression->write(&tfile); break;
		case 2:mMidishortcut->write(&tfile); break;
		case 3: luafile::write(mConf, &tfile); break;
		default: break;
		}
	}

	tfile.Write();
	tfile.Close();
	fileHistory->AddFileToHistory(settingName.GetFullPath());
}
void Expresseur::settingOpen()
{
	wxString        str;
	wxFileName f;
	// open the file
	wxTextFile      tfile;
	if (settingName.IsFileReadable() == false)
		return;
	tfile.Open(settingName.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	str = tfile.GetFirstLine();
	if (str != CONFIG_FILE)
	{
		wxMessageDialog(this, "First line of the file is not the expected one", "read setting error", wxICON_ERROR | wxOK );
		return;
	}

	mMixer->read( &tfile);
	mMidishortcut->read( &tfile);
	mExpression->read(&tfile);
	luafile::read(mConf, &tfile);

	tfile.Close();


}
bool Expresseur::settingReset(bool all)
{
	bool retcode = true;

	// stop the timer to be quite
	mtimer->Stop();

	// close and load the right LUA script
	luafile::reset(mConf , all);

	// setup the menus
	SetMenuAction(true);

	// load the mixer
	if (mMixer != NULL)
	{
		mMixer->Show(false);
		delete mMixer;
	}
	mMixer = NULL;
	mMixer = new mixer(this, wxID_ANY, _("mixer"), mConf, mViewerscore);

	// load the shortcuts
	if (mMidishortcut != NULL)
	{
		mMidishortcut->Show(false);
		delete mMidishortcut;
	}
	mMidishortcut = NULL;
	mMidishortcut = new midishortcut(this, wxID_ANY, _("shortcut"), mConf, nameAction);

	// load the expression
	if (mExpression != NULL)
	{
		mExpression->Show(false);
		delete mExpression;
	}
	mExpression = NULL;
	mExpression = new expression(this, wxID_ANY, _("Expression"), mConf);

	// caculate the prefix of settings, according to valid midi-out devices connected
	mConf->setPrefix();

	int h = posScrollHorizontal;
	int v = posScrollVertical;

	int nrDeviceAudio = mConf->get(CONFIG_DEFAULT_AUDIO, 0);
	basslua_call(moduleLuabass, "audioDefaultDevice", "i", nrDeviceAudio + 1);

	viewerscore *newViewerscore = NULL;
	typeViewer = EMPTYVIEWER;
	mode = modeNil;

	wxString ext = fileName.GetExt();
	if ((ext == SUFFIXE_MUSICXML) || (ext == SUFFIXE_MUSICMXL))
	{
		newViewerscore = new musicxmlscore(this, wxID_ANY, mConf, musicxmlDll);
		if (newViewerscore->setFile(fileName, onstart))
		{
			typeViewer = MUSICXMLVIEWER;
			mode = modeScore;
		}
		else
		{
			delete newViewerscore;
			newViewerscore = NULL;
			typeViewer = EMPTYVIEWER;
		}
	}
	if (ext == SUFFIXE_BITMAPCHORD)
	{
		newViewerscore = new bitmapscore(this, wxID_ANY, mConf);
		if (newViewerscore->setFile(fileName, onstart))
		{
			typeViewer = BITMAPVIEWER;
			mode = modeChord;
		}
		else
		{
			delete newViewerscore;
			newViewerscore = NULL;
			typeViewer = EMPTYVIEWER;
		}
	}
	if (ext == SUFFIXE_TEXT)
	{
		newViewerscore = new musicxmlscore(this, wxID_ANY, mConf, musicxmlDll);
		if (newViewerscore->setFile(fileName, onstart))
		{
			typeViewer = MUSICXMLVIEWER;
			mode = modeScore;
		}
		else
		{
			delete newViewerscore;
			newViewerscore = NULL;
			typeViewer = EMPTYVIEWER;
			mode = modeChord;
		}
	}
	if (newViewerscore == NULL)
	{
		// empty viewer for the score
		newViewerscore = new emptyscore(this, wxID_ANY, mConf);
		v = 50;
		h = 100;
	}

	basslua_setMode(mode);
	if (mode != modeScore)
		musicxmlcompile::clearLuaScore();

	mTextscore->setFile(fileName);

	waitBeforeToCompile = 2000 / periodTimer;

	sizer_text_viewer->Replace(mViewerscore, newViewerscore);
	delete mViewerscore;
	mViewerscore = newViewerscore;
	setOrientation(v, h);
	layoutWaiting = true;

	// set the size of the windows
	int x, y, width, height;
	x = mConf->get(CONFIG_MIXERX, 30);
	y = mConf->get(CONFIG_MIXERY, 30);
	width = mConf->get(CONFIG_MIXERWIDTH, 500);
	height = mConf->get(CONFIG_MIXERHEIGHT, 350);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mMixer->SetSize(x, y, width, height);
	mMixer->Show(mConf->get(CONFIG_MIXERVISIBLE,false));

	x = mConf->get(CONFIG_SHORTCUTX, 50);
	y = mConf->get(CONFIG_SHORTCUTY, 50);
	width = mConf->get(CONFIG_SHORTCUTWIDTH, 500);
	height = mConf->get(CONFIG_SHORTCUTHEIGHT, 300);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mMidishortcut->SetSize(x, y, width, height);
	mMidishortcut->Show(false);

	x = mConf->get(CONFIG_EXPRESSIONX, 80);
	y = mConf->get(CONFIG_EXPRESSIONY, 80);
	width = mConf->get(CONFIG_EXPRESSIONWIDTH, 500);
	height = mConf->get(CONFIG_EXPRESSIONHEIGHT, 250);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mExpression->SetSize(x, y, width, height);
	mExpression->Show(mConf->get(CONFIG_EXPRESSIONVISIBLE, false));

	if (end_ok)
	{
		mMixer->reset(localoff,true);
		mMidishortcut->reset();
		mExpression->reset();
	}

	mViewerscore->displayFile();

	this->Raise();

	// restart the timer
	timerTask(true,true);

	return retcode;
}
void Expresseur::OnReset(wxCommandEvent& WXUNUSED(event))
{
	end_ok = true;
	settingReset(true);
}
void Expresseur::OnLog(wxCommandEvent& WXUNUSED(event))
{
	// load the log
	if (mLog != NULL)
	{
		delete mLog;
	}
	mLog = NULL;
	mLog = new logerror(this, wxID_ANY, _("log !!! timeline bottom->up : last-event is the first-line !!!"));
	mLog->Show();
}
void Expresseur::OnSettingOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog
		openFileDialog(this, _("Open setting file"), "", "",
		"setting files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...

	settingName.Assign(openFileDialog.GetPath());
	settingOpen();
	settingReset(true);

}
void Expresseur::OnSettingSave(wxCommandEvent& WXUNUSED(event))
{
	if (settingName.IsFileWritable() == false)
	{
		wxFileDialog
			openFileDialog(this, _("Save setting file"), "", "",
			"setting files (*.txt)|*.txt", wxFD_SAVE);
		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return; // the user changed idea...
		settingName.Assign(openFileDialog.GetPath());
	}
	settingSave();
}
void Expresseur::OnSettingSaveas(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog
		openFileDialog(this, _("Save setting file"), "", "",
		"list files (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	settingName.Assign(openFileDialog.GetPath());
	settingSave();
}

void Expresseur::OnAbout(wxCommandEvent& WXUNUSED(event)) 
{	
	wxString s;
	s.Printf("Expresseur 3.%d\n(C) 2015 REVOLLE Franck <franck.revolle@orange.fr>", VERSION_EXPRESSEUR);
 	wxMessageBox(s,"about");
	// wxAboutDialogInfo info;
	// info.SetName(_("Expresseur chord"));
	// info.SetVersion(wxT("3.0 Beta"));
	// info.SetDescription(_("Help to play chords from a simple text file, using any MIDI device.\nExtension possible with LUA script ( www.lua.org ) ."));
	// info.SetCopyright(wxT("(C) 2015 REVOLLE Franck <franck.revolle@orange.fr>"));
	// wxAboutBox(info);
}
void Expresseur::OnHelp(wxCommandEvent& WXUNUSED(event)) 
{
	wxLaunchDefaultBrowser("http://www.expresseur.com");
}
void Expresseur::initFirstUse()
{
	// is is the first time the Expresseur start ?
	bool initialized = mConf->get(CONFIG_INITIALIZED, false);
	if (initialized)
		return;

	// set as already initialized
	mConf->set(CONFIG_INITIALIZED, true);

	// open the LUA script
	luafile::reset(mConf, true);

	// set a prefix on the actual Midi config
	mConf->setPrefix();
	
	// get the user directory in its documents folder
	wxFileName finstruments;
	wxStandardPaths mpath = wxStandardPaths::Get();
	finstruments.AssignDir(mpath.GetAppDocumentsDir());
	if (finstruments.GetFullPath().Contains(APP_NAME) == false)
		finstruments.AppendDir(APP_NAME);

	// get the first score as example
	wxString sfirst;
	sfirst = wxDir::FindFirst(finstruments.GetFullPath(), "*.txt");
	finstruments.SetFullName(sfirst);
	wxString defScore = finstruments.GetFullPath();
	mConf->set(CONFIG_FILENAME, defScore);
	finstruments.SetFullName("");

	// set the directory of instruments
	finstruments.AppendDir("instruments");
	wxString sinstruments = finstruments.GetFullPath();
	mConf->set(CONFIG_DIR_INSTRUMENTS, sinstruments);

	// get the actions from the LUA script
	getLuaAction(false, NULL);

	// create the objects to initialize
	mMixer = new mixer(this, wxID_ANY, _("mixer"), mConf, NULL);
	mMidishortcut = new midishortcut(this, wxID_ANY, _("shortcut"), mConf, nameAction);
	mExpression = new expression(this, wxID_ANY, _("Expression"), mConf);

	// load the dfautl setting for the shorcuts, ...
	settingName.AssignDir(mConf->get(CONFIG_DIR_INSTRUMENTS, ""));
	settingName.SetFullName("default_setting.txt");
	settingOpen();

	// run the wizard to tune up the audio, and to inform the user
	wizard();

	// clean everything
	delete mMixer;
	delete mMidishortcut;
	delete mExpression;
	mMixer = NULL;
	mMidishortcut = NULL;
	mExpression = NULL;
}
void Expresseur::OnAudioSetting(wxCommandEvent& WXUNUSED(event))
{
	wizard(true);
	settingReset();
}
void Expresseur::wizard(bool audio_only)
{
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);
	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	wxString labalwizard;
	if (audio_only)
		labalwizard = "Audio setting";
	else
		labalwizard = "Wizard Expresseur";
	wxWizard *mwizard = new wxWizard(this, wxID_ANY, labalwizard);

	wxWizardPageSimple *pwizard_welcome = new wxWizardPageSimple(mwizard );
	wxBoxSizer *topsizer_welcome = new wxBoxSizer(wxVERTICAL);
	wxString sstart = _("\
Welcome to Expresseur Wizard.\n\n\
Next screens will hep you to setup MIDI and audio devices.\n\
Last screens will describe the default basic tunings to play a score or to improvise.\n");
	topsizer_welcome->Add(new wxStaticText(pwizard_welcome, wxID_ANY,sstart ), sizerFlagMaximumPlace);
	pwizard_welcome->SetSizerAndFit(topsizer_welcome);

	wxWizardPageSimple *pwizard_midi = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_midi = new wxBoxSizer(wxVERTICAL);
	wxString smidiin;
	wxArrayString nameMidiDevices;
	int nrMidiDevice = 0;
	char nameMidiDevice[MAXBUFCHAR];
	*nameMidiDevice = '\0';
	while (true)
	{
		basslua_call(moduleLuabass, sinGetMidiName, "i>s", nrMidiDevice + 1, nameMidiDevice);
		if (*nameMidiDevice == '\0')
			break;
		bool valid = false;
		basslua_call(moduleGlobal, sinMidiIsValid, "s>b", nameMidiDevice, &valid);
		if ( valid )
			nameMidiDevices.Add(nameMidiDevice);
		nrMidiDevice++;
	}
	if (nameMidiDevices.GetCount() == 0)
	{
		smidiin = _("No MIDI keyboard connected. You can play on the computer keyboard with a limited experience. With a MIDI keyboard, it will be easier to play music, adding sensivity and velocity.\n\n");
	}
	else
	{
		smidiin = _("MIDI keyboard detected : it will be easy to play music, adding sensivity and velocity.\n\n");
	}
		topsizer_midi->Add(new wxStaticText(pwizard_midi, wxID_ANY, smidiin + _("\
With an electronic piano, you will add the possibility to play the sound of this piano, with a good quality.\n\n\
If you have a software instrument (e.g. Pianoteq ), connect it on the virtual midi-in cable, and connect Expresseur on the virtual midi-out cable.\n")), sizerFlagMaximumPlace);
	pwizard_midi->SetSizerAndFit(topsizer_midi);

	wxString saudio;
	wxWizardPageSimple *pwizard_audio = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_audio = new wxBoxSizer(wxVERTICAL);
	wxArrayString nameaudioDevices;
	int nraudioDevice = 0;
	char nameaudioDevice[MAXBUFCHAR];
	*nameaudioDevice = '\0';
	while (true)
	{
		basslua_call(moduleLuabass, "audioName", "i>s", nraudioDevice + 1, nameaudioDevice);
		if (*nameaudioDevice == '\0')
			break;
		nameaudioDevices.Add(nameaudioDevice);
		nraudioDevice++;
	}
	if (nameaudioDevices.GetCount() == 0)
	{
#ifdef __WINDOWS__
		saudio = _("No Asio audio device installed.\n\
Asio audio is used to play SF2 software instruments within Expresseur. If you have an electronic-piano to render the notes, you can skip this step.\n\
To use Asio audio with low latency, please install at minimum freeware Asio4all audio driver.");
#else
		saudio = _("No audio device installed.\n\
Audio is used to play SF2 software instruments within Expresseur. If you have an electroni-piano to play the notes, you can skip this step.\n");
#endif
		topsizer_audio->Add(new wxStaticText(pwizard_audio, wxID_ANY, saudio), sizerFlagMaximumPlace);
	}
	else
	{
#ifdef __WINDOWS__
		saudio = _("\
Audio is used to play VSTi & SF2.\n\
Click the audio-device to use by default.\n\n\
Double-click the audio-device to tune it.\n\
Decrease the buffer size to decrease latency.\n\
BE SURE TO KEEP A GOOD QUALITY OF SOUND, using the \"test audio\".");
#else
		saudio = _("Select default audio device\n\
Audio is used to play SF2 software instruments within Expresseur. If you have an electroni-piano to play the notes, you can skip this step.");
#endif
		topsizer_audio->Add(new wxStaticText(pwizard_audio, wxID_ANY, saudio), sizerFlagMaximumPlace);
		wxListBox *mlistAudio = new	wxListBox(pwizard_audio, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameaudioDevices);
		int defaultNrDevice = mConf->get(CONFIG_DEFAULT_AUDIO, 0);
		mlistAudio->SetSelection(defaultNrDevice);
		mlistAudio->Bind(wxEVT_LISTBOX, &Expresseur::OnAudioChoice, this);
		mlistAudio->Bind(wxEVT_LISTBOX_DCLICK, &Expresseur::OnAudioSet, this);
		topsizer_audio->Add(mlistAudio);
#ifdef __WINDOWS__
		wxButton *mTest = new wxButton(pwizard_audio, wxID_ANY, _("TEST AUDIO"));
		mTest->Bind(wxEVT_BUTTON, &Expresseur::OnAsioTest, this);
		topsizer_audio->Add(mTest);
#endif
	}
	pwizard_audio->SetSizerAndFit(topsizer_audio);

	wxWizardPageSimple *pwizard_playscore = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_playscore = new wxBoxSizer(wxVERTICAL);
	wxString splayscore = _("\
To play a score : open a musicXML file, and play with F2 to B2 keys on your MIDI keyboard, or with 'azqw' keys on your computer.\n\n\
To come back at the beginning of the score, use C#2 ( or CTRL-UP).\n\
Some example of musicXML files have been installed in \"My documents\".");
	topsizer_playscore->Add(new wxStaticText(pwizard_playscore, wxID_ANY, splayscore), sizerFlagMaximumPlace);
	pwizard_playscore->SetSizerAndFit(topsizer_playscore);

	wxWizardPageSimple *pwizard_improvise = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_improvise = new wxBoxSizer(wxVERTICAL);
	wxString simprovise = _("\
To improvise on a grid : open a chord file, improvise with pitches in the chord using F3 to A4 white keys , and black keys for pithes out of the chords ( or respectively 'sdfghjkl' and 'rtyuio' on the computer keyboard). \n\
Move to next chord with D#3, or 'x'.\n\n\
To come back at the beginning of the score, use C#2 ( or CTRL-UP).\n\
To switch-off sounds, use E3 ( or BACK )\n\n\
Some example of text files with chords have been installed in \"My documents\".");
	topsizer_improvise->Add(new wxStaticText(pwizard_improvise, wxID_ANY, simprovise), sizerFlagMaximumPlace);
	pwizard_improvise->SetSizerAndFit(topsizer_improvise);

	wxWizardPageSimple *pwizard_end = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_end = new wxBoxSizer(wxVERTICAL);
	wxString send = _("\
Please consult the web help, to benefit all the features, or to change the configuration-behavior.\n\n\
To come back later in this wizard, select the menu setup/wizard.");
	topsizer_end->Add(new wxStaticText(pwizard_end, wxID_ANY, send), sizerFlagMaximumPlace);
	wxButton *bHelp = new wxButton(pwizard_end, wxID_ANY, _("web help"));
	bHelp->Bind(wxEVT_BUTTON, &Expresseur::OnHelp, this);
	topsizer_end->Add(bHelp);
	pwizard_end->SetSizerAndFit(topsizer_end);

	if (audio_only)
	{
		pwizard_audio->SetPrev(NULL);
		pwizard_audio->SetNext(NULL);
		mwizard->RunWizard(pwizard_audio);
	}
	else
	{
		pwizard_welcome->SetPrev(NULL);
		pwizard_welcome->SetNext(pwizard_midi);
		pwizard_midi->SetPrev(pwizard_welcome);
		pwizard_midi->SetNext(pwizard_audio);
		pwizard_audio->SetPrev(pwizard_midi);
		pwizard_audio->SetNext(pwizard_playscore);
		pwizard_playscore->SetPrev(pwizard_audio);
		pwizard_playscore->SetNext(pwizard_improvise);
		pwizard_improvise->SetPrev(pwizard_playscore);
		pwizard_improvise->SetNext(pwizard_end);
		pwizard_end->SetPrev(pwizard_improvise);
		pwizard_end->SetNext(NULL);
		mwizard->RunWizard(pwizard_welcome);
	}

	mwizard->Destroy();
}
void Expresseur::OnAudioChoice(wxCommandEvent& event)
{
	int nrDevice = event.GetSelection();
	basslua_call(moduleLuabass, "audioClose", "");
	mConf->set(CONFIG_DEFAULT_AUDIO, nrDevice);
	basslua_call(moduleLuabass, "audioDefaultDevice", "i", nrDevice + 1);
}
void Expresseur::OnAudioSet(wxCommandEvent& event)
{
	int nrDevice = event.GetSelection();
	OnAudioChoice(event);
	basslua_call(moduleLuabass, "audioAsioSet", "i", nrDevice + 1);
}
void Expresseur::OnAsioTest(wxCommandEvent& WXUNUSED(event))
{
	basslua_call(moduleLuabass, "audioClose", "");
	wxFileName fsound;
	fsound.AssignDir(mConf->get(CONFIG_DIR_INSTRUMENTS, ""));
	fsound.SetFullName("test.wav");
	char buff[MAXBUFCHAR];
	wxString fs = fsound.GetFullPath();
	strcpy(buff, fs.c_str());
	basslua_call(moduleLuabass, "outSoundPlay", "s", buff);
}

void Expresseur::OnUpdate(wxCommandEvent& WXUNUSED(event))
{
	checkUpdate();;
}
void Expresseur::checkUpdate()
{
	wxHTTP get;
	get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
	get.SetTimeout(3); // 10 seconds of timeout instead of 10 minutes ...

	// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
	if (!get.Connect(_T("www.expresseur.com")))
	{
		get.Close();
		return;// only the server, no pages here yet ...
	}

	wxApp::IsMainLoopRunning(); // should return true

	// use _T("/") for index.html, index.php, default.asp, etc.
	wxInputStream *httpStream = get.GetInputStream(_T("/update/"));

	// wxLogVerbose( wxString(_T(" GetInputStream: ")) << get.GetResponse() << _T("-") << ((resStream)? _T("OK ") : _T("FAILURE ")) << get.GetError() );

	if (get.GetError() == wxPROTO_NOERR)
	{
		wxString res;
		wxStringOutputStream out_stream(&res);
		httpStream->Read(out_stream);

		int pos = res.Find("Version#");
		if (pos != wxNOT_FOUND)
		{
			pos += 10;
			int epos = pos ;
			while ((res[epos] >= '0') && (res[epos] <= '9') && (epos < (pos + 10)))
				epos++;
			wxString sv = res.Mid(pos,epos - pos);
			long l;
			if (sv.ToLong(&l))
			{
				int vo = mConf->get(CONFIG_VERSION_CHECKED, VERSION_EXPRESSEUR);
				mConf->set(CONFIG_VERSION_CHECKED, l);
				if (l > vo)
				{
					wxString mes;
					mes.Printf("New version 3.%d available. Go to the web-page ?", l);
					if (wxMessageBox(mes, "update", wxYES_NO) == wxYES)
					{
						wxDELETE(httpStream);
						get.Close();
						wxLaunchDefaultBrowser("http://www.expresseur.com/update/");
						return;
					}
				}
			}
		}
	}
	wxDELETE(httpStream);
	get.Close();

}

void Expresseur::OnTest(wxCommandEvent& WXUNUSED(event))
{
}
void Expresseur::OnRecentFile(wxCommandEvent& event)
{
	wxString f(fileHistory->GetHistoryFile(event.GetId() - wxID_FILE1));

	if (!f.empty())
	{
		Open(f);
	}
}
void Expresseur::Open(wxString f)
{
	wxFileName fn(f);
	wxString ext = fn.GetExt();
	if ( ext == SUFFIXE_TEXT)
	{
		// read the fist line of the file
		wxTextFile      tfile;
		if (fn.IsFileReadable() == false)
			return;
		tfile.Open(fn.GetFullPath());
		if (tfile.IsOpened() == false)
			return;
		wxString str = tfile.GetFirstLine();
		tfile.Close();

		if (str == CONFIG_FILE)
		{
			settingName.Assign(f);
			settingOpen();
			settingReset(true);
			return;
		}
		if (str == LIST_FILE)
		{
			listName.Assign(f);
			ListOpen();
			return;
		}
		fileName.Assign(f);
		FileOpen();
	}
	if ((ext == SUFFIXE_MUSICMXL) || (ext == SUFFIXE_MUSICXML))
	{
		fileName.Assign(f);

		wxFileName txtFilename = fileName;
		txtFilename.SetExt(SUFFIXE_TEXT);
		if (txtFilename.IsFileReadable())
		{
			// a musicxml file is loaded, and the txt file already exists : warning 
			if (wxMessageBox(_("Current text file already exists ! Overwrite ?"), _("File txt exists"),
				wxICON_QUESTION | wxYES_NO, NULL) == wxNO)
				return;
		}
		FileOpen();
	}
	if (ext == SUFFIXE_BITMAPCHORD)
	{
		fileName.Assign(f);
		FileOpen();
	}
}
