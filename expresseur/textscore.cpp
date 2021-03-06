/////////////////////////////////////////////////////////////////////////////
// Name:        textscore.cpp
// Purpose:     display a text of the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 31 / 10 / 2016 10 : 00
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
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/listctrl.h"
#include "wx/valgen.h"
#include "wx/listbox.h"
#include "wx/tokenzr.h"
#include "wx/config.h"
#include "wx/dcclient.h"
#include "wx/msgdlg.h"
#include "wx/image.h"
#include "wx/filehistory.h"

#include "global.h"
#include "luabass.h"
#include "basslua.h"
#include "mxconf.h"
#include "expression.h"
#include "textscore.h"
#include "luafile.h"

wxBEGIN_EVENT_TABLE(textscore, wxPanel)
EVT_SIZE(textscore::OnSize)
wxEND_EVENT_TABLE()

textscore::textscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf)
: wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxSUNKEN_BORDER | wxTE_RICH | wxTE_DONTWRAP)
{
	mParent = parent;
	mConf = lMxconf;
	
	textAttrRecognized.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttrNormal.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttrPosition.SetFontFamily(wxFONTFAMILY_TELETYPE);

	textAttrRecognized.SetTextColour(*wxBLUE);
	textAttrNormal.SetTextColour(*wxBLACK);
	textAttrPosition.SetTextColour(*wxRED);

	setMyFontSize(mConf->get(CONFIG_FONTSIZE, 12));

	textAttrRecognized.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);
	textAttrNormal.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);
	textAttrPosition.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);

	SetDefaultStyle(textAttrNormal);

	oldchordStart = -1;
	oldchordEnd = -1;
	oldsectionStart = -1;
	oldsectionEnd = -1;
}
textscore::~textscore()
{
}
void textscore::compileText()
{
	if (oldText == GetValue())
		return;

	bool userModification = IsModified();
	oldText = GetValue();
	char buf[5000];
	strcpy(buf, oldText.c_str());
	basslua_call(moduleChord, functionChordSetScore, "s", buf);
	int lenText = oldText.Length();
	SetStyle(0, lenText, textAttrNormal);
	int start = -1;
	int end = -1 ;
	basslua_call(moduleChord, functionChordGetRecognizedScore, ">ii", &start, &end);
	while (start >= 0)
	{
		if ( start > 0 )
			SetStyle(start - 1, end, textAttrRecognized);
		basslua_call(moduleChord, functionChordGetRecognizedScore, ">ii", &start, &end);
	}
	if (!userModification)
		DiscardEdits();
	
	oldchordStart = -1;
	oldchordEnd = -1;
	oldsectionStart = -1;
	oldsectionEnd = -1;
}
int textscore::scanPosition(bool editmode)
{
	int sectionStart, sectionEnd, chordStart, chordEnd , nrChord;
	bool userModification = this->IsModified();
	basslua_call(moduleChord, functionChordGetPosition, ">iiiii", &sectionStart, &sectionEnd, &chordStart, &chordEnd, &nrChord);
	if (nrChord >= 0)
	{
		if ((sectionStart != oldsectionStart) || (sectionEnd != oldsectionEnd))
		{
			if (oldsectionStart > 0)
				SetStyle(oldsectionStart - 1, oldsectionEnd, textAttrRecognized);
			if (sectionStart > 0)
				SetStyle(sectionStart - 1, sectionEnd, textAttrPosition);
			oldsectionStart = sectionStart;
			oldsectionEnd = sectionEnd;
		}
		if ((chordStart != oldchordStart) || (chordEnd != oldchordEnd))
		{
			if (oldchordStart > 0)
				SetStyle(oldchordStart - 1, oldchordEnd, textAttrRecognized);
			if (chordStart > 0)
				SetStyle(chordStart - 1, chordEnd, textAttrPosition);
			oldchordStart = chordStart ;
			oldchordEnd = chordEnd;
			if (!editmode)
				ShowPosition(chordStart);
		}
	}
	if (! userModification)
		DiscardEdits();
	return nrChord;
}
int textscore::getInsertionPoint()
{
	return GetInsertionPoint();
}
bool textscore::setFile(const wxFileName &filename)
{
	bool retcode = false;
	oldText.Empty();
	Clear();
	SetDefaultStyle(textAttrNormal);
	wxFileName filetext(filename);
	filetext.SetExt(SUFFIXE_TEXT);
	if (!filetext.FileExists())
	{
		wxTextFile f(filetext.GetFullPath());
		if (!f.Create())
			return false;
		f.Close();
	}
	if (LoadFile(filetext.GetFullPath()))
	{
		oldchordStart = -1;
		oldchordEnd = -1;
		oldsectionStart = -1;
		oldsectionEnd = -1;
		retcode = true;
	}
	SetModified(false);
	return retcode;
}
void textscore::saveFile(const wxFileName &filename)
{
	wxFileName f(filename);
	f.SetExt(SUFFIXE_TEXT);
	wxString s = f.GetFullPath();
	if (SaveFile(s))
	{
		SetModified(false);
	}
}
bool textscore::needToSave()
{
	return this->IsModified();
}
void textscore::noNeedToSave()
{
	SetModified(false);
}
void textscore::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Refresh();
}
int textscore::getFontSize()
{
	return sizeFont;
}
void textscore::setEditMode(bool editMode)
{
	if (editMode)
		SetBackgroundColour(*wxWHITE);
	else
		SetBackgroundColour(wxColour(220, 220, 220, wxALPHA_OPAQUE));
}
void textscore::setMyFontSize(int t)
{
	sizeFont = t;
	if (sizeFont < 4)
		sizeFont = 4;
	if (sizeFont > 40)
		sizeFont = 40;


	textAttrRecognized.SetFontSize(sizeFont);
	textAttrNormal.SetFontSize(sizeFont);
	textAttrPosition.SetFontSize(sizeFont);

	SetDefaultStyle(textAttrNormal);
	
	mConf->set(CONFIG_FONTSIZE, sizeFont);
}
void textscore::zoom(int d)
{
	setMyFontSize(sizeFont + d);
	wxTextAttr textAttr;
	textAttr.SetFontSize(sizeFont);
	textAttr.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttr.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE);
	SetStyle(0, 10000, textAttr);
}