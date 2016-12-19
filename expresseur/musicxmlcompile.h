// update : 01/12/2016 21:00

#ifndef DEF_MUSICXMLCOMPILE

#define DEF_MUSICXMLCOMPILE

// class to have a list of lMeasureMarks
///////////////////////////////////////
class c_measureMark
{
public:
	c_measureMark(int inr) { number = inr; name.sprintf("M%d", inr); }
	void changeMeasure(int inr) { number = inr; name.sprintf("M%d", inr); }
	void merge(const c_measureMark &m) 
	{ 
		rehearsal = rehearsal | m.rehearsal; 
		restart = restart | m.restart;
		segno = segno | m.segno;
		repeatForward = repeatForward | m.repeatForward;
		repeatBackward = repeatBackward | m.repeatBackward;
		jumpnext = jumpnext | m.jumpnext;
		coda = coda | m.coda;
		tocoda = tocoda | m.tocoda;
		dacapo = dacapo | m.dacapo;
		fine = fine | m.fine;
		dalsegno = dalsegno | m.dalsegno;
	}
	wxString name;
	int number;
	bool rehearsal = false;
	bool restart = false;
	bool segno = false;
	bool repeatForward = false;
	bool repeatBackward = false;
	bool jumpnext = false;
	bool coda = false;
	bool tocoda = false;
	bool dacapo = false;
	bool fine = false;
	bool dalsegno = false;
};
WX_DECLARE_LIST(c_measureMark, l_measureMark);

// class to have a list of ornements
////////////////////////////////////
class c_ornament
{
public:
	c_ornament(int itype, int imeasureNumber, int it, int ipartNr, int istaffNr, int irepeat , bool ibefore , wxString ivalue )
	{
		type = itype;
		measureNumber = imeasureNumber;
		t = it;
		twelve_t = 12 * it;
		partNr = ipartNr;
		staffNr = istaffNr;
		repeat = irepeat;
		before = ibefore;
		value = ivalue;
	};
	bool isSameAs(c_ornament *m)
	{
		bool r =
			(type == m->type) &&
			(partNr == m->partNr) &&
			(staffNr == m->staffNr) &&
			(measureNumber == m->measureNumber) &&
			(t == m->t) &&
			(chord_order == m->chord_order) &&
			(twelve_t == m->twelve_t) &&
			(repeat == m->repeat) &&
			(before == m->before) &&
			(value == m->value) &&
			(absolute_measureNr == m->absolute_measureNr) &&
			(mark_prefix == m->mark_prefix);
		return r;
	};
	int type; // type of lOrnaments : enum lOrnaments o_xx
	int partNr = -1; // track to apply the ornament ( -1 == all )
	int staffNr = -1; // staff to apply the ornament ( -1 == all )
	int measureNumber; // measure Number
	int t; // original time in the meausre, in divisions of the measure
	int chord_order = -1; // order in the chord 
	int twelve_t; // time in the meausre, in 2*2*3*divisions of the measure
	int repeat = -1; // restrict the ornament for the 1st time ( 0 ), 2nd time, ... always (-1)
	bool before = false; // to make the ornament before the time specified
	wxString value = wxEmptyString; // generic value
	bool absolute_measureNr = false; // specify an absolute position ( after compilation of the measures )
	int mark_prefix = -1; // if there is a marker to specify the relative measure nr 
	int tInBeat = NULL_INT;
	int beat = NULL_INT;
	bool processed = false;
};
WX_DECLARE_LIST(c_ornament, l_ornament);

