/////////////////////////////////////////////////////////////////////////////
// Name:        muscixmlpage.cpp
// Purpose:     display a musicxml page of the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     04/08/2015
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////


#include "global.h"

#ifdef	RUN_WIN

#define 	WIN32_LEAN_AND_MEAN
#include 	<windows.h>
#include 	<stdio.h>
#include 	<CommDlg.h>
#include 	<ShellAPI.h>
#define		PREFMACC

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);


#endif

#ifdef	RUN_MAC

#import <Cocoa/Cocoa.h>
#include 	<Carbon/Carbon.h>

@interface MNLAppDelegate : NSObject {
	NSWindow		*window;

}
-(void)applicationIdle;
@end

@interface NSMNLView : NSView {

}
@end

#define		PREFMACC	extern "C"

#endif

#include  "musicxmlpage.h"

musicxmlpage::musicxmlpage(HWND lhWnd, HMODULE myDLL)
{
	hWnd = lhWnd;
	docID = 0;
	scalefactor = 1.0;
	barNr = 0;
	mDll = myDLL;

	status = MNLLoad();
}
musicxmlpage::~musicxmlpage()
{
	if (myDll)
	{
		if (docID)
			MNLCloseDocument(docID);
		if (MNLQuit)
			MNLQuit();
		FreeLibrary(myDll);
		myDll = NULL;
		return ERCODE_NONE;
	}
	return ERCODE_DLL_IS_NOT_LOADED;
}
bool musicxmlpage::isOK()
{
	return (status == ERCODE_NONE);
}
void musicxmlpage::onSize()
{
	RECT r;
	GetClientRect(hWnd, &r);
	width = r.right - r.left;
	height = r.bottom - r.top ;
}
int musicxmlpage::setBarNr(long n)
{
	barNr = n;
	long totalSystems, totalMeasuresTable[100], systemHeightTable[100] ;
	totalSystems = 100;
	long r = MNLSimulateDisplayWrap(docID, barNr, 0, 0, scalefactor, 0, 0, width, height, (float)(MINSCALEBAR), (float)(MAXSCALEBAR), &totalSystems, totalMeasuresTable, systemHeightTable);
	if (r != ERCODE_NONE)
		return -1;
	int nbBar = 0;
	int padding = 0;
	for (int i = 1; i < totalSystems; i++)
	{
		nbBar += totalMeasuresTable[i];
		padding += systemHeightTable[i];
	}
	if (totalSystems > 1)
		padding = (height - padding) / (totalSystems - 1) - 5;
	else
		padding = height - padding - 5 ;
	return nbBar;
}
void musicxmlpage::setScalefactor(float f)
{
	scalefactor = f;
}
bool musicxmlpage::setFile(const char*fileName)
{
	char f[1024];
	strcpy(f, fileName);
	MNLOpenDocumentProc	*lMNLOpenDocument = (MNLOpenDocumentProc *)GetProcAddress(myDll, "MNLOpenDocument");

	long ercode = lMNLOpenDocument(f, &docID);
	if (ercode != ERCODE_NONE)
		docID = 0;
	return ercode;
}
void musicxmlpage::drawPage()
{
	HDC graphic = GetDC(hWnd);
	MNLDisplayWrap(docID, barNr, nbBar, 0, 0, scalefactor, 0, 0, width, height, long(graphic), (float)(MINSCALEBAR), (float)(MAXSCALEBAR), padding);
}
long musicxmlpage::MNLLoad()
{
	long 		k;

	if (myDll)
		return ERCODE_DLL_ALREADY_LOADED;


	if (myDll)
	{	// Section I-A - Loading the library

		MNLInit = (MNLInitProc *)GetProcAddress(myDll, "MNLLoad");
		MNLQuit = (MNLQuitProc *)GetProcAddress(myDll, "MNLRelease");

		// Section I-B - Loading and saving documents

		MNLOpenDocument = (MNLOpenDocumentProc *)GetProcAddress(myDll, "MNLOpenDocument");
		MNLOpenDocumentFromBuffer = (MNLOpenDocumentFromBufferProc *)GetProcAddress(myDll, "MNLOpenDocumentFromBuffer");
		MNLCloseDocument = (MNLCloseDocumentProc *)GetProcAddress(myDll, "MNLCloseDocument");
		MNLSaveDocument = (MNLSaveDocumentProc *)GetProcAddress(myDll, "MNLSaveDocument");
		MNLSaveDocumentToBuffer = (MNLSaveDocumentToBufferProc *)GetProcAddress(myDll, "MNLSaveDocumentToBuffer");

		// Section I-C - Getting information about a document

		MNLGetScoreInformation = (MNLGetScoreInformationProc *)GetProcAddress(myDll, "MNLGetScoreInformation");
		MNLGetPageSize = (MNLGetPageSizeProc *)GetProcAddress(myDll, "MNLGetPageSize");
		MNLGetPageMeasures = (MNLGetPageMeasuresProc *)GetProcAddress(myDll, "MNLGetPageMeasures");
		MNLGetKeySignature = (MNLGetKeySignatureProc *)GetProcAddress(myDll, "MNLGetKeySignature");
		MNLGetTimeSignature = (MNLGetTimeSignatureProc *)GetProcAddress(myDll, "MNLGetTimeSignature");

		// Section I-D - Displaying the score

		MNLDisplayPage = (MNLDisplayPageProc *)GetProcAddress(myDll, "MNLDisplayPage");
		MNLDisplayLinear = (MNLDisplayLinearProc *)GetProcAddress(myDll, "MNLDisplayLinear");
		MNLDisplayWrap = (MNLDisplayWrapProc *)GetProcAddress(myDll, "MNLDisplayWrap");
		MNLSimulateDisplayWrap = (MNLSimulateDisplayWrapProc *)GetProcAddress(myDll, "MNLSimulateDisplayWrap");
		MNLFindPosition = (MNLFindPositionProc *)GetProcAddress(myDll, "MNLFindPosition");
		MNLGetMeasurePosition = (MNLGetMeasurePositionProc *)GetProcAddress(myDll, "MNLGetMeasurePosition");
		MNLMeasureUnitsToPosition = (MNLMeasureUnitsToPositionProc *)GetProcAddress(myDll, "MNLMeasureUnitsToPosition");
		MNLMeasurePositionToUnits = (MNLMeasurePositionToUnitsProc *)GetProcAddress(myDll, "MNLMeasurePositionToUnits");

		// Section I-E - Modifying the score

		MNLTranspose = (MNLTransposeProc *)GetProcAddress(myDll, "MNLTranspose");
		MNLTransposeByInterval = (MNLTransposeByIntervalProc *)GetProcAddress(myDll, "MNLTransposeByInterval");
		MNLNewPageLayout = (MNLNewPageLayoutProc *)GetProcAddress(myDll, "MNLNewPageLayout");

		// Section I-F - Playing the score

		MNLOpenMidiPort = (MNLOpenMidiPortProc *)GetProcAddress(myDll, "MNLOpenMidiPort");
		MNLCloseMidiPort = (MNLCloseMidiPortProc *)GetProcAddress(myDll, "MNLCloseMidiPort");
		MNLOpenSoundLibrary = (MNLOpenSoundLibraryProc *)GetProcAddress(myDll, "MNLOpenSoundLibrary");
		MNLCloseSoundLibrary = (MNLCloseSoundLibraryProc *)GetProcAddress(myDll, "MNLCloseSoundLibrary");
		MNLIdle = (MNLIdleProc *)GetProcAddress(myDll, "MNLIdle");
		MNLStartPlaying = (MNLStartPlayingProc *)GetProcAddress(myDll, "MNLStartPlaying");
		MNLStopPlaying = (MNLStopPlayingProc *)GetProcAddress(myDll, "MNLStopPlaying");
		MNLGetPlayingPosition = (MNLGetPlayingPositionProc *)GetProcAddress(myDll, "MNLGetPlayingPosition");
		MNLSetPlaybackOptions = (MNLSetPlaybackOptionsProc *)GetProcAddress(myDll, "MNLSetPlaybackOptions");
		MNLGetPlaybackParameter = (MNLGetPlaybackParameterProc *)GetProcAddress(myDll, "MNLGetPlaybackParameter");
		MNLSetPlaybackParameter = (MNLSetPlaybackParameterProc *)GetProcAddress(myDll, "MNLSetPlaybackParameter");
		MNLDisplayKeyboard = (MNLDisplayKeyboardProc *)GetProcAddress(myDll, "MNLDisplayKeyboard");
		MNLKeyboardMidiFilter = (MNLKeyboardMidiFilterProc *)GetProcAddress(myDll, "MNLKeyboardMidiFilter");
		MNLRefreshKeyboard = (MNLRefreshKeyboardProc *)GetProcAddress(myDll, "MNLRefreshKeyboard");
		MNLReleaseKeyboard = (MNLReleaseKeyboardProc *)GetProcAddress(myDll, "MNLReleaseKeyboard");
		MNLDisplayFretboard = (MNLDisplayFretboardProc *)GetProcAddress(myDll, "MNLDisplayFretboard");
		MNLFretboardMidiFilter = (MNLFretboardMidiFilterProc *)GetProcAddress(myDll, "MNLFretboardMidiFilter");
		MNLRefreshFretboard = (MNLRefreshFretboardProc *)GetProcAddress(myDll, "MNLRefreshFretboard");
		MNLReleaseFretboard = (MNLReleaseFretboardProc *)GetProcAddress(myDll, "MNLReleaseFretboard");
		MNLResetMidi = (MNLResetMidiProc *)GetProcAddress(myDll, "MNLResetMidi");

		if (MNLInit)
		{
			k = MNLInit("MNL folder", "8FF2B7A1BD5ACCA");
			if (k == ERCODE_NONE)
				return ERCODE_NONE;
			MNLRelease();
			return k;
		}

		MNLRelease();
	}

	return ERCODE_COULD_NOT_LOAD_DLL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

long musicxmlpage::MNLRelease(void)
{
}
