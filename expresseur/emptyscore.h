// update : 15/11/2016 18:00
#ifndef DEF_EMPTYSCORE

#define DEF_EMPTYSCORE


class emptyscore
	: public viewerscore
{

public:
	emptyscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~emptyscore();
	void onPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
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
	mxconf *mConf;


	wxDECLARE_EVENT_TABLE();

};

#endif