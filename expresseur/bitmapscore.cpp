/////////////////////////////////////////////////////////////////////////////
// Name:        bitmapscore.cpp
// Purpose:     display a bitmap of the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 15/11/2016 18:00
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
#include "viewerscore.h"
#include "expression.h"
#include "bitmapscore.h"

wxBEGIN_EVENT_TABLE(bitmapscore, wxPanel)
EVT_PAINT(bitmapscore::onPaint)
EVT_SIZE(bitmapscore::OnSize)
EVT_LEFT_DOWN(bitmapscore::OnLeftDown)
EVT_LEFT_UP(bitmapscore::OnLeftUp)
EVT_MOUSE_EVENTS(bitmapscore::OnMouse)
wxEND_EVENT_TABLE()

bitmapscore::bitmapscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf)
: viewerscore(parent, id)
{
	mParent = parent;
	mConf = lMxconf;
	mImage = NULL;
	mPointStart = wxDefaultPosition;
	alertSetRect = true;
	selectedRect.SetWidth(0);
	nbRectChord = 0;
	nrChord = -1; 
	prevNrChord = -1;
	for (int i = 0; i < MAX_RECTCHORD; i++)
	{
		rectChord[i].SetX(0);
		rectChord[i].SetY(0);
		rectChord[i].SetWidth(0);
		rectChord[i].SetHeight(0);
	}
	xScale = 1.0;
	yScale = 1.0;

}
bitmapscore::~bitmapscore()
{
	if ( mImage )
		delete mImage;
}
bool bitmapscore::setFile(const wxFileName &lfilename , bool onstart)
{
	// load the image
	wxFileName filename(lfilename);
	filename.SetExt(SUFFIXE_BITMAPCHORD);
	bool retcode = false;
	if (mImage)
		delete mImage;
	mImage = NULL;
	if (filename.IsFileReadable())
	{
		mImage = new wxImage(filename.GetFullPath());
		if (mImage->IsOk())
		{
			retcode = true;
			// load the rect linked to the chords
			fileRectChord = filename;
			fileRectChord.SetExt("txb");
			readRectChord(onstart);
		}
	}
	if (retcode == false)
	{
		if (mImage)
			delete mImage;
		mImage = NULL;
	}
	Refresh();
	return retcode;
}
bool bitmapscore::displayFile()
{
	Refresh();
	return true;
}
int bitmapscore::getTrackCount()
{
	return 0;
}
wxString bitmapscore::getTrackName(int WXUNUSED(trackNr))
{
	return wxEmptyString;
}

void bitmapscore::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Refresh();
}
void bitmapscore::newLayout()
{
}

void bitmapscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	if (mImage == NULL)
	{
		return;
	}

	wxSize sizeClient = dc.GetSize();
	wxSize sizeImage = mImage->GetSize();
	wxSize sizeDisplay;

	sizeDisplay.SetWidth(sizeClient.GetWidth());
	sizeDisplay.SetHeight((sizeImage.GetHeight()*sizeClient.GetWidth()) / sizeImage.GetWidth());
	if (sizeDisplay.GetHeight() > sizeClient.GetHeight())
	{
		sizeDisplay.SetHeight(sizeClient.GetHeight());
		sizeDisplay.SetWidth((sizeImage.GetWidth()*sizeClient.GetHeight()) / sizeImage.GetHeight());
	}
	xScale = (double)(sizeDisplay.GetWidth()) / (double)(sizeImage.GetWidth());
	yScale = (double)(sizeDisplay.GetHeight()) / (double)(sizeImage.GetHeight());
	wxImage lImage = mImage->Scale(sizeDisplay.GetWidth(), sizeDisplay.GetHeight(), wxIMAGE_QUALITY_HIGH);
	wxBitmap mBitmap(lImage);
	dc.DrawBitmap(mBitmap,0,0);
	prevNrChord = -1;
	nrChord = -1;
}
void bitmapscore::OnLeftDown(wxMouseEvent& event)
{
	wxClientDC mDC(this);
	mDC.SetUserScale(xScale, yScale);
	mPointStart = event.GetLogicalPosition(mDC);
}
void bitmapscore::OnLeftUp(wxMouseEvent& event)
{
	wxClientDC mDC(this);
	mDC.SetUserScale(xScale, yScale);
	mPointEnd = event.GetLogicalPosition(mDC);
	selectedRect = highlight(false, mPointStart, mPointEnd, &mDC);
	mPointStart = wxDefaultPosition;
	if (((selectedRect.GetWidth() < 3) && (selectedRect.GetHeight() < 3)) || (nrChord == -1))
	{
		selectedRect.SetWidth(5);
		selectedRect.SetHeight(5);
		wxRect r;
		for (int nrRectChord = 0; nrRectChord < nbRectChord; nrRectChord++)
		{
			r = rectChord[nrRectChord] * selectedRect;
			if (!r.IsEmpty())
			{
				basslua_call(moduleChord, functionChordSetNrEvent, "i", nrRectChord);
				break;
			}
		}
	}
	else
	{
		if (alertSetRect)
		{
			wxMessageDialog *mDialog = new wxMessageDialog(this, _("link this rectangle to the current chord ?"), _("Score link"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION | wxCENTRE);
			mDialog->SetYesNoCancelLabels(_("Always link"), _("Link now"), _("Cancel"));
			switch (mDialog->ShowModal())
			{
			case wxID_YES:
				alertSetRect = false;
			case wxID_NO:
				break;
			default:
				return;
			}
		}
		rectChord[nrChord] = selectedRect;
		if (nrChord >= nbRectChord)
			nbRectChord = nrChord + 1;
		writeRectChord();
		Refresh();
	}
}
wxRect bitmapscore::highlight(bool on, wxPoint start, wxPoint end , wxDC *lDC)
{
	wxRect mRect;
	if (end.x >= mPointStart.x)
	{
		mRect.SetX(start.x);
		mRect.SetWidth(end.x - mPointStart.x);
	}
	else
	{
		mRect.SetX(end.x);
		mRect.SetWidth(mPointStart.x - end.x);
	}
	if (end.y >= mPointStart.y)
	{
		mRect.SetY(start.y);
		mRect.SetHeight(end.y - mPointStart.y);
	}
	else
	{
		mRect.SetY(end.y);
		mRect.SetHeight(mPointStart.y - end.y);
	}
	if (on)
	{
		lDC->SetLogicalFunction(wxINVERT);
		if (! prevRect.IsEmpty())
			lDC->DrawRectangle(prevRect);
		lDC->DrawRectangle(mRect);
		prevRect = mRect;
	}
	else
	{
		lDC->SetLogicalFunction(wxINVERT);
		lDC->DrawRectangle(prevRect);
		prevRect.SetWidth(0);
	}
	return mRect;
}
void bitmapscore::OnMouse(wxMouseEvent& event)
{
	if ((event.Dragging()) && ( mPointStart != wxDefaultPosition))
	{
		wxClientDC mDC(this);
		mDC.SetUserScale(xScale, yScale);
		wxPoint mPos = event.GetLogicalPosition(mDC);
		highlight(true, mPointStart, mPos, &mDC);
	}
}
void bitmapscore::readRectChord(bool onstart)
{
	wxTextFile      tfile;
	nbRectChord = 0;
	if (fileRectChord.IsFileReadable() == true)
	{

		tfile.Open(fileRectChord.GetFullPath());
		if (tfile.IsOpened() == false)
			return;
		wxString str = tfile.GetFirstLine(); // LIST_RECT
		str = tfile.GetNextLine();
		wxString token;
		long l;
		while (!tfile.Eof())
		{
			wxStringTokenizer tokenizer(str, ";");
			if (tokenizer.CountTokens() == 4)
			{
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetX(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetY(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetWidth(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetHeight(l);
			}
			nbRectChord++;
			str = tfile.GetNextLine();
		}
		tfile.Close();
	}

	if ((onstart == false ) && (nbRectChord == 0 ))
	{
		if (mConf->get(CONFIG_BITMAPSCOREWARNINGTAGIMAGE, 1) == 1)
		{
			wxMessageDialog *mDialog = new wxMessageDialog(this, _("No chord tagged in the image"), _("Score image"), wxYES | wxNO | wxHELP | wxICON_INFORMATION | wxCENTRE);
			mDialog->SetYesNoLabels(_("OK"), _("Don't show again this message"));
			switch (mDialog->ShowModal())
			{
			case wxID_NO:
				mConf->set(CONFIG_BITMAPSCOREWARNINGTAGIMAGE, 0);
				break;
			case wxID_HELP:
				wxLaunchDefaultBrowser("http://www.expresseur.com/help/imagechord.html");
				break;
			default:
				break;
			}
		}
	}
}
void bitmapscore::writeRectChord()
{
	wxString s;
	wxFileName f;
	wxTextFile tfile;
	if (fileRectChord.IsFileWritable() == false)
		tfile.Create(fileRectChord.GetFullPath());
	tfile.Open(fileRectChord.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	tfile.Clear();
	tfile.AddLine(LIST_RECT);
	for (int nrRectChord = 0; nrRectChord < nbRectChord; nrRectChord++)
	{
		s.Printf("%d;%d;%d;%d", rectChord[nrRectChord].GetX(), rectChord[nrRectChord].GetY(), rectChord[nrRectChord].GetWidth(), rectChord[nrRectChord].GetHeight());
		tfile.AddLine(s);
	}
	tfile.Write();
	tfile.Close();
}
void bitmapscore::highighNrChord(int nrChord)
{
	wxClientDC mDC(this);
	mDC.SetLogicalFunction(wxINVERT);
	mDC.SetUserScale(xScale, yScale);
	if ((prevNrChord != -1) && (!rectChord[prevNrChord].IsEmpty()))
		mDC.DrawRectangle(rectChord[prevNrChord]);
	if (!rectChord[nrChord].IsEmpty())
		mDC.DrawRectangle(rectChord[nrChord]);
	prevNrChord = nrChord;

}
void bitmapscore::setPosition(int pos, bool WXUNUSED( playing), bool WXUNUSED(quick))
{
	if ((pos < 0) || (pos > MAX_RECTCHORD))
		return;

	if (nrChord != pos)
	{
		nrChord = pos;
		highighNrChord(nrChord);
	}
}
void bitmapscore::zoom(int WXUNUSED(dzoom))
{

}
void bitmapscore::gotoPosition()
{

}
