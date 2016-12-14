/////////////////////////////////////////////////////////////////////////////
// Name:        luafile.cpp
// Purpose:     non-modal dialog to select lua files /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     08/06/2015
// update : 5/11/ 2016 10 : 00
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/notebook.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/textctrl.h"
#include "wx/statline.h"
#include "wx/config.h"
#include "wx/filepicker.h"
#include "wx/filefn.h"
#include "wx/dir.h"

#include "global.h"
#include "mxconf.h"
#include "luabass.h"
#include "basslua.h"
#include "luafile.h"

bool static calledback = false ;

enum
{
	IDM_LUAFILE_LUA_SCRIPT = ID_LUAFILE ,
	IDM_LUAFILE_LUA_PARAMETER
};

wxBEGIN_EVENT_TABLE(luafile, luafile::wxDialog)
EVT_SIZE(luafile::OnSize)
EVT_CHOICE(IDM_LUAFILE_LUA_SCRIPT, luafile::OnLuaFile)
EVT_TEXT(IDM_LUAFILE_LUA_PARAMETER, luafile::OnLuaParameter)
wxEND_EVENT_TABLE()

luafile::luafile(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	mParent = parent;
	mThis = this;
	mConf = lMxconf;

	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer *paramsizer = new wxFlexGridSizer(3, wxSize(5, 5));
	paramsizer->AddGrowableCol(1);

	wxArrayString lScript, lfScript;
	wxFileName f;
	f.AssignCwd();
	f.AppendDir("lua");
	wxDir::GetAllFiles(f.GetPath(), &lScript, "*.lua", wxDIR_FILES);
	for (unsigned int i = 0; i < lScript.GetCount(); i++)
	{
		wxFileName fs(lScript[i]);
		lfScript.Add(fs.GetFullName());
	}


	paramsizer->Add(new wxStaticText(this, wxID_ANY, _("LUA Script")), sizerFlagMaximumPlace);
	wxString luascriptfile = mConf->get(CONFIG_LUA_SCRIPT, "expresseur.lua");
	wxChoice *cLuaScript = new wxChoice(this, IDM_LUAFILE_LUA_SCRIPT, wxDefaultPosition, wxDefaultSize, lfScript);
	if (lfScript.Index(luascriptfile) != wxNOT_FOUND)
		cLuaScript->SetSelection(lfScript.Index(luascriptfile));
	cLuaScript->SetToolTip(_("LUA script which manages technically midi inputs, midi outputs, ..."));
	paramsizer->Add(cLuaScript, sizerFlagMaximumPlace);

	wxString luascriptparameter = mConf->get(CONFIG_LUA_PARAMETER, "");
	wxTextCtrl *mLuaParameter = new wxTextCtrl(this, IDM_LUAFILE_LUA_PARAMETER, luascriptparameter);
	mLuaParameter->SetToolTip(_("parameter for the LUA script"));
	paramsizer->Add(mLuaParameter, sizerFlagMaximumPlace);

	topsizer->Add(paramsizer, sizerFlagMaximumPlace);
	topsizer->Add(CreateButtonSizer(wxCLOSE), sizerFlagMaximumPlace);
	SetSizerAndFit(topsizer);

	SetReturnCode(0);

}
luafile::~luafile()
{
}
void luafile::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void luafile::OnLuaFile(wxCommandEvent& event)
{
	wxString f = event.GetString();
	mConf->set(CONFIG_LUA_SCRIPT, f);

	wxString s;
	s.Printf("lua file %s ", f);
	mParent->SetStatusText(s);

	SetReturnCode(1);
}
void luafile::OnLuaParameter(wxCommandEvent&  event)
{
	wxString p = event.GetString();
	mConf->set(CONFIG_LUA_PARAMETER, p);

	wxString s;
	s.Printf("lua parameter %s ", p.c_str());
	mParent->SetStatusText(s);

	SetReturnCode(1);
}
void luafile::reset(mxconf* mConf, bool all)
{
	wxFileName f;
	f.AssignCwd();
	f.AppendDir("lua");
	wxString luascriptfile;
	wxString luascriptparameter;

	luascriptfile = mConf->get(CONFIG_LUA_SCRIPT, "expresseur.lua");
	luascriptparameter = mConf->get(CONFIG_LUA_PARAMETER, "");

	f.SetFullName(luascriptfile);
	wxDateTime d = f.GetModificationTime();
	long dd = d.GetTicks();
	wxString s = f.GetFullPath();

	wxFileName flog;
	flog.SetPath(wxFileName::GetTempDir());
	flog.SetName("expresseur_log");

	basslua_open(s.c_str(), luascriptparameter.c_str(), all, dd, functioncallback, flog.GetFullPath().c_str());
}
void luafile::write(mxconf* mConf, wxTextFile *lfile)
{
	mConf->writeFile(lfile, CONFIG_LUA_SCRIPT, "expresseur.lua");
	mConf->writeFile(lfile, CONFIG_LUA_PARAMETER, "");
}
void luafile::read(mxconf* mConf , wxTextFile *lfile)
{
	mConf->readFile(lfile, CONFIG_LUA_SCRIPT, "expresseur.lua");
	mConf->readFile(lfile, CONFIG_LUA_PARAMETER, "");
}
void luafile::functioncallback()
{
	calledback = true;
}
bool luafile::isCalledback()
{
	if (calledback)
	{
		calledback = false;
		return true;
	}
	return false;
}