// class to have a sorted list of musicXml events
/////////////////////////////////////////////////
class c_musicxmlevent
{
public:
	c_musicxmlevent() {}
	c_musicxmlevent(const c_musicxmlevent &musicxmlevent)
	{
		partNr = musicxmlevent.partNr;
		staffNr = musicxmlevent.staffNr;
		voice = musicxmlevent.voice;
		original_measureNr = musicxmlevent.original_measureNr;
		start_measureNr = musicxmlevent.start_measureNr;
		start_twelve_t = musicxmlevent.start_twelve_t;
		start_order = musicxmlevent.start_order;
		stop_measureNr = musicxmlevent.stop_measureNr;
		stop_twelve_t = musicxmlevent.stop_twelve_t;
		stop_order = musicxmlevent.stop_order;
		pitch = musicxmlevent.pitch;
		twelve_duration = musicxmlevent.twelve_duration;
		repeat = musicxmlevent.repeat;
		twelve_division_measure = musicxmlevent.twelve_division_measure;
		twelve_division_beat = musicxmlevent.twelve_division_beat;
		twelve_division_quarter = musicxmlevent.twelve_division_quarter;
		fifths = musicxmlevent.fifths;
		visible = musicxmlevent.visible;
		played = musicxmlevent.played;
	}
	c_musicxmlevent(int ipartNr, int istaffNr, int ivoice, int istart_measureNr, int ioriginal_measureNr, int istart_t, int istop_measureNr, int istop_t, int ipitch, int idivision_measure, int idivision_beat, int idivision_quarter, int irepeat, int iorder, int ififths)
	{
		partNr = ipartNr;
		staffNr = istaffNr;
		voice = ivoice;
		original_measureNr = ioriginal_measureNr;
		start_measureNr = istart_measureNr;
		start_twelve_t = 12 * istart_t;
		stop_measureNr = istop_measureNr;
		stop_twelve_t = 12 * istop_t;
		pitch = ipitch;
		twelve_duration = 12 * idivision_measure * (stop_measureNr - start_measureNr) + ( stop_twelve_t - start_twelve_t ) ;
		repeat = irepeat;
		twelve_division_measure = 12 * idivision_measure;
		twelve_division_beat = 12 * idivision_beat;
		twelve_division_quarter = 12 * idivision_quarter;
		start_order = iorder;
		fifths = ififths;
	}
	int nr = 0; // index sorted
	int partNr = 0;
	int staffNr = 0;
	int original_measureNr = 1;
	int voice = 0;
	bool visible = true;
	bool played = true;

	int twelve_division_measure = 12*4*4; // nb division per measure * 12
	int twelve_division_beat = 12*4; // nb division per beat * 12
	int twelve_division_quarter = 12; // nb division per quarter * 12
	int repeat = 0; // repeat of this event in the score 

	int start_measureNr = 1;
	int start_twelve_t = 0; // start of the note in 2*2*3*divisions of the quarter
	int start_order = 0; // start_order of the lOrnaments for the same start_twelve_t
	int chord_order = 0;

	int stop_measureNr = 1;
	int stop_twelve_t = 0; // start of the note in 2*2*3*divisions of the quarter
	int stop_order = 0; 

	wxArrayInt starts; // index of musicxmlevent to start synchronously at the trigger-on
	wxArrayInt stops; // index of musicxmlevent to stop synchronously 
	int will_stop_index = -1 ; // in a starts musicxmlevent, link with the index of musicxmlevent to stop at the trigger-off
	int stop_index = -1; // in a starts musicxmlevent, link with the index of musicxmlevent to stop synchronously with the trigger-on 
	bool stop_orpheline = true; // true when the stop is not linked to a musicxmlevent

	int pitch = -1 ;
	int nuance = NULL_INT;
	int velocity = 64 ;
	int transpose = NULL_INT;
	int delay = 0;
	int twelve_duration = 0; // duration of the note in 2*2*3*divisions of the quarter
	bool tenuto = false;
	int accent = 0;
	bool crescendo = false;
	int dynamic = -1; // influence on the dynamic of velocity-input
	int random_delay = -1;
	int pedal = -1 ;
	int o_arpeggiate = 0 ;
	wxString text;
	wxString lua;
	int fifths = 0;
	int nb_ornaments = 0;
};
WX_DECLARE_LIST(c_musicxmlevent, l_musicxmlevent);

// class to have a list of arpeggiate
////////////////////////////////////
class c_arpeggiate_toapply
{
public:
	c_arpeggiate_toapply(int inr, bool idown, bool ibefore, c_musicxmlevent *imusicxmlevent)
	{
		nr = inr;
		down = idown;
		before = ibefore;
		musicxmlevent = imusicxmlevent;
	}
	int nr;
	bool down;
	bool before;
	c_musicxmlevent *musicxmlevent;
};
WX_DECLARE_LIST(c_arpeggiate_toapply, l_arpeggiate_toapply);


