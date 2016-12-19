// update : 15/11/2016 18:00

#ifndef DEF_MUSICXMLSCORE

#define DEF_MUSICXMLSCORE

typedef	long (MNLOpenDocumentProc)(char *pathname, long *docID);
typedef	long (MNLGetScoreInformationProc)(long docID, long *totalMeasures, long *totalStaves, long *totalPages, char **title);
typedef	long (MNLDisplayWrapProc)(long docID, long firstMeasure, long totalMeasures, long offsetX, long offsetY, float scale, long x1, long y1, long x2, long y2, long graphic, float minMeasScale, float maxMeasScale, long padding);
typedef	long(MNLDisplayPageProc)(long docID, long pageNumber, long offsetX, long offsetY, float scale,long x1, long y1, long x2, long y2, long graphic);
typedef	long (MNLSimulateDisplayWrapProc)(long docID, long firstMeasure, long offsetX, long offsetY, float scale, long x1, long y1, long x2, long y2, float minMeasScale, float maxMeasScale, long *totalSystems, long *totalMeasuresTable, long *systemHeightTable);
typedef long (MNLGetPageSizeProc)(long docID, long pageNumber, long *width, long *height);
typedef long (MNLNewPageLayoutProc)(long docID, long paperWidth, long paperHeight, float scale, long leftMargin, long rightMargin, long topMargin, long bottomMargin, unsigned long staffFlags, long staffChords);
typedef long (MNLMeasureUnitsToPositionProc)(long docID, long measureNumber, long staffNumber,long units, long *posX);
typedef long (MNLGetMeasurePositionProc)(long docID, long measureNumber, long staffNumber, long *posX, long *posY);
typedef long (MNLCloseDocumentProc)(long docID);
typedef long (MNLGetPageMeasuresProc)(long docID, long pageNumber, long *firstMeasure, long *lastMeasure);
typedef long (MNLFindPositionProc)(long docID, long x, long y, long *measureNumber, long *beat, long *staffNumber);

class musicxmlscore
	: public viewerscore
{

public:
	musicxmlscore(wxWindow *parent, wxWindowID id, mxconf* config, const wxDynamicLibrary  &myDll);
	~musicxmlscore();

	void onPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void newLayout();
	bool GetScoreInfo();
	bool xmlIsOk();

	void xmlProcessScore();
	void xmlGetParam();
	void xmlProcessPartList(wxXmlNode *xmlParentIn, wxXmlNode *xmlParentOut);
	wxXmlNode *GetXmlPath(wxXmlNode *parent, const wxString name, wxString *content);
	bool displayFile();
	bool getScorePosition(int *absolute_measure_nr, int *measure_nr, int *beat, int *t);

	virtual bool setFile(const wxFileName &lfilename, bool onstart);
	virtual int getTrackCount();
	virtual wxString getTrackName(int trackNr);
	virtual void setPosition(int pos, bool playing, bool quick);
	virtual void zoom(int dzoom);
	virtual void gotoPosition();

private:
	wxWindow *mParent;
	mxconf *mConf;
	wxFileName xmlName;

	MNLDisplayPageProc *MNLDisplayPage;
	MNLGetPageSizeProc *MNLGetPageSize;
	MNLDisplayWrapProc *MNLDisplayWrap;
	MNLGetScoreInformationProc *MNLGetScoreInformation;
	MNLOpenDocumentProc	*MNLOpenDocument;
	MNLNewPageLayoutProc *MNLNewPageLayout;
	MNLCloseDocumentProc *MNLCloseDocument;
	MNLMeasureUnitsToPositionProc *MNLMeasureUnitsToPosition;
	MNLGetMeasurePositionProc *MNLGetMeasurePosition;
	MNLGetPageMeasuresProc *MNLGetPageMeasures;
	MNLFindPositionProc *MNLFindPosition;

	musicxmlcompile *xmlCompile = NULL;
	bool xmlExtractXml(wxFileName f);
	bool xmlLoad();
	bool xmlLoadMusicXml();

	long docID;
	int nrChord;

	bool displayed = false;
	float fzoom = 1.0;
	wxSize sizeClient;
	bool sizeOk;
	long totalMeasures, totalStaves, totalPages;
	int currentNrEvent = 0;
	int prevPos = -1;
	bool prevPlaying = true;
	wxRect rectPrevPos;
	void setCursor(int nrEvent, wxRect *prect , bool red, bool green);
	bool setPage(int measureNr);
	void drawpage(int pageNr, bool left, wxDC *dc);
	int pageNrLeft = -1;
	int pageNrRight = -1;
	int sizeX;
	int sizeY;
	wxArrayInt measurePage;

	wxDECLARE_EVENT_TABLE();

};

#endif