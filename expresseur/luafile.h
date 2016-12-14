// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_LUAFILE

#define DEF_LUAFILE

class luafile
	: public wxDialog
{

public:
	luafile(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf);
	~luafile();

	void OnSize(wxSizeEvent& event);

	static void reset(mxconf* mConf, bool all = true);
	static void write(mxconf* mConf, wxTextFile *lfile);
	static void read(mxconf* mConf, wxTextFile *lfile);
	static void functioncallback();
	static bool isCalledback();
	void OnLuaFile(wxCommandEvent& event);
	void OnLuaParameter(wxCommandEvent&  event);

private:
	wxFrame *mParent;
	wxDialog *mThis;
	mxconf* mConf;

	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;

	wxDECLARE_EVENT_TABLE();
};

#endif