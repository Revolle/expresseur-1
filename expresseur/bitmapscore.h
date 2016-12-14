// update : 15/11/2016 18:00
#ifndef DEF_BITMAPSCORE

#define DEF_BITMAPSCORE


class bitmapscore
	: public viewerscore
{

public:
	bitmapscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~bitmapscore();
	void onPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMouse(wxMouseEvent& event);
	void newLayout();
	virtual bool setFile(const wxFileName &lfilename, bool onstart);
	virtual bool displayFile();
	virtual void setPosition(int pos, bool playing, bool quick);
	virtual int getTrackCount();
	virtual void zoom(int dzoom);
	virtual wxString getTrackName(int trackNr);
	virtual void gotoPosition();

private:
	wxWindow *mParent;
	wxImage *mImage;
	mxconf *mConf;

	double xScale, yScale;


	wxPoint mPointStart, mPointEnd;
	wxRect prevRect , selectedRect ;
	bool alertSetRect;
	wxRect highlight(bool on, wxPoint start, wxPoint end, wxDC *lDC);

	wxRect rectChord[MAX_RECTCHORD];
	int nbRectChord;
	int nrChord;
	int prevNrChord;
	wxFileName fileRectChord;
	void readRectChord(bool onstart);
	void writeRectChord();
	void highighNrChord(int nrChord);

	wxDECLARE_EVENT_TABLE();

};

#endif