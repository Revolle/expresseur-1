/////////////////////////////////////////////////////////////////////////////
// Name:        muscixmlscore.cpp
// Purpose:     display a musicxml of the score 
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 30/11/2016 22:00
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
#include "wx/dcbuffer.h"
#include "wx/xml/xml.h"
#include "wx/filefn.h"
#include "wx/wfstream.h"
#include "wx/zipstrm.h"
#include "wx/dynarray.h"
#include "wx/dynlib.h"

#include "global.h"
#include "luabass.h"
#include "basslua.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "expression.h"
#include "luafile.h"

#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"
#include "MNL.h"

enum
{
	IDM_MUSICXML_PANEL = ID_MUSICXML 
};

wxBEGIN_EVENT_TABLE(musicxmlscore, wxPanel)
EVT_LEFT_DOWN(musicxmlscore::OnLeftDown)
EVT_PAINT(musicxmlscore::onPaint)
EVT_SIZE(musicxmlscore::OnSize)
wxEND_EVENT_TABLE()

musicxmlscore::musicxmlscore(wxWindow *parent, wxWindowID id, mxconf* lconf, const wxDynamicLibrary  &myDll)
: viewerscore(parent, id)
{
	mParent = parent;
	mConf = lconf;
	nrChord = -1;
	xmlName.Clear();

	docID = 0;
	xmlCompile = NULL;

	this->SetBackgroundStyle(wxBG_STYLE_PAINT);

	MNLOpenDocument = NULL;
	MNLGetScoreInformation = NULL;
	MNLDisplayPage = NULL;
	MNLDisplayWrap = NULL;
	MNLGetPageSize = NULL;
	MNLNewPageLayout = NULL;
	MNLCloseDocument = NULL;
	MNLMeasureUnitsToPosition = NULL;
	MNLGetPageMeasures = NULL;

	MNLOpenDocument = (MNLOpenDocumentProc *)(myDll.GetSymbol("MNLOpenDocument"));
	MNLGetPageSize = (MNLGetPageSizeProc *)(myDll.GetSymbol("MNLGetPageSize"));
	MNLGetScoreInformation = (MNLGetScoreInformationProc *)(myDll.GetSymbol("MNLGetScoreInformation"));
	MNLDisplayPage = (MNLDisplayPageProc *)(myDll.GetSymbol("MNLDisplayPage"));
	MNLDisplayWrap = (MNLDisplayWrapProc *)(myDll.GetSymbol("MNLDisplayWrap"));
	MNLNewPageLayout = (MNLNewPageLayoutProc *)(myDll.GetSymbol("MNLNewPageLayout"));
	MNLCloseDocument = (MNLCloseDocumentProc *)(myDll.GetSymbol("MNLCloseDocument"));
	MNLMeasureUnitsToPosition = (MNLMeasureUnitsToPositionProc *)(myDll.GetSymbol("MNLMeasureUnitsToPosition"));
	MNLGetMeasurePosition = (MNLGetMeasurePositionProc *)(myDll.GetSymbol("MNLGetMeasurePosition"));
	MNLGetPageMeasures = (MNLGetPageMeasuresProc *)(myDll.GetSymbol("MNLGetPageMeasures"));
	MNLFindPosition = (MNLFindPositionProc *)(myDll.GetSymbol("MNLFindPosition"));

}
musicxmlscore::~musicxmlscore()
{
	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = NULL;

}
bool musicxmlscore::xmlIsOk()
{
	if (xmlCompile == NULL)
		return false;
	return (xmlCompile->isOk());
}
bool musicxmlscore::setFile(const wxFileName &lfilename, bool WXUNUSED(onstart))
{
	wxBusyCursor wait;

	if (!MNLOpenDocument)
		return false;

	prevPos = -1;
	pageNrLeft = -1;
	pageNrRight = -1;
	rectPrevPos.SetWidth(0);
	measurePage.Clear();

	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = new musicxmlcompile();

	wxFileName fm;
	fm.SetPath(wxFileName::GetTempDir());
	fm.SetFullName("expresseur_in.xml");
	xmlCompile->music_xml_complete_file = fm.GetFullPath();
	fm.SetFullName("expresseur_out.xml");
	xmlCompile->music_xml_displayed_file = fm.GetFullPath();

	if ((lfilename.GetExt() == SUFFIXE_MUSICXML) || (lfilename.GetExt() == SUFFIXE_MUSICMXL))
	{
		// extract the music_xml_complete_file from the xmlname file ( compressed with zipped or not )
		if (!xmlExtractXml(lfilename))
			return false;
		wxFileName txtfile(lfilename);
		txtfile.SetExt(SUFFIXE_TEXT);
		xmlCompile->setNameFile(txtfile, lfilename);
		// compile the musicxml-source-file, to create the music-xml-expresseur MUSICXML_FILE for display
		xmlCompile->loadXmlFile(xmlCompile->music_xml_complete_file, xmlCompile->music_xml_displayed_file, false);
	}
	if (lfilename.GetExt() == SUFFIXE_TEXT)
	{
		wxFileName xmlfile = xmlCompile->loadTxtFile(lfilename);
		if ((!xmlfile.IsOk()) || ((xmlfile.GetExt() != SUFFIXE_MUSICXML) && (xmlfile.GetExt() != SUFFIXE_MUSICMXL)))
			return false;
		xmlCompile->setNameFile(lfilename, xmlfile);
		// extract the music_xml_complete_file from the xmlname file ( compressed with zipped or not )
		if (!xmlExtractXml(xmlfile))
			return false;
		// compile the musicxml-source-file, to create the music-xml-expresseur MUSICXML_FILE for display
		xmlCompile->loadXmlFile(xmlCompile->music_xml_complete_file, xmlCompile->music_xml_displayed_file, true);
	}
	return xmlIsOk();
}
bool musicxmlscore::xmlExtractXml(wxFileName f)
{
	// extract musicXML file, not conpressed , in the file MUSICXML_COMPLETE_FILE

	wxBusyCursor wait;
	if ((f.GetExt() == SUFFIXE_MUSICXML) && (f.IsFileReadable()))
	{
		// xml file not compressed. Copy it directky to the temporary full score MUSICXML_COMPLETE_FILE
		if (!wxCopyFile(f.GetFullPath(), xmlCompile->music_xml_complete_file))
		{
			wxString s;
			s.sprintf("File %s cannot be copy in %s", f.GetFullPath(), xmlCompile->music_xml_complete_file);
			wxMessageBox(s);
			return false;
		}
		return true;
	}
	if ((f.GetExt() != SUFFIXE_MUSICMXL) || (!(f.IsFileReadable())))
		return false;
	// xml file compressed ( mxl ). unzip to the temporary full score MUSICXML_COMPLETE_FILE
	wxFFileInputStream in(f.GetFullPath());
	if (!in.IsOk())
	{
		wxString s;
		s.sprintf("Error opening stream file %s", xmlName.GetFullName());
		wxMessageBox(s);
		return false;
	}

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		wxString s;
		s.sprintf("Error reading zip structure of %s", xmlName.GetFullName());
		wxMessageBox(s);
		return false;
	}

	wxZipEntry *zipEntry;
	zipEntry = zip.GetNextEntry();
	while (zipEntry != NULL)
	{
		wxString name = zipEntry->GetName();
		delete zipEntry;
		wxFileName ffzip(name);
		if (ffzip.GetDirCount() == 0)
		{
			wxFileOutputStream  stream_out(xmlCompile->music_xml_complete_file);
			if (!stream_out.IsOk())
			{
				wxString s;
				s.sprintf("Error reading zip entry %s of %s", name, xmlName.GetFullName());
				wxMessageBox(s);
				return false;
			}
			zip.Read(stream_out);
			if (zip.LastRead() < 10)
			{
				wxString s;
				s.sprintf("Error content in zip entry %s of %s", name, xmlName.GetFullName());
				wxMessageBox(s);
				return false;
			}
			stream_out.Close();
			zip.CloseEntry();
			return true;
		}
		zipEntry = zip.GetNextEntry();
	}
	return false;
}
void musicxmlscore::zoom(int dzoom)
{
	fzoom -= 0.05 * float(dzoom);

	//newLayout();
	//this->Refresh();
}
bool musicxmlscore::displayFile()
{
	// display the music xml file

	if (!xmlIsOk())
		return false;
	
	xmlGetParam();

	if (docID)
		MNLCloseDocument(docID);
	docID = 0;
	sizeOk = false;
	char buffilename[MAXBUFCHAR];
	strcpy(buffilename, xmlCompile->music_xml_displayed_file.c_str());
	long retcode = MNLOpenDocument(buffilename, &docID);
	if (retcode != 0)
	{
		docID = 0;
		return false;
	}
	newLayout();

	Refresh();
	return true;
}
int musicxmlscore::getTrackCount()
{
	if (!xmlIsOk())
		return 0;
	return (xmlCompile->getTracksCount()); 
}
wxString musicxmlscore::getTrackName(int trackNr)
{
	if (!xmlIsOk())
		return "";
	wxString name;
	name = xmlCompile->getTrackName(trackNr);
	return (name);
}
void musicxmlscore::drawpage(int ipageNr, bool left , wxDC *dc)
{
	int pageNr = ipageNr;
	if ((pageNr < 1) || (pageNr > totalPages))
		pageNr = (left ? pageNrLeft : pageNrRight );
	if ((ipageNr == -1 ) || (pageNr != (left ? pageNrLeft : pageNrRight)))
	{
#ifdef RUN_WIN
		HWND mHwnd = (HWND)(dc->GetHandle());
#endif
#ifdef RUN_MAC
		CGContextRef mHwnd = (CGContextRef)(dc->GetHandle());
#endif
		MNLDisplayPage(docID, pageNr, left ? 0 : sizeX , 0, 1.0, 
			left ? 0 : sizeX , 0, left ? sizeX: 2*sizeX, sizeY, (long)(mHwnd));
		if ( left )
			pageNrLeft = pageNr;
		else
			pageNrRight = pageNr;
	}
}
bool musicxmlscore::setPage(int imeasureNr)
{
	int measureNr = imeasureNr;
	if (measureNr >= (int)(measurePage.GetCount()))
		measureNr = measurePage.GetCount() -1;
	int pageNr = measurePage[measureNr];
	bool left = ((pageNr % 2) == 1); 
	if ((pageNr == (left ? pageNrLeft : pageNrRight)) && (((pageNr + 1) > totalPages) || ((pageNr + 1) == (left ? pageNrRight : pageNrLeft))))
		return left;
	wxClientDC dc(this);
	wxBufferedDC bdc(&dc);
	drawpage(pageNr, left, &bdc);
	drawpage(pageNr + 1, (!left), &bdc);
	return left;
}
bool musicxmlscore::getScorePosition(int *absolute_measure_nr, int *measure_nr, int *beat, int *t)
{
	return xmlCompile->getScorePosition(currentNrEvent , absolute_measure_nr, measure_nr, beat, t);
}
void musicxmlscore::setCursor(int nrEvent, wxRect *rect, bool setRed, bool setGreen)
{
	if (nrEvent >= 0)
	{
		currentNrEvent = nrEvent;
		int measureNumber, units480;
		if (!(xmlCompile->getInfoEvent(nrEvent, &measureNumber, &units480)))
			return;
		bool left = setPage(measureNumber);
		long posX, posY;
		if (MNLGetMeasurePosition(docID, measureNumber, totalStaves, &posX, &posY) != ERCODE_NONE)
			return;
		long dX;
		if (MNLMeasureUnitsToPosition(docID, measureNumber, 1, units480, &dX) != ERCODE_NONE)
			return;
		rect->SetTopLeft(wxPoint(posX + dX - 5 + ((left) ? 0 : sizeX), posY - 10));
		rect->SetTopRight(wxPoint(posX + dX + 8 + ((left) ? 0 : sizeX ), posY - 10));
		rect->SetHeight(35);
	}

	wxClientDC dc(this);
	wxBufferedDC *bdc = new wxBufferedDC(&dc);
	wxColour col;
	int minx = rect->GetX();
	int miny = rect->GetY();
	int maxx = rect->GetX()  +rect->GetWidth();
	int maxy = rect->GetY() + rect->GetHeight();
	int prev_red = -1;
	int prev_green = -1;
	int green, red, blue;
	wxPen pen;
	bdc->SetPen(pen);
	for (int x = minx; x < maxx; x++)
	{
		for (int y = miny; y < maxy; y++)
		{
			bdc->GetPixel(x, y, &col);
			blue = col.Blue();
			if (blue < 250)
			{
				red = setRed ? 255 : blue;
				green = setGreen ? 255 : blue;
				if ((red != prev_red) || (green != prev_green))
				{
					pen.SetColour(red , green, blue);
					bdc->SetPen(pen);
				}
				bdc->DrawPoint(x, y);
			}
		}
	}
	delete bdc;
}
void musicxmlscore::setPosition(int pos, bool playing, bool quick)
{
	if ((!displayed) || ((pos == prevPos) && (playing == prevPlaying)))
		return;
	if (playing)
	{
		setCursor(-1, &rectPrevPos, false, false);
		setCursor(pos - 1, &rectPrevPos, true, false);
		prevPos = pos;
		prevPlaying = playing;
	}
	else
	{
		if ( !quick)
		{
			setCursor(-1, &rectPrevPos, false, false);
			setCursor(pos - 1, &rectPrevPos, false , true);
			prevPos = pos;
			prevPlaying = playing;
		}
	}
}
void musicxmlscore::OnLeftDown(wxMouseEvent& event)
{
	if (!docID)
		return;
	wxClientDC dc(this);
	wxPoint mPoint = event.GetLogicalPosition(dc);
	bool left = ( mPoint.x < sizeX);
	drawpage(-1, left, &dc);
	long measureNumber;
	long beat;
	long staffNumber;
	if (MNLFindPosition(docID, mPoint.x, mPoint.y, &measureNumber, &beat, &staffNumber) != ERCODE_NONE)
		return;
	int nrEvent = xmlCompile->measureBeatToEventNr(measureNumber, beat);
	prevPos = -1;
	prevPlaying = -1;
	if (nrEvent != -1)
		basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
	else
		gotoPosition();
}
void musicxmlscore::gotoPosition()
{
	wxTextEntryDialog mdialog(NULL, "goto measure ( or label )", "goto");
	if (mdialog.ShowModal() == wxID_OK)
	{
		wxString sgoto = mdialog.GetValue();
		int nrEvent = xmlCompile->stringToEventNr(sgoto);
		if (nrEvent != -1)
			basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
	}
}
void musicxmlscore::OnSize(wxSizeEvent& WXUNUSED(event))
{
	newLayout();
}
void musicxmlscore::newLayout()
{
	if (!docID)
		return;
	wxClientDC dc(this);
	sizeClient = dc.GetSize();
	//wxSize mmsizeClient = dc.GetSizeMM();
	if ((sizeClient.GetWidth() < 200) || (sizeClient.GetHeight() < 100))
		return;
	sizeX = sizeClient.GetX() / 2 ;
	sizeY = sizeClient.GetY();
	int wmm = (127 * 1 * sizeX ) / 360;
	int hmm = (170 * 1 * sizeY ) / 360;
	MNLNewPageLayout(docID, wmm , hmm, 1.0, 5, 5, 5, 5, 0, 1);
	sizeOk = true;
	GetScoreInfo();
	pageNrLeft = 1;
	pageNrRight = 2;
	prevPos = -1;
	pageNrLeft = -1;
	pageNrRight = -1;
	rectPrevPos.SetWidth(0);
}
void musicxmlscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	if ((!docID) || (!sizeOk) || (!xmlIsOk()))
		return;
	wxAutoBufferedPaintDC dc(this);
	sizeOk = false;
	dc.Clear();
	drawpage(-1, false, &dc);
	drawpage(-1, true, &dc);
	prevPos = -1;
	rectPrevPos.SetWidth(0);
	displayed = true;
}
bool musicxmlscore::GetScoreInfo()
{
	if ((!docID) || (!sizeOk) || (!xmlIsOk()))
		return false;

	char *title;
	long ret = MNLGetScoreInformation(docID, &totalMeasures, &totalStaves, &totalPages, &title);
	if (ret == ERCODE_NONE)
	{
		measurePage.Clear();
		measurePage.Add(0);
		for (long pageNr = 1; pageNr <= totalPages; pageNr++)
		{
			long firstMeasure, lastMeasure;
			ret = MNLGetPageMeasures(docID, pageNr, &firstMeasure, &lastMeasure);
			if (ret == ERCODE_NONE)
			{
				for (int n = firstMeasure; n <= lastMeasure; n++)
					measurePage.Add(pageNr);
			}
			else
			{
				wxMessageBox("can't get information about mesures in pages of the musicXml document","Error musicXml");
				MNLCloseDocument(docID);
				docID = 0;
				return false;
			}
		}
		/*
		for (unsigned int i = 0; i < measurePage.GetCount(); i++)
		{
			int p;
			p = measurePage[i];
			p = p + 0;
		}
		*/
		return true;
	}
	else
	{
		wxMessageBox("can't get information about the musicXml document", "Error musicXml");
		MNLCloseDocument(docID);
		docID = 0;
		return false;
	}
}
void musicxmlscore::xmlGetParam()
{
	// get the mixer parameters

	unsigned int partNb = xmlCompile->getTracksCount();
	wxArrayString names = xmlCompile->getTracksName();
	wxArrayInt display = xmlCompile->getTracksDisplay();
	wxArrayInt play = xmlCompile->getTracksPlay();
	wxASSERT((names.GetCount() == partNb) && (display.GetCount() == partNb) && (play.GetCount() == partNb));
}

