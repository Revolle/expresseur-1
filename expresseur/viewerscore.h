// update : 15/11/2016 18:00
#ifndef DEF_VIEWERSCORE

#define DEF_VIEWERSCORE

class viewerscore : public wxPanel
{
public:
	viewerscore(wxWindow *parent, wxWindowID id);
	virtual ~viewerscore();
	virtual bool setFile(const wxFileName &lfilename, bool onstart) = 0;
	virtual bool displayFile() = 0;
	virtual int getTrackCount() = 0;
	virtual wxString getTrackName(int trackNr) = 0;
	virtual void newLayout() = 0;
	virtual void setPosition(int pos, bool playing, bool quick) = 0;
	virtual void zoom(int dzoom) = 0;
	virtual void gotoPosition() =  0;
};

#endif
