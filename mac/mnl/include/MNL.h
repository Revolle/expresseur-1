/************************************************************************
   SOURCE HEADER FILE
   MUSIC NOTATION LIBRARY LEVEL I
   
   Version 1.1.0 of 7 November 2012
   
************************************************************************/

#ifndef __MUSIC_NOTATION_LIBRARY_DLL_H__
#define __MUSIC_NOTATION_LIBRARY_DLL_H__

// Error codes

#define		ERCODE_NONE									0
#define		ERCODE_COULD_NOT_LOAD_DLL					1
#define		ERCODE_DLL_IS_NOT_LOADED					2
#define		ERCODE_DLL_ALREADY_LOADED					3
#define		ERCODE_COULD_NOT_OPEN_FILE_FOR_READING		4
#define		ERCODE_FORMAT_ERROR_IN_PIZ_FILE				5
#define		ERCODE_COULD_NOT_OPEN_CHORD_LIBRARY			6
#define		ERCODE_COULD_NOT_OPEN_RESOURCE_FILE			7
#define		ERCODE_COULD_NOT_IMPORT_MIDI_FILE			8
#define		ERCODE_COULD_NOT_OPEN_TABLE_FONT			9
#define		ERCODE_COULD_NOT_IMPORT_MUSICXML_FILE		10
#define		ERCODE_INVALID_DOC_ID						11
#define		ERCODE_INVALID_TYPE_OF_FILE					12
#define		ERCODE_COULD_NOT_SAVE_PIZ_FILE				13
#define		ERCODE_COULD_NOT_SAVE_MID_FILE				14
#define		ERCODE_COULD_NOT_SAVE_XML_FILE				15
#define		ERCODE_COULD_NOT_CREATE_PDF_FILE			16
#define		ERCODE_NO_PAGE_LAYOUT_DEFINED				17
#define		ERCODE_IMPOSSIBLE_TO_CREATE_TEMP_FILE		18
#define		ERCODE_IMPOSSIBLE_TO_WRITE_TEMP_FILE		19
#define		ERCODE_BUFFER_TOO_SMALL						20
#define		ERCODE_IMPOSSIBLE_TO_OPEN_TEMP_FILE			21
#define		ERCODE_INVALID_PAGE_NUMBER					22
#define		ERCODE_INVALID_KEYSIG_INDEX					23
#define		ERCODE_INVALID_TIMESIG_INDEX				24
#define		ERCODE_INVALID_RANGE_OF_MEASURES			25
#define		ERCODE_ALREADY_PLAYING						26
#define		ERCODE_NO_SCORE_PLAYING						27
#define		ERCODE_INVALID_MIDI_PORT_INDEX				28
#define		ERCODE_MIDI_PORT_ALREADY_OPEN				29
#define		ERCODE_NO_MIDI_PORT_DEFINED					30
#define		ERCODE_STILL_PLAYING						31
#define		ERCODE_NO_MEASURES_IN_PAGE					32
#define		ERCODE_INVALID_STAFF_NUMBER					33
#define		ERCODE_INVALID_PARAMETER_NUMBER				34
#define		ERCODE_INVALID_PARAMETER_VALUE				35
#define		ERCODE_NO_SCORE_DISPLAYED					36
#define		ERCODE_POSITION_NOT_INSIDE_MEASURE			37
#define		ERCODE_KEYBOARD_ALREADY_DEFINED				38
#define		ERCODE_NO_KEYBOARD_DEFINED					39
#define		ERCODE_FRETBOARD_ALREADY_DEFINED			40
#define		ERCODE_NO_FRETBOARD_DEFINED					41
#define		ERCODE_LIBRARY_ALREADY_LOADED				42
#define		ERCODE_NO_LIBRARY_LOADED					43
#define		ERCODE_MIDI_RUNNING							44
#define		ERCODE_COULD_NOT_LOAD_LIBRARY				45
#define		ERCODE_INVALID_MODE_SPECIFICATION			46
#define		ERCODE_COULD_NOT_CREATE_AUDIO_FILE			47
#define		ERCODE_OUT_OF_MEMORY						48
#define		ERCODE_INVALID_INTERVAL_SPECIFICATION		49
#define		ERCODE_COULD_NOT_CREATE_DOCUMENT			50
#define		ERCODE_INVALID_MEASURE_NUMBER				51
#define		ERCODE_INVALID_OBJECT_INDEX					52
#define		ERCODE_INVALID_OBJECT						53
#define		ERCODE_INVALID_LYRIC_LINE_NUMBER			54
#define		ERCODE_INVALID_CHORD_INDEX					55
#define		ERCODE_INVALID_MIDIEVENT_INDEX				56
#define		ERCODE_INTERNAL_ERROR						57
#define		ERCODE_INVALID_RANGE_OF_STAVES				58
#define		ERCODE_INVALID_SYSTEM_NUMBER				59
#define		ERCODE_INVALID_SDK_LICENCE					60
#define		ERCODE_MNL_ALREADY_LOADED					61
#define		ERCODE_MNL_NOT_LOADED						62