// class to compile a musicXML score
////////////////////////////////////
class musicxmlcompile
{
public:
	musicxmlcompile();
	~musicxmlcompile();
	wxFileName loadTxtFile(wxFileName txtfile);
	void setNameFile(wxFileName txtfile,wxFileName xmlfile);
	bool loadXmlFile(wxString xmlfilein, wxString xmlfileout, bool useMarkFile = true);
	bool isOk(bool compiled_score = false);
	bool getInfoEvent(int nrEvent, int *measureNr, int *t480);
	int measureBeatToEventNr(int measureNr, int beat);
	int stringToEventNr(wxString s);
	bool getScorePosition( int nrEvent , int *absolute_measure_nr , int *measure_nr, int *beat, int *t );
	bool getTrackDisplay(int nrTrack);
	bool getTrackPlay(int nrTrack);
	wxArrayInt getTracksPlay();
	wxArrayInt getTracksDisplay();
	int getTrackNr(wxString idTrack);
	wxString getTitle();
	wxArrayString getTracksName();
	wxString getTrackName(int nrTrack);
	wxString getTrackId(int nrTrack);
	int getTracksCount();
	static void clearLuaScore();
	wxString pitchToString(int p);
	wxString pitchToString(wxArrayInt p);
	wxArrayInt stringToPitch(wxString s, int *nbChord);
	static wxArrayString getListOrnament();
	wxString music_xml_complete_file;
	wxString music_xml_displayed_file;

private:
	void dump_musicxmlevents();
	void compile(bool reanalyse, wxString xmlfileout);
	void writeMarks();
	void readMarks(bool full = true);
	bool readMarkLine(wxString line, wxString sectionName);
	void xmlLoad(wxString xmlfilein);
	void analyseMeasure(); // analyse the default repeat-sequence from "score" to "measureMark" and "markList"
	void analyseMeasureMarks();
	int getMarkNr(int measureNr);
	int getMeasureNr(int measureNr);
	void analyseList();
	void analyseNoteOrnaments(c_note *note, int measureNumber, int t);
	void sortMeasureMarks();
	int getDivision(int measure_nr, int *division_quarter, int *division_measure);
	int getPartNr(wxString spart, int *partNb = NULL);
	int compileNote(c_part *part,c_note *note, int measureNr, int originalMeasureNr, int t, int division_measure, int division_beat, int division_quarter, int repeat, int key_fifths);
	void compileTie(c_part *part, c_note *note, int *measureNr, int *t, int nbDivision);
	void compileMusicxmlevents(bool second_time = false);
	void pushLuaMusicxmlevents();
	void addOrnaments();
	void clearOrnaments();
	void singleOrnaments();
	void createOrnament(c_ornament *ornament);
	void addGraces(wxString gracePitches, bool before, c_musicxmlevent *musicxmlevent);
	void addGraces(wxArrayInt gracePitches, bool before, c_musicxmlevent *musicxmlevent);
	void addOrnament(c_ornament *ornament, c_musicxmlevent *musicxmlevent,int nr_ornament);
	void compileCrescendo();
	void compileTransposition();
	void compileArppegio();
	void compilePedalBar();
	void removeExpresseurPart();
	void createListMeasures();
	void buildMeasures();
	void compileScore();
	void delete_bar_label(c_measure *newMeasure);
	void addExpresseurPart();
	void compileExpresseurPart();
	void addNote(c_measure *measure, int from_t, int to_t, bool rest, bool tie_back, bool tie_next, bool first_note , int nbOrnaments , wxString *text);
	void addSymbolNote(c_measure *measure, int duration, bool rest, bool tie_back, bool tie_next, bool first_note, int nbOrnaments, wxString *text);
	void calculateDuration(int duration, int divisions, int *duration_done , wxString *typeNote , int *dot , int *tuplet);
	wxFileName txtFile;
	wxFileName musicxmlFile;
	c_score_partwise *score; // original score
	l_measureMark lMeasureMarks; // list of markers : a marker is linked to a measure in the original score . e.g. marker[1]=measure(10), marker[2] =measure(14) , marker[3]=18
	wxArrayInt markList; // list of markers to play succesfully, refer to measureMark . e.g. 1,2,2,1 means play marker[1], marker[2] twice, and marker[1] to finish
	wxArrayInt measureList; // list of measures to play, refer to markList and score. e.g. 10,11,12,13,14,15,16,17,14,15,16,17,10,11,12,13
	l_ornament lOrnaments; // list of lOrnaments to compile and add 
	c_score_partwise *compiled_score = NULL; // score compiled, refer to score, measureList, partidToPlay, partidToPlay
	l_musicxmlevent lMusicxmlevents;
	l_musicxmlevent lOrnamentsMusicxmlevents;
	wxString grace;
	l_arpeggiate_toapply lArpeggiate_toapply;
	int nbEvents = 0;
};


#endif
