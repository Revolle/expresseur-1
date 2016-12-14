// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_TEXTSCORE

#define DEF_TEXTSCORE


class textscore
	: public wxTextCtrl
{

public:
	textscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~textscore();

	void OnSize(wxSizeEvent& event);

	bool setFile(const wxFileName &lfilename);
	void saveFile(const wxFileName &filename);

	void setEditMode(bool editMode);
	void compileText();
	int scanPosition(bool editMode);
	int getInsertionPoint();
	int getFontSize();
	bool needToSave();
	void noNeedToSave();
	void setMyFontSize(int t);
	void zoom(int d);

private:
	wxWindow *mParent;
	mxconf *mConf;

	int sizeFont;

	int oldchordStart, oldchordEnd;
	int oldsectionStart, oldsectionEnd;
	wxTextAttr textAttrRecognized;
	wxTextAttr textAttrNormal;
	wxTextAttr textAttrPosition;
	
	wxString oldText;


	wxDECLARE_EVENT_TABLE();

};

#endif