// Type of callback function to call when playback is finished

typedef	void (callbackWhenFinishedProc)(void);

// Section I-A - Loading the library

long	MNLLoad(char *path,char *validationCode);
long	MNLRelease(void);

// Section I-B - Loading and saving documents

typedef	long (MNLOpenDocumentProc)(char *pathname,long *docID);
typedef	long (MNLOpenDocumentFromBufferProc)(void *buffer,long bufSize,long *docID);
typedef	long (MNLCloseDocumentProc)(long docID);
typedef	long (MNLSaveDocumentProc)(long docID,char *pathname,long typeOfFile);
typedef	long (MNLSaveDocumentToBufferProc)(long docID,void *buffer,long *bufSize,long typeOfFile);

extern	MNLOpenDocumentProc					*MNLOpenDocument;
extern	MNLOpenDocumentFromBufferProc		*MNLOpenDocumentFromBuffer;
extern	MNLCloseDocumentProc				*MNLCloseDocument;
extern	MNLSaveDocumentProc					*MNLSaveDocument;
extern	MNLSaveDocumentToBufferProc			*MNLSaveDocumentToBuffer;

// Section I-C - Getting information about a document

typedef	long	(MNLGetScoreInformationProc)(long docID,long *totalMeasures,long *totalStaves,long *totalPages,char **title);
typedef	long	(MNLGetPageSizeProc)(long docID,long pageNumber,long *width,long *height);
typedef	long	(MNLGetPageMeasuresProc)(long docID,long pageNumber,long *firstMeasure,long *lastMeasure);
typedef	long	(MNLGetKeySignatureProc)(long docID,long index,long *ksig,long *measure);
typedef	long	(MNLGetTimeSignatureProc)(long docID,long index,long *num,long *denom,long *measure);

extern	MNLGetScoreInformationProc			*MNLGetScoreInformation;
extern	MNLGetPageSizeProc					*MNLGetPageSize;
extern	MNLGetPageMeasuresProc				*MNLGetPageMeasures;
extern	MNLGetKeySignatureProc				*MNLGetKeySignature;
extern	MNLGetTimeSignatureProc				*MNLGetTimeSignature;

// Section I-D - Displaying the score

typedef	long (MNLDisplayPageProc)(long docID,long pageNumber,long offsetX,long offsetY,float scale,long x1,long y1,long x2,long y2,long graphic);
typedef	long (MNLDisplayLinearProc)(long docID,long firstMeasure,long totalMeasures,long offsetX,long offsetY,float scale,long x1,long y1,long x2,long y2,long graphic);
typedef	long (MNLDisplayWrapProc)(long docID,long firstMeasure,long totalMeasures,long offsetX,long offsetY,float scale,long x1,long y1,long x2,long y2,long graphic,float minMeasScale,float maxMeasScale,long padding);
typedef	long (MNLSimulateDisplayWrapProc)(long docID,long firstMeasure,long offsetX,long offsetY,float scale,long x1,long y1,long x2,long y2,float minMeasScale,float maxMeasScale,long *totalSystems,long *totalMeasuresTable,long *systemHeightTable);
typedef	long (MNLFindPositionProc)(long docID,long x,long y,long *measureNumber,long *beat, long *staffNumber);
typedef	long (MNLGetMeasurePositionProc)(long docID,long measureNumber,long staffNumber,long *posX,long *posY);
typedef	long (MNLMeasureUnitsToPositionProc)(long docID,long measureNumber,long staffNumber,long units,long *posX);
typedef	long (MNLMeasurePositionToUnitsProc)(long docID,long measureNumber,long staffNumber,long posX, long *units);


extern	MNLDisplayPageProc					*MNLDisplayPage;
extern	MNLDisplayLinearProc				*MNLDisplayLinear;
extern	MNLDisplayWrapProc					*MNLDisplayWrap;
extern	MNLSimulateDisplayWrapProc			*MNLSimulateDisplayWrap;
extern	MNLFindPositionProc					*MNLFindPosition;
extern	MNLGetMeasurePositionProc			*MNLGetMeasurePosition;
extern	MNLMeasureUnitsToPositionProc		*MNLMeasureUnitsToPosition;
extern	MNLMeasurePositionToUnitsProc		*MNLMeasurePositionToUnits;

// Section I-E - Modifying the score

typedef	long (MNLTransposeProc)(long docID,long ksig,long transposeUp,long fromMeasure,long toMeasure);
typedef	long (MNLTransposeByIntervalProc)(long docID,long noteInterval,long midiInterval,long adaptKeySignature,long fromMeasure,long toMeasure);
typedef	long (MNLNewPageLayoutProc)(long docID,long paperWidth,long paperHeight,float scale,long leftMargin,long rightMargin, long topMargin, long bottomMargin,unsigned long staffFlags,long staffChords);

extern	MNLTransposeProc					*MNLTranspose;
extern	MNLTransposeByIntervalProc			*MNLTransposeByInterval;
extern	MNLNewPageLayoutProc				*MNLNewPageLayout;

// Section I-F - Playing the score

typedef	long (MNLOpenMidiPortProc)(long midiOutIndex);
typedef	long (MNLCloseMidiPortProc)(void);
typedef	long (MNLOpenSoundLibraryProc)(char *libraryName);
typedef	long (MNLCloseSoundLibraryProc)(void);
typedef	long (MNLIdleProc)(void);
typedef	long (MNLStartPlayingProc)(long docID,long fromMeasure,long toMeasure,callbackWhenFinishedProc *myCallbackWhenFinished,long graphic,long playbackMode);
typedef	long (MNLStopPlayingProc)(void);
typedef	long (MNLGetPlayingPositionProc)(long *measure,long *beat,long *unit);
typedef	long (MNLSetPlaybackOptionsProc)(long docID,long symbolFlags,long midiFlags);
typedef	long (MNLGetPlaybackParameterProc)(long docID,long staffNumber,long paramNumber,long *paramValue);
typedef	long (MNLSetPlaybackParameterProc)(long docID,long staffNumber,long paramNumber,long paramValue);
typedef	long (MNLDisplayKeyboardProc)(long keyWidth,long x1,long y1,long x2,long y2,long octave,long graphic);
typedef	long (MNLKeyboardMidiFilterProc)(long midiFilter);
typedef	long (MNLRefreshKeyboardProc)(long graphic);
typedef	long (MNLReleaseKeyboardProc)(void);
typedef	long (MNLDisplayFretboardProc)(long fretDist,long x1,long y1,long x2,long y2,long graphic);
typedef	long (MNLFretboardMidiFilterProc)(long midiFilter);
typedef	long (MNLRefreshFretboardProc)(long graphic);
typedef	long (MNLReleaseFretboardProc)(void);
typedef	long (MNLResetMidiProc)(long midiResetMode,long keyboardReset,long fretboardReset);

extern	MNLOpenMidiPortProc					*MNLOpenMidiPort;
extern	MNLCloseMidiPortProc				*MNLCloseMidiPort;
extern	MNLOpenSoundLibraryProc				*MNLOpenSoundLibrary;
extern	MNLCloseSoundLibraryProc			*MNLCloseSoundLibrary;
extern	MNLIdleProc							*MNLIdle;
extern	MNLStartPlayingProc					*MNLStartPlaying;
extern	MNLStopPlayingProc					*MNLStopPlaying;
extern	MNLGetPlayingPositionProc			*MNLGetPlayingPosition;
extern	MNLSetPlaybackOptionsProc			*MNLSetPlaybackOptions;
extern	MNLGetPlaybackParameterProc			*MNLGetPlaybackParameter;
extern	MNLSetPlaybackParameterProc			*MNLSetPlaybackParameter;
extern	MNLDisplayKeyboardProc				*MNLDisplayKeyboard;
extern	MNLKeyboardMidiFilterProc			*MNLKeyboardMidiFilter;
extern	MNLRefreshKeyboardProc				*MNLRefreshKeyboard;
extern	MNLReleaseKeyboardProc				*MNLReleaseKeyboard;
extern	MNLDisplayFretboardProc				*MNLDisplayFretboard;
extern	MNLFretboardMidiFilterProc			*MNLFretboardMidiFilter;
extern	MNLRefreshFretboardProc				*MNLRefreshFretboard;
extern	MNLReleaseFretboardProc				*MNLReleaseFretboard;
extern	MNLResetMidiProc					*MNLResetMidi;

#endif 	// __MUSIC_NOTATION_LIBRARY_DLL_H__
