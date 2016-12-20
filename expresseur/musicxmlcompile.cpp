/////////////////////////////////////////////////////////////////////////////
// Name:        musicxmlcompile.cpp
// Purpose:     class to compile musicxml-source-file
// output :     musicxml-Expresseur-file MUSICXML_FILE + structure-to-play
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/11/2015
// update : 01/12/2016 22:00 
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
#include "wx/tokenzr.h"
#include "wx/xml/xml.h"
#include "wx/filefn.h"
#include "wx/wfstream.h"
#include "wx/dynarray.h"
#include "wx/arrstr.h"
#include "wx/textfile.h"
#include "global.h"

#include "luabass.h"
#include "basslua.h"

#include "musicxml.h"
#include "musicxmlcompile.h"


//#include <wx/arrimpl.cpp>
//WX_DEFINE_SORTED_ARRAY(ArrayOfMusicxmlevents);
//WX_DEFINE_SORTED_ARRAY(musicxmlevent *, SortedArrayOfMusicxmlevents);

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(l_measureMark);
WX_DEFINE_LIST(l_ornament);
WX_DEFINE_LIST(l_musicxmlevent);
WX_DEFINE_LIST(l_arpeggiate_toapply);
enum ornamentType
{
	o_divisions, // divisions per quarter note, used to count in a measure. For information to the end-user
	o_dynamic,
	o_random_delay,
	o_pedal_bar,
	o_pedal,
	o_lua,
	o_text,
	o_pianissimo,
	o_piano,
	o_mesopiano,
	o_mesoforte,
	o_forte,
	o_fortissimo,
	o_crescendo,
	o_diminuendo,
	o_tenuto,
	o_staccato,
	o_accent,
	o_grace,
	o_mordent,
	o_turn,
	o_btrill,
	o_trill,
	o_arpeggiate,
	o_transpose,
	o_delay,
	o_before,
	o_after,
	o_flagend
};
char const *ornamentName[] =
{
	"divisions_per_quarter",
	"dynamic",
	"random_delay",
	"pedal_bar",
	"pedal",
	"lua",
	"text",
	"pianissimo",
	"piano",
	"mesopiano",
	"mesoforte",
	"forte",
	"fortissimo",
	"crescendo",
	"diminuendo",
	"tenuto",
	"staccato",
	"accent",
	"grace",
	"mordent",
	"turn",
	"btrill",
	"trill",
	"arpeggiate",
	"transpose",
	"delay"
	"before",
	"after",
	"flagend",
};

#define END_OF_THE_SCORE "END_OF_THE_SCORE"
#define PART_PLAYED "played"
#define PART_NOT_PLAYED "not played"
#define PART_VISIBLE "visible"
#define PART_NOT_VISIBLE "not visible"

int musicXmlEventsCompareStart(const void *arg1, const void *arg2)
{
	// funtion used to sort the list of musicxmlevent, using start time : l_musicxmlevent
	c_musicxmlevent *r1 = *(c_musicxmlevent**)arg1;
	c_musicxmlevent *r2 = *(c_musicxmlevent**)arg2;
	if (r1->start_measureNr < r2->start_measureNr)
		return -1;
	if (r1->start_measureNr >r2->start_measureNr)
		return 1;

	if (r1->start_twelve_t < r2->start_twelve_t)
		return -1;
	if (r1->start_twelve_t >r2->start_twelve_t)
		return 1;
	if (r1->start_order < r2->start_order)
		return -1;
	if (r1->start_order > r2->start_order)
		return 1;


	if (r1->stop_measureNr < r2->stop_measureNr)
		return -1;
	if (r1->stop_measureNr > r2->stop_measureNr)
		return 1;
	if (r1->stop_twelve_t < r2->stop_twelve_t)
		return -1;
	if (r1->stop_twelve_t > r2->stop_twelve_t)
		return 1;
	if (r1->stop_order < r2->stop_order)
		return -1;
	if (r1->stop_order > r2->stop_order)
		return 1;

	if (r1->partNr < r2->partNr)
		return -1;
	if (r1->partNr > r2->partNr)
		return 1;

	if (r1->voice < r2->voice)
		return -1;
	if (r1->voice > r2->voice)
		return 1;

	if (r1->pitch < r2->pitch)
		return -1;
	if (r1->pitch >r2->pitch)
		return 1;

	return 0;
}
int musicXmlEventsCompareStop(const void *arg1, const void *arg2)
{
	// funtion used to sort the list of c_musicxmlevent, using stop time : l_musicxmlevent
	c_musicxmlevent *r1 = *(c_musicxmlevent**)arg1;
	c_musicxmlevent *r2 = *(c_musicxmlevent**)arg2;

	if (r1->stop_measureNr < r2->stop_measureNr)
		return -1;
	if (r1->stop_measureNr > r2->stop_measureNr)
		return 1;

	if (r1->stop_twelve_t < r2->stop_twelve_t)
		return -1;
	if (r1->stop_twelve_t > r2->stop_twelve_t)
		return 1;

	if (r1->stop_order < r2->stop_order)
		return -1;
	if (r1->stop_order > r2->stop_order)
		return 1;

	if ((!(r1->tenuto)) && (r2->tenuto))
		return -1;
	if ((r1->tenuto) && (!(r2->tenuto)))
		return 1;

	if (r1->start_measureNr < r2->start_measureNr)
		return 1;
	if (r1->start_measureNr > r2->start_measureNr)
		return -1;

	if (r1->start_twelve_t < r2->start_twelve_t)
		return 1;
	if (r1->start_twelve_t > r2->start_twelve_t)
		return -1;

	if (r1->start_order < r2->start_order)
		return 1;
	if (r1->start_order > r2->start_order)
		return -1;

	if (r1->nr < r2->nr)
		return -1;
	if (r1->nr > r2->nr)
		return 1;

	return 0;
}
int measureMarkCompare(const void *pa, const void *pb)
{
	// compare function for the list of l_measureMark
	c_measureMark *ppa = *(c_measureMark**)pa;
	c_measureMark *ppb = *(c_measureMark**)pb;
	if (ppa->number == ppb->number) 
		return 0;
	if (ppa->number > ppb->number) 
		return 1;
	return -1;
}
int ornamentCompare(const void *pa, const void *pb)
{
	// compare function for the list of l_ornament
	c_ornament *ppa = *(c_ornament**)pa;
	c_ornament *ppb = *(c_ornament**)pb;
	if (ppa->measureNumber < ppb->measureNumber)
		return -1;
	if (ppa->measureNumber > ppb->measureNumber)
		return 1;
	if (ppa->twelve_t < ppb->twelve_t)
		return -1;
	if (ppa->twelve_t > ppb->twelve_t)
		return 1;
	if (ppa->partNr < ppb->partNr)
		return -1;
	if (ppa->partNr > ppb->partNr)
		return 1;
	return 0;
}
int arpeggiateCompare(const void *pa, const void *pb)
{
	// compare function for the list of l_ornament
	c_arpeggiate_toapply *ppa = *(c_arpeggiate_toapply**)pa;
	c_arpeggiate_toapply *ppb = *(c_arpeggiate_toapply**)pb;
	if (ppa->nr < ppb->nr)
		return -1;
	if (ppa->nr > ppb->nr)
		return 1;
	if (ppa->musicxmlevent->pitch < ppb->musicxmlevent->pitch)
		return ((ppa->down && !(ppa->before)) || (!(ppa->down) && ppa->before) ? 1 : -1);
	if (ppa->musicxmlevent->pitch > ppb->musicxmlevent->pitch)
		return ((ppa->down && !(ppa->before)) || (!(ppa->down) && ppa->before) ? -1 : 1);
	return 0;
}
void musicxmlcompile::dump_musicxmlevents()
{
	/*
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		int nr = current_musicxmlevent->nr;
		int start = current_musicxmlevent->start_twelve_t / 12;
		int stop = current_musicxmlevent->stop_twelve_t / 12;
		int track = current_musicxmlevent->partNr;
		int pitch = current_musicxmlevent->pitch;
		int i = 1;
	}
	*/

}
musicxmlcompile::musicxmlcompile()
{
	// slMusicxmlevents = new SortedArrayOfMusicxmlevents(ComparemusicXmlEvents);
	score = NULL;
	compiled_score = NULL;
	lMusicxmlevents.DeleteContents(true);
	lMeasureMarks.DeleteContents(true);
	lOrnaments.DeleteContents(true);
}
musicxmlcompile::~musicxmlcompile()
{
	lMusicxmlevents.DeleteContents(true);
	lMeasureMarks.DeleteContents(true);
	lArpeggiate_toapply.DeleteContents(true);
	lOrnaments.DeleteContents(true);
	lOrnamentsMusicxmlevents.DeleteContents(true);

	if (score != NULL)
		delete score;
	if (compiled_score != NULL)
		delete compiled_score;
}
wxFileName musicxmlcompile::loadTxtFile(wxFileName itxtFile)
{
	// just extract the musicxml file from the txtfile
	txtFile = itxtFile;
	readMarks(false);
	return musicxmlFile;
}
void musicxmlcompile::setNameFile(wxFileName itxtFile, wxFileName ixmlFile)
{
	txtFile = itxtFile;
	musicxmlFile = ixmlFile;
}
bool musicxmlcompile::loadXmlFile(wxString xmlfilein, wxString xmlfileout, bool useMarkFile)
{
	// load the musicxml musicxmlFile, compile it, and generate the MUSICXML_FILE for the musicxml-viewer

	// load the inupt muscixml file in the C++ score structure
	xmlLoad(xmlfilein);

	if (!isOk())
		return false;

	// score->write("copytest.xml", false); // for debug

	// compile the C++ score structure into :
	// - a musicxml file to diplay 
	// - a set of events to play
	// - a file of parameters ( lOrnaments, repetitions, .. )
	compile(useMarkFile, xmlfileout);

	return isOk();
}
void musicxmlcompile::xmlLoad(wxString xmlfilein)
{
	// load the inupt muscixml file in the C++ score structure

	if (score != NULL)
		delete score;
	score = NULL;

	wxXmlDocument *xmlDoc = new wxXmlDocument();
	if (!xmlDoc->Load(xmlfilein))
	{
		delete xmlDoc;
		return;
	}

	wxXmlNode *root = xmlDoc->GetRoot();
	wxString name = root->GetName();
	if (name != "score-partwise")
	{
		wxMessageBox("Only musicXML score-partwise is accepted", "MusicXML load", wxICON_ERROR);
		delete xmlDoc;
		return ;
	}

	// load xml in the C++ score-structure
	score = new c_score_partwise(root);
	delete xmlDoc;

	if (!isOk())
	{
		delete score;
		score = NULL;
	}

}
void musicxmlcompile::compile(bool useMarkFile, wxString xmlfileout)
{
	// compile the C++ score structure into :
	// - a musicxml file to diplay 
	// - a set of events to play
	// - a file of parameters ( lOrnaments, repetitions, .. )

	if (!isOk())
		return;

	lMusicxmlevents.DeleteContents(true);
	lMeasureMarks.DeleteContents(true);
	lArpeggiate_toapply.DeleteContents(true);
	lOrnaments.DeleteContents(true);
	lOrnamentsMusicxmlevents.DeleteContents(true);

	lMusicxmlevents.Clear();
	lMeasureMarks.Clear();
	lArpeggiate_toapply.Clear();
	lOrnaments.Clear();
	lOrnamentsMusicxmlevents.Clear();

	score->compile();
	// analyse the default repeat-sequence from "score" to "measureMark" and "markList"
	analyseMeasure();
	// create a new score, from the original one, without the measures
	if (compiled_score != NULL)
		delete compiled_score;
	compiled_score = new c_score_partwise(*score, false);
	// remove an existing Expresseur Part
	removeExpresseurPart();
	// overload the parameters read from the score, with the optional data available in the input text file
	if (useMarkFile)
		readMarks();
	else
		writeMarks();
	// create the list of measures, according to repetitions
	createListMeasures();
	// build the sequence of measures in the compiled parts
	addExpresseurPart();
	buildMeasures();
	// compile the score and build the notes to play in lMusicxmlevents
	compileScore();
	//  add lOrnaments in the notes to play in lMusicxmlevents
	addOrnaments();
	// lMusicxmlevents contains the notes to play. Compile lMusicxmlevents
	compileMusicxmlevents();
	// add the part for Expresseur, according to lMusicxmlevents
	compileExpresseurPart();
	// write the xml to display
	compiled_score->write(xmlfileout, true);
	// push the events to play to the LUA-script
	pushLuaMusicxmlevents();

	nbEvents = lMusicxmlevents.GetCount();
}
bool musicxmlcompile::isOk(bool check_compiled_score)
{
	// return true if the C++ score is OK

	if (score == NULL)
		return false;
	if (score->part_list == NULL)
		return false;
	if (score->parts.GetCount() == 0)
		return false;
	if (check_compiled_score)
	{
		if (compiled_score == NULL)
			return false;
		if (compiled_score->part_list == NULL)
			return false;
		if (compiled_score->parts.GetCount() == 0)
			return false;
		if (compiled_score->part_list->score_parts.GetCount() == 0)
			return false;
	}
	return true;
}
void musicxmlcompile::pushLuaMusicxmlevents()
{
	// push to LUA the musicXemEvents compiled
	basslua_call(moduleScore, functionScoreInitScore, "");
	
	lMusicxmlevents.Sort(musicXmlEventsCompareStart);
	getMarkNr(-1);
	getMeasureNr(-1);
	int nr_musicxmlevent;
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(), nr_musicxmlevent = 0; iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++, nr_musicxmlevent++)
	{
		c_musicxmlevent *m = *iter_musicxmlevent;
		char buflua[256];
		strcpy(buflua, m->lua.c_str());

		int markNr = getMarkNr(m->original_measureNr);
		int measureNr = getMeasureNr(m->original_measureNr);
		int measureLength = m->twelve_division_measure / 12 ;
		// push the event itself
#define pushLUAparameters "iiiiiiiiisiiiiiiiiiii"
		basslua_call(moduleScore, functionScoreAddEvent, pushLUAparameters,
			m->played , m->visible, 
			(m->partNr) + 1, m->pitch, m->velocity, m->delay ,
			m->dynamic, m->random_delay, m->pedal,
			buflua ,
			(m->will_stop_index) + 1, (m->stop_index) + 1,
			markNr, measureNr, measureLength,
			m->start_measureNr,m->start_twelve_t / 12, m->start_order, 
			m->stop_measureNr, m->stop_twelve_t / 12, m->stop_order			
			);
		// push the starts of the event
		int nb = m->starts.GetCount();
		for (int n = 0; n < nb; n++)
			basslua_call(moduleScore, functionScoreAddEventStarts, "i",( m->starts[n]) + 1);
		// push the stops of the event
		nb = m->stops.GetCount();
		for (int n = 0; n < nb; n++)
			basslua_call(moduleScore, functionScoreAddEventStops, "i", (m->stops[n]) + 1);
	}
	// push the tracks to LUA
	int nb_tracks = getTracksCount();
	for (int n = 0; n < nb_tracks; n++)
	{
		wxString s = getTrackName(n);
		char buf[256];
		strcpy(buf, s.c_str());
		basslua_call(moduleScore, functionScoreAddTrack, "s", buf);
	}
	// finish the push
	basslua_call(moduleScore, functionScoreFinishScore, "");
}
void musicxmlcompile::clearLuaScore()
{
	basslua_call(moduleScore, functionScoreInitScore, "");
}
void musicxmlcompile::analyseMeasure()
{
	// analyse the default repeat-sequence from "score" to "measureMark" and "markList"

	analyseMeasureMarks();

	sortMeasureMarks();
	
	analyseList();

	int nbMarks = lMeasureMarks.GetCount();
	int nbList = markList.GetCount();
	if ((nbMarks < 3) && (nbList < 3))
	{
		lMeasureMarks.Clear();
		markList.Clear();
	}
	// for debug :
	//l_measureMark::iterator iter_measureMark;
	//for (iter_measureMark = lMeasureMarks.begin(); iter_measureMark != lMeasureMarks.end(); ++iter_measureMark)
	//{
	//	c_measureMark *current_measureMark = *iter_measureMark;
	//	int i = current_measureMark->number;
	//	wxString s = current_measureMark->name;
	//}
	//wxArrayInt::iterator iter_markList;
	//for (iter_markList = markList.begin(); iter_markList != markList.end(); ++iter_markList)
	//{
	//	int current_markList = *iter_markList;
	//	int i = current_markList;
	//}
}
int musicxmlcompile::getMarkNr(int measureNr)
{
	static int cgetMarkNr = 1;
	static int measureNrgetMarkNr = -1;
	if (measureNr < 0)
	{
		cgetMarkNr = 1;
		measureNrgetMarkNr = 1;
		return 1;
	}
	if (measureNr == measureNrgetMarkNr)
		return cgetMarkNr;
	measureNrgetMarkNr = measureNr;
	l_measureMark::iterator iter_measure_mark;
	for (iter_measure_mark = lMeasureMarks.begin(); iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark)
	{
		c_measureMark *measureMark = *iter_measure_mark;
		if (measureMark->number == measureNr)
		{
			cgetMarkNr++;
			return cgetMarkNr;
		}
	}
	return cgetMarkNr;
}
int musicxmlcompile::getMeasureNr(int measureNr)
{
	static int cMeasureNr = 0;
	static int pMeasureNr = -1;
	if (measureNr < 0)
	{
		cMeasureNr = 0;
		pMeasureNr = -1;
		return 0;
	}
	if (measureNr != pMeasureNr)
		cMeasureNr++;
	pMeasureNr = measureNr;
	return cMeasureNr;
}
void musicxmlcompile::analyseMeasureMarks()
{
	// extract from the current score : rehearsal, special barlines, repeat, coda, segno ..., filling :
	//   - measureMark : marks read from barlines and rehearsal 
	//   - markList : list of marks to play , acording to repeat signs

	// extract from the current score lOrnaments list
	c_measureMark *measureMark;
	bool mark; 
	int partNr = 0;
	l_part::iterator iter_part;
	for (iter_part = score->parts.begin(); iter_part != score->parts.end(); ++iter_part , ++partNr )
	{
		mark = (partNr == 0);
		c_part *part = *iter_part;
		int nbMeasure = part->measures.GetCount();
		l_measure::iterator iter_measure;
		for (iter_measure = part->measures.begin(); iter_measure != part->measures.end(); ++iter_measure)
		{
			c_measure *measure = *iter_measure;
			int timeMeasure = 0;
			if (measure->attributes != NULL)
			{
				if ((measure->attributes->divisions != NULL_INT) && (partNr == 0))
				{
					c_ornament *ornament = new c_ornament(o_divisions, measure->number, 0, -1, -1, -1, false, wxString::Format("%d", measure->attributes->divisions));
					lOrnaments.Append(ornament);
				}
			}
			l_measure_sequence::iterator iter_sequence;
			for (iter_sequence = measure->measure_sequences.begin(); iter_sequence != measure->measure_sequences.end(); ++iter_sequence)
			{
				measureMark = new c_measureMark(measure->number);
				c_measure_sequence *sequence = *iter_sequence;
				switch (sequence->type)
				{
				case t_barline:
					if ( partNr == 0 )
					{
						c_barline *barline = (c_barline *)(sequence->pt);
						// bar_style in { regular, dotted, dashed, heavy, light-light, light-heavy, heavy-light, heavy-heavy, tick, short, none }
						wxChar c = barline->bar_style[0];
						switch (c)
						{
						case 'h':
						case 'H':
						case 'l':
						case 'L':
							mark = true;
							break;
						default:break;
						}
						if (barline->location == "right")
						{
							measureMark->changeMeasure(measure->number + 1);
							if (measure->number == nbMeasure)
							{
								measureMark->name = END_OF_THE_SCORE;
							}
						}
						if ((barline->repeat) && (barline->repeat->direction == "forward"))
						{
							mark = true;
							measureMark->repeatForward = true;
						}
						if ((barline->repeat) && (barline->repeat->direction == "backward"))
						{
							mark = true;
							measureMark->repeatBackward = true;
						}
						if ((barline->ending) && (barline->ending->type == "start") && (barline->ending->number[0] == '1'))
						{
							mark = true;
							measureMark->jumpnext = true;
						}
					}
					break;
				case t_direction:
				{
					c_direction *direction = (c_direction *)(sequence->pt);
					l_direction_type::iterator iter_direction_type;
					l_direction_type direction_type = direction->direction_types;
					for (iter_direction_type = direction_type.begin(); iter_direction_type != direction_type.end(); ++iter_direction_type)
					{
						c_direction_type *direction_type = *iter_direction_type;
						switch (direction_type->type)
						{
						case t_rehearsal:
							mark = true;
							measureMark->name = ((c_rehearsal*)(direction_type->pt))->value;
							measureMark->rehearsal = true;
							break;
						case t_wedge:
						{
							c_wedge *current_wedge = (c_wedge*)(direction_type->pt);
							wxString type = current_wedge->type.MakeLower();
							if (type == "crescendo")
							{
								lOrnaments.Append(new c_ornament(o_crescendo, measure->original_number, timeMeasure, partNr, -1, -1, false, ""));
							}
							else if (type == "diminuendo")
							{
								lOrnaments.Append(new c_ornament(o_diminuendo, measure->original_number, timeMeasure, partNr, -1, -1 , false, ""));
							}
						}
							break;
						case t_pedal:
						{
							c_pedal *pedal = (c_pedal*)(direction_type->pt);
							wxString s = pedal->type.Lower();
							if (s == "start")
								lOrnaments.Append(new c_ornament(o_pedal, measure->original_number, timeMeasure, partNr, -1, -1, false, ""));
						}
						default: break;
						}
					}
					l_sound::iterator iter_sound;
					l_sound sound = direction->sounds;
					for (iter_sound = sound.begin(); iter_sound != sound.end(); ++iter_sound)
					{
						c_sound *sound = *iter_sound;
						mark = true;
						if (sound->name == "dacapo")
						{
							measureMark->changeMeasure(measure->number + 1);
							measureMark->dacapo = true;
						}
						else if (sound->name == "coda")
						{
							measureMark->name = "coda";
							measureMark->coda = true;
						}
						else if (sound->name == "segno")
						{
							measureMark->segno = true;
							measureMark->name = "segno";
						}
						else if (sound->name == "fine")
						{
							measureMark->changeMeasure(measure->number + 1);
							measureMark->fine = true;
						}
						else if (sound->name == "tocoda")
						{
							measureMark->changeMeasure(measure->number + 1);
							measureMark->tocoda = true;
						}
						else if (sound->name == "dalsegno")
						{
							measureMark->changeMeasure(measure->number + 1);
							measureMark->dalsegno = true;
						}
					}
				}
					break;
				case t_note:
				{
					c_note *note = (c_note *)(sequence->pt);
					if (note->chord)
						timeMeasure -= (note->duration == NULL_INT) ? 0 : note->duration;
					analyseNoteOrnaments(note, measure->number, timeMeasure);
					timeMeasure += (note->duration == NULL_INT) ? 0 : note->duration;
				}
					break;
				case t_backup:
				{
					c_backup *backup = (c_backup *)(sequence->pt);
					timeMeasure -= backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *forward = (c_forward *)(sequence->pt);
					timeMeasure += forward->duration;
				}
					break;
				default:
					break;
				}
				if (mark)
					lMeasureMarks.Append(measureMark);
				else
					delete measureMark;
				mark = false;
			}
		}
	}
}
void musicxmlcompile::analyseNoteOrnaments(c_note *note, int measureNumber, int t)
{
	// extract the list of arnaments from the note read in the muscixml source file
	if (note->grace)
	{
		wxString s;
		wxString sep;
		wxString alter;
		if (grace.IsEmpty() == false)
		{
			if (note->chord)
				sep = "+";
			else
				sep = ",";
		}
		switch (note->pitch->alter)
		{
		case -2: alter = "bb"; break;
		case -1: alter = "b"; break;
		case 1: alter = "#"; break;
		case 2: alter = "##"; break;
		default: alter = ""; break;
		}
		s.Printf("%s%s%s%d", sep, note->pitch->step, alter, note->pitch->octave);
		grace.Append(s);
	}
	else
	{
		if (grace.IsEmpty() == false)
		{
			lOrnaments.Append(new c_ornament(o_grace, measureNumber, t, note->partNr, note->staff, -1, false, grace));
			grace.Empty();
		}
	}

	if (note && (note->notations))
	{
		if (note->notations->arpeggiate)
		{
			if (note->notations->arpeggiate->direction == "down")
				lOrnaments.Append(new c_ornament(o_arpeggiate, measureNumber, t, note->partNr, note->staff, -1, false, "down"));
			else
				lOrnaments.Append(new c_ornament(o_arpeggiate, measureNumber, t, note->partNr, note->staff, -1, false, "up"));
		}
		if ((note->notations->lOrnaments) && (note->notations->lOrnaments->lOrnaments.GetCount() > 0))
		{
			wxString stype = note->notations->lOrnaments->lOrnaments[0].Lower();
			if (stype == "inverted-mordent")
			{
				lOrnaments.Append(new c_ornament(o_mordent, measureNumber, t,  note->partNr, note->staff, -1, false, "inverted"));
			}
			else if (stype == "mordent")
			{
				lOrnaments.Append(new c_ornament(o_mordent, measureNumber, t, note->partNr, note->staff, -1, false, ""));
			}
			else if (stype == "inverted-turn")
			{
				lOrnaments.Append(new c_ornament(o_turn, measureNumber, t, note->partNr, note->staff, -1, false, "inverted"));
			}
			else if (stype == "turn")
			{
				lOrnaments.Append(new c_ornament(o_turn, measureNumber, t,  note->partNr, note->staff, -1, false, ""));
			}
			/*
			else if (stype == "delayed-inverted-turn")
			{
				lOrnaments.Append(new c_ornament(o_delayed_turn, measureNumber, t, partNr, -1, false, "inverted"));
			}
			else if (stype == "delayed-turn")
			{
				lOrnaments.Append(new c_ornament(o_delayed_turn, measureNumber, t, partNr, -1, false, ""));
			}
			*/
			else if (stype == "trill-mark")
			{
				if (((note->mtype == "whole") || (note->mtype == "half")) && (note->dots == 0))
					lOrnaments.Append(new c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "8"));
				else if (((note->mtype == "whole") || (note->mtype == "half")) && (note->dots > 0))
					lOrnaments.Append(new c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "12"));
				else if (note->dots > 0)
					lOrnaments.Append(new c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "6"));
				else
					lOrnaments.Append(new c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "4"));
			}
		}
		if ((note->notations->articulations) && (note->notations->articulations->articulations.GetCount() > 0))
		{
			wxArrayString::iterator iter_articulations;
			c_articulations *current_articulation = note->notations->articulations;
			for (iter_articulations = current_articulation->articulations.begin(); iter_articulations != current_articulation->articulations.end(); ++iter_articulations)
			{
				wxString stype = (*iter_articulations).Lower();
				if (stype.Contains("accent"))
				{
					lOrnaments.Append(new c_ornament(o_accent, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
				else if (stype == "tenuto")
				{
					lOrnaments.Append(new c_ornament(o_tenuto, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
				else if (stype == "staccato")
				{
					lOrnaments.Append(new c_ornament(o_staccato, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
			}
		}
	}
}
void musicxmlcompile::analyseList()
{
	// capture list of marks in meausureMarks, into markList

	unsigned int markPlayed = 0; // start at the beginning of the score
	bool inDacapo = false;
	int inRepeat = 0;
	unsigned int markRepeat = 0;
	int markSegno = 0;
	int antiloop = 0;
	while (true)
	{
		// specific markers
		if (lMeasureMarks[markPlayed]->segno)
			markSegno = markPlayed;
		if ((inRepeat == 2) && (lMeasureMarks[markPlayed]->repeatBackward) && (markPlayed != markRepeat))
		{
			inRepeat = 0;
		}
		if ((inRepeat == 0) && (inDacapo == false) && (lMeasureMarks[markPlayed]->repeatForward))
		{
			markRepeat = markPlayed;
			inRepeat = 1;
		}

		if ((inRepeat != 1) && (inDacapo == false) && (lMeasureMarks[markPlayed]->dacapo))
		{
			markPlayed = 0;
			inDacapo = true;
		}
		else if ((inRepeat != 1) && (inDacapo == false) && (lMeasureMarks[markPlayed]->dalsegno))
		{
			markPlayed = markSegno;
			inDacapo = true;
		}
		else if ((inRepeat == 1) && (lMeasureMarks[markPlayed]->repeatBackward) && (markPlayed != markRepeat))
		{
			markPlayed = markRepeat;
			inRepeat = 2;
		}
		else if ((inRepeat == 2) && (lMeasureMarks[markPlayed]->jumpnext))
		{
			markPlayed++;
			inRepeat = 0;
		}
		else if ((inDacapo) && (lMeasureMarks[markPlayed]->fine))
		{
			break;
		}
		else if ((inDacapo) && (lMeasureMarks[markPlayed]->tocoda))
		{
			bool found = false;
			l_measureMark::iterator iter_measureMark;
			int nrMark = 0;
			for (iter_measureMark = lMeasureMarks.begin(); iter_measureMark != lMeasureMarks.end(); ++iter_measureMark, ++nrMark)
			{
				c_measureMark *current_measureMark = *iter_measureMark;
				if (current_measureMark->coda)
				{
					found = true;
					markPlayed = nrMark;
				}
			}
			if (found == false)
				break;
		}
		else
		{
			markList.Add(markPlayed);
			markPlayed++;
		}
		if (markPlayed >= lMeasureMarks.GetCount())
			break;
		if ((antiloop++) > 1000)
		{
			wxASSERT(false);
			break;
		}
	}
}
void musicxmlcompile::sortMeasureMarks()
{
	// sort the list of markers, by measure number 

	lMeasureMarks.Sort(measureMarkCompare);
	// suppress markers with same measure number
	l_measureMark::iterator iter_measureMark;
	l_measureMark::iterator prev_iter_measureMark;
	int prev_number = -1;
	c_measureMark *prev_measureMark = NULL;
	for (iter_measureMark = lMeasureMarks.begin(); iter_measureMark != lMeasureMarks.end();)
	{
		c_measureMark *measureMark = *iter_measureMark;
		if (measureMark->number == prev_number)
		{
			if (measureMark->rehearsal)
			{
				measureMark->merge(*prev_measureMark);
				iter_measureMark = lMeasureMarks.erase(prev_iter_measureMark);
				if (lMeasureMarks.GetCount() == 0)
					break;
			}
			else
			{
				prev_measureMark->merge(*measureMark);
				iter_measureMark = lMeasureMarks.erase(iter_measureMark);
				if (lMeasureMarks.GetCount() == 0)
					break;
			}
		}
		else
		{
			prev_number = measureMark->number;
			prev_measureMark = measureMark;
			prev_iter_measureMark = iter_measureMark;
			iter_measureMark ++;
		}
	}
}
void musicxmlcompile::singleOrnaments()
{
	// clear the ornaments duplicated 
	if (lOrnaments.GetCount() < 2)
		return;
	lOrnaments.Sort(ornamentCompare);
	c_ornament *prev_ornament = lOrnaments[0];
	l_ornament::iterator iter_ornament;
	for (iter_ornament = lOrnaments.begin(), iter_ornament++ ; iter_ornament != lOrnaments.end();)
	{
		c_ornament *ornament = *iter_ornament;
		if (ornament->isSameAs(prev_ornament))
		{
			iter_ornament = lOrnaments.erase(iter_ornament);
			if (lOrnaments.GetCount() == 0)
				break;
		}
		else
		{
			prev_ornament = ornament;
			iter_ornament++;
		}
	}
}
void musicxmlcompile::clearOrnaments()
{
	// clear the ornaments, except the divisions 
	l_ornament::iterator iter_ornament;
	for (iter_ornament = lOrnaments.begin(); iter_ornament != lOrnaments.end();)
	{
		c_ornament *ornament = *iter_ornament;
		if (ornament->type != o_divisions)
		{
			iter_ornament = lOrnaments.erase(iter_ornament);
			if (lOrnaments.GetCount() == 0)
				break;
		}
		else
			iter_ornament++;
	}
	/*
	wxl_ornamentNode *node = lOrnaments.GetFirst();
	while (node)
	{
	c_ornament *m = node->GetData();
	if (m->type != o_divisions)
	{
	lOrnaments.DeleteObject(m);
	node = lOrnaments.GetFirst();
	}
	else
	{
	node = node->GetNext();
	}
	}
	*/
}
void musicxmlcompile::writeMarks()
{
	// write in a text file <inputfilename> with txt extension :
	// FILE
	// musicXml file
	// SET_MARKS
	// measure_number=label ..
	// PLAY_MARKS
	// markList ...
	// SET_TRACKS
	// trackname : view play
	// SET_ORNAMENTS
	// meausureNr[.beat[.time[.chord_order]]][*repeat][/track][<]=ornament_name
	
	wxTextFile f;
	wxString sf = txtFile.GetFullPath();
	if (! txtFile.FileExists())
		f.Create(sf);
	f.Open(sf);
	if (!f.IsOpened())
	{
		wxLogError("writeMarks : Error on file %s", sf);
		return;
	}
	f.Clear();

	wxString s;

	s.Printf("%s : %s", SET_MUSICXML_FILE, musicxmlFile.GetFullName());
	f.AddLine(s);

	f.AddLine("");
	s.Printf("%s :", SET_MARKS);
	f.AddLine(s);
	sortMeasureMarks();
	if (lMeasureMarks.GetCount() == 0)
	{
		f.AddLine("-- nothing defined.");
		f.AddLine("-- Example with \"A\" on measure 1, and \"B\" on measure 9 :");
		f.AddLine("--   1:A");
		f.AddLine("--   9:B");
	}
	else
	{
		l_measureMark::iterator iter_measure_mark;
		for (iter_measure_mark = lMeasureMarks.begin(); iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark)
		{
			c_measureMark *measureMark = *iter_measure_mark;
			if (!measureMark->name.IsSameAs(END_OF_THE_SCORE))
			{
				s.Printf("%d:%s", measureMark->number, measureMark->name);
				f.AddLine(s);
			}
		}
	}

	f.AddLine("");
	s.Printf("%s :", SET_PLAY_MARKS);
	f.AddLine(s);
	if (lMeasureMarks.GetCount() == 0)
	{
		f.AddLine("-- Nothing defined.");
		f.AddLine("-- Example to organize MARKS A and B : ");
		f.AddLine("--  A");
		f.AddLine("--  B");
		f.AddLine("--  A");
	}
	else
	{
		wxArrayInt::iterator iter_int;
		for (iter_int = markList.begin(); iter_int != markList.end(); ++iter_int)
		{
			int mark = *iter_int;
			if ((mark >= 0) && (mark < (int)(lMeasureMarks.GetCount())))
			{
				c_measureMark *measureMark = lMeasureMarks[mark];
				s = measureMark->name;
				if (!s.IsSameAs(END_OF_THE_SCORE))
				{
					f.AddLine(s);
				}
			}
		}
	}

	f.AddLine("");
	s.Printf("%s :", SET_PARTS);
	f.AddLine(s);
	l_score_part::iterator iter_score_part;
	// list the parts
	unsigned int partNr = 0;
	for (iter_score_part = compiled_score->part_list->score_parts.begin(), partNr = 0; iter_score_part != compiled_score->part_list->score_parts.end(); ++iter_score_part, partNr++)
	{
		c_score_part *current = *iter_score_part;
		wxString alias;
		if (current->part_alias != current->part_name)
		{
			alias.Printf(",alias=%s/%s", current->part_alias, current->part_alias_abbreviation);
		}
		s.Printf("P%d_%s:%s/%s%s", partNr + 1, current->part_name, 
			current->play ? PART_PLAYED : PART_NOT_PLAYED, current->view ? PART_VISIBLE : PART_NOT_VISIBLE,
			alias);
		f.AddLine(s);
	}

	f.AddLine("");
	s.Printf("%s :", SET_ORNAMENTS);
	f.AddLine(s);
	singleOrnaments();
	l_ornament::iterator iter_ornament;
	for (iter_ornament = lOrnaments.begin(); iter_ornament != lOrnaments.end(); ++iter_ornament)
	{
		c_ornament *ornament = *iter_ornament;
		wxString sr , st , srepeatNr ,  strack , sbefore , sname;
		int tInBeat , beat ;
		int division_beat, division_quarter, division_measure;
		division_beat = getDivision(ornament->measureNumber, &division_quarter, &division_measure);
		beat = ornament->t / division_beat;
		tInBeat = ornament->t % division_beat;
		if ((tInBeat != 0) || (ornament->chord_order > 0))
		{
			if (ornament->chord_order <= 0)
				st.Printf(".%d.%d", beat + 1, tInBeat);
			else
				st.Printf(".%d.%d.%d", beat + 1, tInBeat, ornament->chord_order + 1);
		}
		else
			st.Printf(".%d", beat + 1);
		if (ornament->repeat != -1)
			srepeatNr.Printf("*%d", ornament->repeat);
		if (ornament->partNr != -1)
			strack.Printf("@P%d_%s", ornament->partNr + 1, getTrackName(ornament->partNr));
		if (ornament->before)
			sbefore = "<";
		if (ornament->value.IsEmpty() == false)
			sname.Printf("%s=%s", ornamentName[ornament->type], ornament->value);
		else
			sname = ornamentName[ornament->type];
		if (ornament->absolute_measureNr)
			sr = "!";
		else
		{
			if ((ornament->mark_prefix >= 0) && (ornament->mark_prefix < (int)(lMeasureMarks.GetCount())))
				sr.Printf("%s.", lMeasureMarks[ornament->mark_prefix]->name);
		}
		s.Printf("%s%d%s%s%s%s:%s",sr, ornament->measureNumber, st, srepeatNr, sbefore, strack, sname);
		f.AddLine(s);
	}

	f.Write();
	f.Close();
}
bool musicxmlcompile::readMarkLine(wxString s, wxString sectionName )
{
	bool ret_code = true;
	if (sectionName == SET_MARKS)
	{
		wxString s_name;
		wxString s_number;
		bool number_ok;
		long l_number = 0;
		if (s.Contains(":"))
		{
			s_number = s.BeforeFirst(':', &s_name).Trim(true).Trim(false);
			number_ok = s_number.ToLong(&l_number);
		}
		else
		{
			s_number = s.Trim(true).Trim(false);
			number_ok = s.ToLong(&l_number);
		}
		if (!number_ok)
			return false;
		c_measureMark *measureMark = new c_measureMark(l_number);
		s_name.Trim(true).Trim(false);
		if (s_name.IsEmpty() == false)
			measureMark->name = s_name;
		lMeasureMarks.Append(measureMark);
		return true;
	}
	if (sectionName == SET_PLAY_MARKS)
	{
		s.Trim(true).Trim(false);
		if (s.IsEmpty() == false)
		{
			l_measureMark::iterator iter_measure_mark;
			for (iter_measure_mark = lMeasureMarks.begin(); iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark)
			{
				c_measureMark *measureMark = *iter_measure_mark;
				if (measureMark->name == s)
				{
					markList.Add(lMeasureMarks.IndexOf(measureMark));
					return true;
				}
			}
			return false;
		}
	}
	if (sectionName == SET_PARTS)
	{
		// nr_name :[played]/[visible]
		wxString r;
		wxString id_name = s.BeforeLast(':',&r);
		int partNb = 0;
		int partNr = getPartNr(id_name , &partNb );
		if (partNr == wxNOT_FOUND)
			return false;
		c_score_part *current = compiled_score->part_list->score_parts[partNr];
		current->view = ! ( r.Contains(PART_NOT_VISIBLE)); // view
		current->play = !(r.Contains(PART_NOT_PLAYED)); // play
		wxString alias = s.AfterFirst('=');
		if (!(alias.IsEmpty()))
		{
			wxString abbreviation;
			wxString part_alias = alias.BeforeFirst('/', &abbreviation);
			current->part_alias = part_alias;
			if (!(abbreviation.IsEmpty()))
				current->part_alias_abbreviation = abbreviation;
		}
		return true;
	}
	if (sectionName == SET_ORNAMENTS)
	{
		// [!|mark.]measureNr[.beat[.time[.orderChord]]][*repeat][@track[.staff]][<]:ornament_name[=value]
		wxString sornament_name_value, sornament_name, sornament_value;
		wxString position;
		position = s.BeforeFirst(':', &sornament_name_value);
		if (sornament_name_value.Contains("="))
			sornament_name = sornament_name_value.BeforeFirst('=', &sornament_value);
		else
			sornament_name = sornament_name_value;
		bool absolute = false;
		if (position.StartsWith("!"))
		{
			position = position.Mid(1);
			absolute = true;
		}
		int nr_marker = -1 ;
		if (position.Find('.') != wxNOT_FOUND)
		{
			wxString mmarker = position.BeforeFirst('.');
			int nr = 0;
			l_measureMark::iterator iter_measure_mark;
			for (iter_measure_mark = lMeasureMarks.begin(), nr= 0; iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark, nr++ )
			{
				c_measureMark *measureMark = *iter_measure_mark;
				if (measureMark->name.IsSameAs(mmarker))
				{
					nr_marker = nr ;
					mmarker = position.BeforeFirst('.',&position);
					break;
				}
			}
		}

		bool before = false;
		if (position.EndsWith("<"))
		{
			position = position.BeforeLast('<');
			before = true;
		}
		int trackNr = -1;
		int staffNr = -1;
		if (position.Contains("@"))
		{
			wxString strack;
			position = position.BeforeLast('@', &strack);
			if (strack.Contains("."))
			{
				wxString sstaff;
				strack = strack.BeforeLast('.', &sstaff);
				long l;
				if (sstaff.ToLong(&l))
					staffNr = l;
				else 
					return false;
			}
			int n = getPartNr(strack);
			if (n == wxNOT_FOUND)
				return false;
			trackNr = n;
		}
		int repeat = -1;
		if (position.Contains("*"))
		{
			wxString srepeat;
			position = position.BeforeLast('*', &srepeat);
			long l;
			if (srepeat.ToLong(&l))
				repeat = l;
			else
				return false;
		}
		int beat = 0;
		int tInBeat = 0;
		int chord_order = -1;
		if (position.Contains("."))
		{
			wxString stime;
			position = position.BeforeFirst('.', &stime);
			if (stime.Contains("."))
			{
				wxString sBeat, stInBeat;
				sBeat = stime.BeforeFirst('.', &stInBeat);
				long l;
				if (sBeat.ToLong(&l))
					beat = l;
				if (stInBeat.Contains("."))
				{
					wxString sChordOrder, stInBeat2;
					stInBeat2 = stInBeat.BeforeFirst('.', &sChordOrder);
					long l;
					if (stInBeat2.ToLong(&l))
						tInBeat = l;
					if (sChordOrder.ToLong(&l))
						chord_order = l - 1;
				}
				else
				{
					if (stInBeat.ToLong(&l))
						tInBeat = l;
				}
			}
			else
			{
				long l;
				if (stime.ToLong(&l))
					beat = l;
			}
		}
		int measure_nr = -1;
		long l;
		if (position.ToLong(&l))
			measure_nr = l;
		else
			return false;
		int nr_ornament = -1;
		sornament_name.MakeLower().Trim();
		sornament_value.Trim();
		for (int i = 0; i < o_flagend; i++)
		{
			if (ornamentName[i] == sornament_name)
			{
				nr_ornament = i;
				break;
			}
		}
		if ((nr_ornament == -1) || (measure_nr < 0))
			return false;
		if (nr_ornament != o_divisions)
		{
			c_ornament *ornament = new c_ornament(nr_ornament, measure_nr, NULL_INT, trackNr, staffNr, repeat, before, sornament_value);
			ornament->tInBeat = tInBeat;
			ornament->beat = beat;
			ornament->chord_order = chord_order;
			ornament->absolute_measureNr = absolute;
			ornament->mark_prefix = nr_marker;
			lOrnaments.Append(ornament);
		}
	}
	return ret_code;
}
int musicxmlcompile::getDivision(int measure_nr, int *division_quarter, int *division_measure)
{
	c_part *part = compiled_score->parts[0];
	if ((measure_nr < 1) || (measure_nr > (int)(part->measures.GetCount())))
	{
		*division_quarter = 1;
		*division_measure = 4;
		return 1;
	}
	c_measure *measure = part->measures[measure_nr-1];
	*division_quarter = measure->division_quarter;
	*division_measure = measure->division_measure;
	return measure->division_beat;
}
int musicxmlcompile::getPartNr(wxString spart , int *partNb)
{
	// spart = id_name , return partnr
	int nbPart = score->part_list->score_parts.GetCount();
	if (partNb)
		*partNb = nbPart;
	wxString r;
	if (spart.StartsWith("P", &r) || spart.StartsWith("p", &r))
	{
		long l;
		bool ret = r.ToLong(&l);
		if (ret && (l > 0) && (l <= nbPart))
			return (l - 1);
		l_score_part::iterator iter_score_part;
		wxString s1, r;
		int partNr = 0;
		for (iter_score_part = score->part_list->score_parts.begin(), partNr = 0; iter_score_part != score->part_list->score_parts.end(); ++iter_score_part, partNr++)
		{
			c_score_part *current = *iter_score_part;
			s1.Printf("P%d_%s", partNr + 1, current->part_name);
			if ((spart == s1) || ( current->part_alias == spart ))
				return(partNr);
		}
	}
	return(wxNOT_FOUND);
}
void musicxmlcompile::readMarks(bool full)
{
	// read optional parameters, from a text file <inputfilename> with txt extension :
	// SET_MARKS
	// measure_number=label ..
	// PLAY_MARKS
	// markList ...
	// SET_ORNAMENTS
	// measureNr[.time[*repeat[/track]]][<]=ornament_name

	wxTextFile f;
	f.Open(txtFile.GetFullPath());
	if (f.IsOpened() == false)
		return;
	wxString line;
	wxString sectionName;
	musicxmlFile.Clear();
	bool err = false;
	int line_nb = f.GetLineCount();
	for (int line_nr = 0; line_nr < line_nb; line_nr++)
	{
		line = f.GetLine(line_nr);
		wxString s = line.Upper().Trim();
		// ret_code = true;
		if ((s.IsEmpty() == false ) && (s.StartsWith(COMMENT_EXPRESSEUR) == false))
		{
			if (s.StartsWith(SET_MUSICXML_FILE))
			{
				sectionName = "";
				wxString file = line.AfterFirst(':').Trim(true).Trim(false);
				musicxmlFile =  txtFile;
				musicxmlFile.SetFullName(file);
			}
			else if ( full && (s.StartsWith(SET_PLAY_MARKS)))
			{
				sortMeasureMarks();
				sectionName = SET_PLAY_MARKS;
				markList.Clear();
			}
			else if (full && (s.StartsWith(SET_MARKS)))
			{
				lMeasureMarks.DeleteContents(true);
				lMeasureMarks.Clear();
				sectionName = SET_MARKS;
			}
			else if (full && (s.StartsWith(SET_PARTS)))
			{
				sectionName = SET_PARTS;
			}
			else if (full && (s.StartsWith(SET_ORNAMENTS)))
			{
				sectionName = SET_ORNAMENTS;
				clearOrnaments();
			}
			else if (full)
			{
				if (!readMarkLine(line, sectionName))
				{
					f.RemoveLine(line_nr);
					f.InsertLine("-- error : " + line, line_nr);
					err = true;
				}
				// ret_code = readMarkLine(line, sectionName);
			}
		}
		// correctLines.Add(ret_code);
	}
	if ( full )
		sortMeasureMarks();
	if ( err )
		f.Write();
	f.Close();
}
int musicxmlcompile::compileNote(c_part *part, c_note *note, int measureNr, int originalMeasureNr, int t, int division_measure, int division_beat, int division_quarter, int repeat, int key_fifths)
{
	// compile a note in the lMusicxmlevents
	if ((note->grace)  || (note->rest) || (note->cue) || ((note->tie) && ((note->tie->stop) || (note->tie->compiled))))
	{
		if (! note->chord)
			t += (note->duration == NULL_INT) ? 0 : note->duration;
		return t;
	}

	if (note->chord)
		t -= (note->duration == NULL_INT) ? 0 : note->duration;

	int pitch = 64;
	if ( note->pitch )
		pitch = note->pitch->toMidiPitch();

	int stop_measureNr, stop_t;
	stop_measureNr = measureNr; 
	stop_t = t;
	compileTie(part, note, &stop_measureNr, &stop_t, division_measure);

	c_musicxmlevent *mmusicxmlevent = new c_musicxmlevent(part->idNr,note->staff, note->voice, measureNr, originalMeasureNr, t, stop_measureNr, stop_t, pitch, division_measure, division_beat, division_quarter, repeat, 0, key_fifths);
	mmusicxmlevent->chord_order = note->chord_order;
	if (note->notehead.IsSameAs("x",false))
		mmusicxmlevent->velocity = 0;

	lMusicxmlevents.Append(mmusicxmlevent);

	t += (note->duration == NULL_INT) ? 0 : note->duration;

	return t;
}
void musicxmlcompile::compileTie(c_part *part, c_note *note, int *measureNr , int *t , int division_measure )
{
	// compile the note->tie in the part, to calculate note->realDuration
	// it updates measureNr and t with the end of the note, according to note->tie, which ties additional next notes

	int current_division_measure = division_measure;
	*t += note->duration;
	if (*t >= current_division_measure)
	{
		*t = 0;
		(*measureNr)++;
	}
	if ((note->tie == NULL) || (note->tie->start == false))
		return;
	// scan a note which can be tied to current note
	l_measure::iterator iter_measure;
	for (iter_measure = part->measures.begin(); iter_measure != part->measures.end(); iter_measure++)
	{
		int current_t = 0;
		c_measure *current_measure = *iter_measure;
		if (current_measure->number > (*measureNr))
			return ;
		if (current_measure->number == (*measureNr))
		{
			current_division_measure = current_measure->division_measure;
			l_measure_sequence::iterator iter_measure_sequence;
			for (iter_measure_sequence = current_measure->measure_sequences.begin(); iter_measure_sequence != current_measure->measure_sequences.end(); iter_measure_sequence++)
			{
				c_measure_sequence *current_measure_sequence = *iter_measure_sequence;
				switch (current_measure_sequence->type)
				{
				case t_note:
				{
					c_note *current_note = (c_note *)(current_measure_sequence->pt);
					// process the note 
					if (current_note->chord)
						current_t -= current_note->duration;
					if (note->pitch && current_note->pitch && (note->pitch->isEqual(*(current_note->pitch))) && (note->voice == current_note->voice))
					{
						if (((*measureNr) == current_measure->number) && ((*t) == current_t))
						{
							if (current_note->tie->stop)
							{
								current_note->tie->compiled = true;
								compileTie(part, current_note, measureNr, t, current_division_measure);
								return;
							}
						}
					}
					current_t += current_note->duration;
				}
					break;
				case t_backup:
				{
					c_backup *current_backup = (c_backup *)(current_measure_sequence->pt);
					current_t -= current_backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *current_forward = (c_forward *)(current_measure_sequence->pt);
					current_t += current_forward->duration;
				}
					break;
				default:
					break;
				}
			}
		}
	}
	return ;
}
void musicxmlcompile::compilePedalBar()
{
	if (compiled_score->pedal_bar == 0)
		return;
	// add a pedal on each bar
	int p_measureNr = -1;
	l_musicxmlevent pedalPending;
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); ++iter_musicxmlevent)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		if (musicxmlevent->start_measureNr != p_measureNr)
		{
			c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
			m->played = false;
			m->visible = false;
			m->start_order = -128;
			m->pedal = compiled_score->pedal_bar;
			pedalPending.Append(m);
			p_measureNr = musicxmlevent->start_measureNr;
		}
	}
	for (iter_musicxmlevent = pedalPending.begin(); iter_musicxmlevent != pedalPending.end(); ++iter_musicxmlevent)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		lMusicxmlevents.Append(musicxmlevent);
	}
	pedalPending.DeleteContents(false);
}
void musicxmlcompile::compileCrescendo()
{
	l_musicxmlevent::iterator iter_musicxmlevent;
	int startMusicxmlevent[MAX_SCORE_PART];
	int pVelocity[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
		pVelocity[i] = 64;

	// fix all velocities, not yet defined by an ornament-velocity
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); ++iter_musicxmlevent)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		if (musicxmlevent->nuance != NULL_INT)
			pVelocity[musicxmlevent->partNr] = musicxmlevent->nuance;
		musicxmlevent->velocity = pVelocity[musicxmlevent->partNr];
		musicxmlevent->velocity += musicxmlevent->accent;
		if (musicxmlevent->velocity > 127)
			musicxmlevent->velocity = 127;
		if (musicxmlevent->velocity < 1)
			musicxmlevent->velocity = 1;
	}

	bool crescendo_pending[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
	{
		crescendo_pending[i] = false;
		pVelocity[i] = 64;
	}
	int nrEvent = 0;
	// fix crescendos
	for (iter_musicxmlevent = lMusicxmlevents.begin(), nrEvent = 0; iter_musicxmlevent != lMusicxmlevents.end(); ++iter_musicxmlevent, nrEvent++)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		if ((crescendo_pending[musicxmlevent->partNr] == false) && (musicxmlevent->nuance != NULL_INT))
			pVelocity[musicxmlevent->partNr] = musicxmlevent->nuance;
		if (musicxmlevent->crescendo)
		{
			// marker on this crescendo, on this part
			startMusicxmlevent[musicxmlevent->partNr] = nrEvent;
			crescendo_pending[musicxmlevent->partNr] = true;
		}
		else
		{
			if ((crescendo_pending[musicxmlevent->partNr]) && (musicxmlevent->nuance != NULL_INT))
			{
				// crescendo pending : applied up this nuance
				crescendo_pending[musicxmlevent->partNr] = false;
				int cPartNr = musicxmlevent->partNr;
				c_musicxmlevent *musicxmleventStart = lMusicxmlevents[startMusicxmlevent[cPartNr]];
				c_musicxmlevent *musicxmleventStop = musicxmlevent;
				int dt = (musicxmleventStop->start_measureNr - musicxmleventStart->start_measureNr)*(musicxmleventStart->twelve_division_measure) + (musicxmleventStop->start_twelve_t - musicxmleventStart->start_twelve_t);
				int start_velocity = pVelocity[musicxmlevent->partNr];
				int stop_velocity = musicxmlevent->nuance;
				for (int i = startMusicxmlevent[cPartNr]; i < nrEvent; i++)
				{
					c_musicxmlevent *musicxmlevent2 = lMusicxmlevents[i];
					if (musicxmlevent2->partNr == cPartNr)
					{
						int dtl = (musicxmlevent2->start_measureNr - musicxmleventStart->start_measureNr)*(musicxmleventStart->twelve_division_measure) + (musicxmlevent2->start_twelve_t - musicxmleventStart->start_twelve_t);
						musicxmlevent2->velocity = start_velocity + ((stop_velocity - start_velocity) * dtl) / dt;
					}
				}
			}
		}
	}
}
void musicxmlcompile::compileTransposition()
{
	// fix transposition
	int pTransposition[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
		pTransposition[i] = 0;
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); ++iter_musicxmlevent)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		if (musicxmlevent->transpose != NULL_INT)
			pTransposition[musicxmlevent->partNr] = musicxmlevent->transpose;
		musicxmlevent->pitch += pTransposition[musicxmlevent->partNr];
	}
}
void musicxmlcompile::compileArppegio()
{
	// proccess pending arpegiatte
	if (lArpeggiate_toapply.GetCount() == 0)
		return;
	lArpeggiate_toapply.Sort(arpeggiateCompare);
	int dt = 0;
	l_arpeggiate_toapply::iterator iter_arpeggiate_toapply;
	int pnr = lArpeggiate_toapply[0]->nr;
	for (iter_arpeggiate_toapply = lArpeggiate_toapply.begin(), iter_arpeggiate_toapply++; iter_arpeggiate_toapply != lArpeggiate_toapply.end(); ++iter_arpeggiate_toapply)
	{
		c_arpeggiate_toapply *arpeggiate_toapply = *iter_arpeggiate_toapply;
		if (arpeggiate_toapply->nr == pnr)
		{
			dt += (arpeggiate_toapply->before) ? -1 : 1;
			arpeggiate_toapply->musicxmlevent->start_order = dt;
			arpeggiate_toapply->musicxmlevent->visible = false;
		}
		else
		{
			pnr = arpeggiate_toapply->nr;
			dt = 0;
		}
	}

}
wxString musicxmlcompile::pitchToString(int pitch)
{
	const  char *cpitch[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
	int p = pitch % 12;
	int o = pitch / 12 - 1;
	wxString s;
	s.Printf("%s%d", cpitch[p], o);
	return s;
}
wxString musicxmlcompile::pitchToString(wxArrayInt pitches)
{
	wxString s;
	bool first = true;
	wxArrayInt::iterator iter;
	for (iter = pitches.begin(); iter != pitches.end(); ++iter)
	{
		int p = *iter;
		wxString sp = pitchToString(p);
		if ( ! first )
			s.Append(",");
		s.Append(sp);
	}
	return s;
}
wxArrayInt musicxmlcompile::stringToPitch(wxString s, int *nbChord)
{
	// return array of pitches ( a chord ), extracted from string s in first call, chord by chord
	// e.g. stringToPitch("C4/E4,G4") returns first 64,68 , and nbChord=2
	//      then stringToPitch("") returns 71 
	//      then stringToPitch("") return empty arrary
	static wxArrayInt t;
	static unsigned int pt;
	if (s.IsEmpty() == false)
	{
		*nbChord = 0;
		t.Clear();
		pt = 0;
		wxStringTokenizer tokenizer(s, "+,", wxTOKEN_STRTOK);
		while (tokenizer.HasMoreTokens())
		{
			wxChar sep = tokenizer.GetLastDelimiter();
			wxString token = tokenizer.GetNextToken();
			char cp = token.GetChar(0);
			int p = 0;
			switch (cp)
			{
			case 'C': p = 0; break;
			case 'D': p = 2; break;
			case 'E': p = 4; break;
			case 'F': p = 5; break;
			case 'G': p = 7; break;
			case 'A': p = 9; break;
			case 'B': p = 11; break;
			default: break;
			}
			if (token.Contains("bb"))
				p -= 2;
			else if (token.Contains("b"))
				p--;
			else if (token.Contains("##"))
				p += 2;
			else if (token.Contains("#"))
				p++;
			cp = token.Last();
			switch (cp)
			{
			case '0': p += 1 * 12; break;
			case '1': p += 2 * 12; break;
			case '2': p += 3 * 12; break;
			case '3': p += 4 * 12; break;
			case '4': p += 5 * 12; break;
			case '5': p += 6 * 12; break;
			case '6': p += 7 * 12; break;
			case '7': p += 8 * 12; break;
			case '8': p += 9 * 12; break;
			case '9': p += 10* 12; break;
			default: break;
			}
			while (p < 0)
				p += 12;
			while (p > 127)
				p -= 12;
			if (sep == '+')
				p *= (-1);
			else
				(*nbChord)++;
			t.Add(p);
		}
	}
	wxArrayInt ap;
	if (pt >= t.GetCount())
		return ap;
	ap.Add(abs(t[pt]));
	pt++;
	while (true)
	{
		if ((pt >= t.GetCount()) || (t[pt] > 0))
			return ap;
		ap.Add(abs(t[pt]));
		pt++;
	}
}
void musicxmlcompile::addGraces(wxArrayInt gracePitches, bool before, c_musicxmlevent *musicxmlevent)
{
	int nbChord = gracePitches.GetCount() ;
	c_musicxmlevent *mtemplate = new c_musicxmlevent(*musicxmlevent);
	musicxmlevent->visible = true;
	musicxmlevent->played = true;
	mtemplate->visible = false;
	mtemplate->played = true;
	mtemplate->stop_measureNr = mtemplate->start_measureNr;
	mtemplate->stop_twelve_t = mtemplate->start_twelve_t;
	int start_order = musicxmlevent->start_order;
	if (before)
		musicxmlevent->start_order = start_order + 0;
	else
		musicxmlevent->start_order = start_order + 2 * nbChord;
	for (int nrChord = 0; nrChord < nbChord; nrChord++)
	{
		if (before)
		{
			mtemplate->start_order = start_order - 2 * (nbChord - nrChord);
			mtemplate->stop_order = start_order - 2 * (nbChord - nrChord) + 1;
		}
		else
		{
			mtemplate->start_order = start_order + 2 * nrChord;
			mtemplate->stop_order = start_order + 2 * nrChord + 1;
		}
		c_musicxmlevent *m = new c_musicxmlevent(*mtemplate);
		m->pitch = gracePitches[nrChord];
		lOrnamentsMusicxmlevents.Append(m);
	}
	delete mtemplate;
}
void musicxmlcompile::addGraces(wxString gracePitches, bool before, c_musicxmlevent *musicxmlevent)
{
	wxArrayInt pitchChord;
	int nbChord;
	pitchChord = stringToPitch(gracePitches, &nbChord);
	int nrChord = 0;
	c_musicxmlevent *mtemplate = new c_musicxmlevent(*musicxmlevent);
	musicxmlevent->visible = true;
	musicxmlevent->played = true;
	mtemplate->visible = false;
	mtemplate->played = true;
	mtemplate->stop_measureNr = mtemplate->start_measureNr;
	mtemplate->stop_twelve_t = mtemplate->start_twelve_t;
	int start_order = musicxmlevent->start_order;
	if (before)
		musicxmlevent->start_order = start_order + 0;
	else
		musicxmlevent->start_order = start_order + 2 * nbChord;
	while (pitchChord.IsEmpty() == false)
	{
		if (before)
		{
			mtemplate->start_order = start_order - 2 * (nbChord - nrChord);
			mtemplate->stop_order = start_order - 2 * (nbChord - nrChord) + 1;
		}
		else
		{
			mtemplate->start_order = start_order + 2 * nrChord;
			mtemplate->stop_order = start_order + 2 * nrChord + 1;
		}
		int nbPitch = pitchChord.GetCount();
		for (int nrPitch = 0; nrPitch < nbPitch; nrPitch++)
		{
			c_musicxmlevent *m = new c_musicxmlevent(*mtemplate);
			m->pitch = pitchChord[nrPitch];
			lOrnamentsMusicxmlevents.Append(m);
		}
		nrChord++;
		pitchChord = stringToPitch("", NULL);
	}
	delete mtemplate;
}
void musicxmlcompile::addOrnament(c_ornament *ornament, c_musicxmlevent *musicxmlevent, int nr_ornament)
{
	// add the ornament in the musicxmlevent
	bool btrill = false;
	switch (ornament->type)
	{
	case o_delay:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->delay = v;
		break;
	}
	case o_transpose:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->transpose = v;
		break;
	}
	case o_lua:
		musicxmlevent->lua = ornament->value;
		break;
	case o_pianissimo:
		musicxmlevent->nuance = 10;
		break;
	case o_piano:
		musicxmlevent->nuance = 30;
		break;
	case o_mesopiano:
		musicxmlevent->nuance = 50;
		break;
	case o_mesoforte:
		musicxmlevent->nuance = 70;
		break;
	case o_forte:
		musicxmlevent->nuance = 90;
		break;
	case o_fortissimo:
		musicxmlevent->nuance = 100;
		break;
	case o_crescendo:
		musicxmlevent->crescendo = true;
		break;
	case o_diminuendo:
		musicxmlevent->crescendo = true;
		break;
	case o_tenuto:
		musicxmlevent->tenuto = true;
		break;
	case o_staccato:
	{
		c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
		musicxmlevent->visible = true;
		musicxmlevent->played = false;
		m->visible = false;
		m->played = true;
		if (ornament->value == "2/3")
		{
			m->twelve_duration = (musicxmlevent->twelve_duration * 2) / 3;
			m->stop_twelve_t = musicxmlevent->start_twelve_t + musicxmlevent->twelve_duration;
		}
		else if (ornament->value == "1/3")
		{
			m->twelve_duration = musicxmlevent->twelve_duration / 3;
			m->stop_twelve_t = musicxmlevent->start_twelve_t + musicxmlevent->twelve_duration;
		}
		else
		{
			m->twelve_duration = musicxmlevent->twelve_duration / 2;
			m->stop_twelve_t = musicxmlevent->start_twelve_t + musicxmlevent->twelve_duration;
		}
		lOrnamentsMusicxmlevents.Append(m);
		break;
	}
	case o_accent:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->accent = v;
		break;
	}
	case o_grace:
	{
		wxArrayInt gracePitches;
		if (ornament->value.IsEmpty())
		{
			gracePitches.Add(c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths));
			addGraces(gracePitches, ornament->before, musicxmlevent);
		}
		else if (ornament->value.StartsWith("i"))
		{
			gracePitches.Add(c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths));
			addGraces(gracePitches, ornament->before, musicxmlevent);
		}
		else
			addGraces(ornament->value, ornament->before, musicxmlevent);
		break;
	}
	case o_mordent:
	{
	    wxArrayInt gracePitches;
		gracePitches.Add(musicxmlevent->pitch);
		gracePitches.Add(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? -1 : 1, musicxmlevent->fifths));
		addGraces(gracePitches, ornament->before, musicxmlevent); 
		break;
	}
	case o_turn:
	{
		wxArrayInt gracePitches;
		gracePitches.Add(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? -1 : 1, musicxmlevent->fifths));
		gracePitches.Add(musicxmlevent->pitch);
		gracePitches.Add(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? 1 : -1, musicxmlevent->fifths));
		addGraces(gracePitches, ornament->before, musicxmlevent);
		break;
	}
	case o_btrill:
		btrill = true;
	case o_trill:
	{
		wxArrayInt gracePitches;
		int p0, p1;
		if (btrill)
		{
			p0 = c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths);
			p1 = musicxmlevent->pitch;
		}
		else
		{
			p0 = musicxmlevent->pitch;
			p1 = c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths);
		}
		long l;
		int v = 2;
		if (ornament->value.ToLong(&l))
			v = l;
		for (int i = 0; i < v; i++)
		{
			gracePitches.Add(p0);
			if (i != (v - 1))
				gracePitches.Add(p1);
		}
		addGraces(gracePitches, ornament->before, musicxmlevent);
		musicxmlevent->pitch = p1;
		break;
	}
	case o_arpeggiate:
		lArpeggiate_toapply.Append(new c_arpeggiate_toapply(100 * musicxmlevent->repeat + nr_ornament, (ornament->value == "down"), ornament->before, musicxmlevent));
		break;
	case o_dynamic:
	{
		c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
		m->played = false;
		m->visible = false;
		m->start_order = -128;
		long l;
		int v = 100;
		if (ornament->value.ToLong(&l))
			v = l;
		m->dynamic = v;
		lOrnamentsMusicxmlevents.Append(m);
		break;
	}
	case o_random_delay:
	{
		c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
		m->played = false;
		m->visible = false;
		m->start_order = -128;
		long l;
		int v = 0;
		if (ornament->value.ToLong(&l))
			v = l;
		m->random_delay = v;
		lOrnamentsMusicxmlevents.Append(m);
		break;
	}
	case o_text :
	{
		c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
		m->played = false;
		m->visible = false;
		m->start_order = -128;
		m->text = ornament->value;
		lOrnamentsMusicxmlevents.Append(m);
		break;
	}
	case o_pedal:
	{
		c_musicxmlevent *m = new c_musicxmlevent(*musicxmlevent);
		m->played = false;
		m->visible = false;
		m->start_order = -128;
		long l;
		int v = 65;
		if (ornament->value.ToLong(&l))
			v = l;
		m->pedal = v;
		lOrnamentsMusicxmlevents.Append(m);
		break;
	}
	case o_after:
		(musicxmlevent->start_order) += 2;
		break;
	case o_before:
		(musicxmlevent->start_order) -= 2;
		break;
	case o_pedal_bar:
	{
		long l;
		int v = 65;
		if (ornament->value.ToLong(&l))
			v = l;
		compiled_score->pedal_bar = v;
		break;
	}
	default: break;
	}

}
void musicxmlcompile::createOrnament(c_ornament *ornament)
{
	if (ornament->type == o_divisions)
		return;
	// look for nex start, to finish a new virtual note for the ornament
	l_musicxmlevent::iterator iter_musicxmlevent;
	c_musicxmlevent *after_musicxmlevent = NULL;
	bool found = false;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		after_musicxmlevent = *iter_musicxmlevent;
		if ((after_musicxmlevent->start_measureNr > ornament->measureNumber) || ((after_musicxmlevent->start_measureNr == ornament->measureNumber) && (after_musicxmlevent->start_twelve_t > ornament->twelve_t)))
		{
			found = true;
			break;
		}
	}
	if (!found)
		return;
	if (ornament->twelve_t >= after_musicxmlevent->twelve_division_measure)
		return;
	// add the ornament in the musicxmlevent
	c_musicxmlevent *m = new c_musicxmlevent(ornament->partNr, 0, 0,
		ornament->measureNumber, ornament->measureNumber, ornament->t, 
		after_musicxmlevent->start_measureNr, after_musicxmlevent->start_twelve_t, 0,
		after_musicxmlevent->twelve_division_measure, after_musicxmlevent->twelve_division_beat, after_musicxmlevent->twelve_division_quarter,
		0, 0, 0);
	m->played = true;
	m->visible = true;
	addOrnament(ornament, m,0);
	lMusicxmlevents.Append(m);
}
void musicxmlcompile::addOrnaments()
{
	//  add lOrnaments in the lMusicxmlevents
	lMusicxmlevents.Sort(musicXmlEventsCompareStart);
	lOrnaments.Sort(ornamentCompare);
	lOrnamentsMusicxmlevents.DeleteContents(true);
	lOrnamentsMusicxmlevents.Clear();

	l_ornament::iterator iter_ornament;
	for (iter_ornament = lOrnaments.begin(); iter_ornament != lOrnaments.end(); ++iter_ornament)
	{
		c_ornament *ornament = *iter_ornament;
		if (ornament->t == NULL_INT)
		{
			int division_beat, division_quarter, division_measure;
			division_beat  = getDivision(ornament->measureNumber, &division_quarter, &division_measure);
			ornament->t = ornament->tInBeat + (ornament->beat - 1) * division_beat;
			ornament->twelve_t = 12 * ornament->t;
		}
	}

	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *musicxmlevent = *iter_musicxmlevent;
		l_ornament::iterator iter_ornament;
		int nr_ornament;
		for (iter_ornament = lOrnaments.begin(), nr_ornament = 0; iter_ornament != lOrnaments.end(); ++iter_ornament, nr_ornament++)
		{
			c_ornament *ornament = *iter_ornament;
			bool measureOk = false;
			if (ornament->absolute_measureNr)
			{
				measureOk = (musicxmlevent->start_measureNr == ornament->measureNumber);
			}
			else
			{
				if (ornament->mark_prefix ==  -1 )
				{
					measureOk = (musicxmlevent->original_measureNr == ornament->measureNumber);
				}
				else
				{
					if ((ornament->mark_prefix >= 0) && (ornament->mark_prefix < (int)(lMeasureMarks.GetCount())))
						measureOk = (musicxmlevent->original_measureNr == ornament->measureNumber + lMeasureMarks[ornament->mark_prefix]->number - 1);
				}
			}
			if
			(		measureOk
				   && (musicxmlevent->start_twelve_t == ornament->twelve_t)
				   && ((ornament->chord_order < 0) || (musicxmlevent->chord_order == ornament->chord_order))
				   && ((ornament->partNr < 0) || (musicxmlevent->partNr == ornament->partNr))
				   && ((ornament->staffNr < 0) || (musicxmlevent->staffNr == NULL_INT) || (musicxmlevent->staffNr == ornament->staffNr))
				   && ((ornament->repeat < 0) || (musicxmlevent->repeat == ornament->repeat))
				)
			{
				addOrnament(ornament, musicxmlevent,nr_ornament);
				ornament->processed = true;
			}
		}
	}
	compileCrescendo();
	compileTransposition();
	compileArppegio();
	compilePedalBar();

	for (iter_ornament = lOrnaments.begin(); iter_ornament != lOrnaments.end(); ++iter_ornament)
	{
		c_ornament *ornament = *iter_ornament;
		if	(ornament->processed == false)
		{
			createOrnament(ornament);
		}
	}

	for (iter_musicxmlevent = lOrnamentsMusicxmlevents.begin(); iter_musicxmlevent != lOrnamentsMusicxmlevents.end(); iter_musicxmlevent ++)
	{
		c_musicxmlevent *ornament_musicxmlevent = *iter_musicxmlevent;
		lMusicxmlevents.Append(ornament_musicxmlevent);
	}
	lOrnamentsMusicxmlevents.DeleteContents(false);
	lOrnamentsMusicxmlevents.Clear();
	lMusicxmlevents.Sort(musicXmlEventsCompareStart);

}
void musicxmlcompile::compileMusicxmlevents(bool second_time)
{
	// lMusicxmlevents contains the notes to play. Compile lMusicxmlevents

	lMusicxmlevents.Sort(musicXmlEventsCompareStart);
	/////////////////////////////////////////////////
	

	// set musicxmlevent->nr in the order
	l_musicxmlevent::iterator iter_musicxmlevent;
	int nr_musicxmlevent;
	int nb_measure = 0;
	for (iter_musicxmlevent = lMusicxmlevents.begin(), nr_musicxmlevent = 0; iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++, nr_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		nb_measure = current_musicxmlevent->stop_measureNr;
		current_musicxmlevent->nr = nr_musicxmlevent;
		if (second_time)
		{
			current_musicxmlevent->stop_orpheline = true;
			current_musicxmlevent->stops.Clear();
			current_musicxmlevent->starts.Clear();
			current_musicxmlevent->stop_index = -1;
			current_musicxmlevent->will_stop_index = -1;
		}
	}
	if ( second_time == false )
	{ 
		// add a fake element at the end 
		c_musicxmlevent *last_musicxmlevent = new c_musicxmlevent();
		last_musicxmlevent->nr = nr_musicxmlevent + 1;
		last_musicxmlevent->visible = false;
		last_musicxmlevent->played = false;
		last_musicxmlevent->start_measureNr = nb_measure + 1;
		last_musicxmlevent->stop_measureNr = last_musicxmlevent->start_measureNr;
		last_musicxmlevent->start_twelve_t = 0;
		last_musicxmlevent->stop_twelve_t = 0;
		lMusicxmlevents.Append(last_musicxmlevent);
	}

	// links the starts and stops in the list of Musicxmlevents to play
	int p_MeasureNr = -1;
	int p_t = -1;
	int p_order = -1;
	bool p_tenuto = false;
	c_musicxmlevent *p_musicxmlevent = NULL;

	lMusicxmlevents.Sort(musicXmlEventsCompareStop);
	/////////////////////////////////////////////////

	// link synchronous-stops
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if (current_musicxmlevent->played)
		{
			if ((current_musicxmlevent->stop_measureNr != p_MeasureNr) || (current_musicxmlevent->stop_twelve_t != p_t) || (current_musicxmlevent->stop_order != p_order) || (current_musicxmlevent->tenuto != p_tenuto))
				p_musicxmlevent = current_musicxmlevent;
			if (p_musicxmlevent)
			{
				p_musicxmlevent->stops.Add(current_musicxmlevent->nr);
				if (p_musicxmlevent != current_musicxmlevent)
					current_musicxmlevent->stop_orpheline = false;
			}

			p_MeasureNr = current_musicxmlevent->stop_measureNr;
			p_t = current_musicxmlevent->stop_twelve_t;
			p_order = current_musicxmlevent->stop_order;
			p_tenuto = current_musicxmlevent->tenuto;
		}
	}

	lMusicxmlevents.Sort(musicXmlEventsCompareStart);
	/////////////////////////////////////////////////


	// link synchronous-starts
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	p_tenuto = false;
	p_musicxmlevent = NULL;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if (current_musicxmlevent->played)
		{
			if ((current_musicxmlevent->start_measureNr != p_MeasureNr) || (current_musicxmlevent->start_twelve_t != p_t) || (current_musicxmlevent->start_order != p_order))
				p_musicxmlevent = current_musicxmlevent;
			if (p_musicxmlevent)
				p_musicxmlevent->starts.Add(current_musicxmlevent->nr);

			p_MeasureNr = current_musicxmlevent->start_measureNr;
			p_t = current_musicxmlevent->start_twelve_t;
			p_order = current_musicxmlevent->start_order;
		}
	}

	// links starts with stops-non-tenuto
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	p_tenuto = false;
	p_musicxmlevent = NULL;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if (current_musicxmlevent->played)
		{
			if (current_musicxmlevent->starts.IsEmpty() == false)
				p_musicxmlevent = current_musicxmlevent;
			if ((current_musicxmlevent->stops.IsEmpty() == false) && (current_musicxmlevent->tenuto == false) && (p_musicxmlevent->will_stop_index == -1))
			{
				if (p_musicxmlevent)
				{
					p_musicxmlevent->will_stop_index = current_musicxmlevent->nr;
					current_musicxmlevent->stop_orpheline = false;
				}
			}
		}
	}
	// links stops-tenuto and orpheline-stops-non-tenuto with synchronous starts 
	lMusicxmlevents.Sort(musicXmlEventsCompareStart);
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	p_tenuto = false;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if ((current_musicxmlevent->played) && (current_musicxmlevent->stops.IsEmpty() == false) && ((current_musicxmlevent->tenuto == true) || (current_musicxmlevent->stop_orpheline == true)))
		{
			l_musicxmlevent::iterator iter_musicxmlevent_to;
			for (iter_musicxmlevent_to = iter_musicxmlevent, iter_musicxmlevent_to++; iter_musicxmlevent_to != lMusicxmlevents.end(); iter_musicxmlevent_to++)
			{
				c_musicxmlevent *musicxmlevent_to = *iter_musicxmlevent_to;
				if ((musicxmlevent_to->starts.IsEmpty() == false) && (musicxmlevent_to->start_measureNr == current_musicxmlevent->stop_measureNr) && (musicxmlevent_to->start_twelve_t == current_musicxmlevent->stop_twelve_t))
				{
					musicxmlevent_to->stop_index = current_musicxmlevent->nr;
					current_musicxmlevent->stop_orpheline = false;
					break;
				}
				if (musicxmlevent_to->start_measureNr > current_musicxmlevent->stop_measureNr)
					break;
			}
		}
	}

	// residual stop-orpheline==true will be triggered like a start
	bool orpheline_found = false;
	l_musicxmlevent orpheline_musicxmlevents;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if ((current_musicxmlevent->played) && (current_musicxmlevent->stop_orpheline))
		{
			c_musicxmlevent *orpheline_event = new c_musicxmlevent(*current_musicxmlevent);
			orpheline_event->start_measureNr = current_musicxmlevent->stop_measureNr;
			orpheline_event->start_twelve_t = current_musicxmlevent->stop_twelve_t;
			orpheline_event->start_order = current_musicxmlevent->stop_order;
			orpheline_event->stop_measureNr = current_musicxmlevent->stop_measureNr;
			orpheline_event->stop_twelve_t = current_musicxmlevent->stop_twelve_t + 12;
			orpheline_event->stop_order = current_musicxmlevent->stop_order;
			orpheline_event->velocity = 0;
			orpheline_event->pitch = 0;
			orpheline_event->starts.Add(-1);
			orpheline_event->played = true;
			orpheline_event->visible = true;
			l_musicxmlevent::iterator iter_musicxmlevent_after;
			bool found = false;
			for (iter_musicxmlevent_after = iter_musicxmlevent, iter_musicxmlevent_after++; iter_musicxmlevent_after != lMusicxmlevents.end(); iter_musicxmlevent_after++)
			{
				c_musicxmlevent *musicxmlevent_after = *iter_musicxmlevent_after;
				if ((musicxmlevent_after->visible == false) || (musicxmlevent_after->starts.IsEmpty()))
					continue;
				if (musicxmlevent_after->start_measureNr < orpheline_event->start_measureNr)
					continue;
				if ((musicxmlevent_after->start_measureNr == orpheline_event->start_measureNr) && (musicxmlevent_after->start_twelve_t <= orpheline_event->start_twelve_t))
					continue;
				orpheline_event->stop_measureNr = musicxmlevent_after->start_measureNr;
				orpheline_event->stop_twelve_t = musicxmlevent_after->stop_twelve_t;
				found = true;
				break;
			}
			if (found)
			{
				orpheline_musicxmlevents.Append(orpheline_event);
				orpheline_found = true;
			}
			else
				delete orpheline_event;
		}
	}
	if (second_time) // finish the secund round
		return;
	if (orpheline_found)
	{
		// orpheline are added. Need to recompile in a second round
		for (iter_musicxmlevent = orpheline_musicxmlevents.begin(); iter_musicxmlevent != orpheline_musicxmlevents.end(); iter_musicxmlevent++)
		{
			c_musicxmlevent *current_orpheline_musicxmlevent = *iter_musicxmlevent;
			lMusicxmlevents.Append(current_orpheline_musicxmlevent);
		}
		orpheline_musicxmlevents.DeleteContents(false);
		orpheline_musicxmlevents.Clear();
		compileMusicxmlevents(true);
	}

	// count ornaments per note
	int prev_start_twelve_t = -1;
	int prev_start_measureNr = -1;
	int nb_order_start_blind = 0;
	l_musicxmlevent lmusicxmlevents_visible;
	for (iter_musicxmlevent = lMusicxmlevents.begin(), nr_musicxmlevent = 0; iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++, nr_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if ((current_musicxmlevent->start_twelve_t != prev_start_twelve_t) || (current_musicxmlevent->start_measureNr != prev_start_measureNr))
		{
			if (lmusicxmlevents_visible.GetCount() > 0)
			{
				l_musicxmlevent::iterator iter_musicxmlevent_visible;
				for (iter_musicxmlevent_visible = lmusicxmlevents_visible.begin(); iter_musicxmlevent_visible != lmusicxmlevents_visible.end(); iter_musicxmlevent_visible++)
				{
					c_musicxmlevent *current_musicxmlevent_visible = *iter_musicxmlevent_visible;
					current_musicxmlevent_visible->nb_ornaments = nb_order_start_blind;
				}
				nb_order_start_blind = 0;
				lmusicxmlevents_visible.DeleteContents(false);
				lmusicxmlevents_visible.Clear();
			}
			prev_start_twelve_t = current_musicxmlevent->start_twelve_t;
			prev_start_measureNr = current_musicxmlevent->start_measureNr;
		}
		if (current_musicxmlevent->visible)
			lmusicxmlevents_visible.Append(current_musicxmlevent);
		else
		{
			if (current_musicxmlevent->starts.GetCount() > 0)
				nb_order_start_blind++;
		}
	}
}
void musicxmlcompile::removeExpresseurPart()
{
	// remove an existing Expresseur Part

	l_score_part::iterator iter_score_part;
	l_part::iterator iter_part;
	for (iter_score_part = compiled_score->part_list->score_parts.begin(), iter_part = compiled_score->parts.begin(); iter_score_part != compiled_score->part_list->score_parts.end(); ++iter_score_part, ++iter_part)
	{
		c_score_part *current_score_part = *iter_score_part;
		c_part *current_part = *iter_part;
		if (current_score_part->id == ExpresseurId)
		{
			compiled_score->part_list->score_parts.DeleteObject(current_score_part);
			compiled_score->parts.DeleteObject(current_part);
			break;
		}
	}
}
void musicxmlcompile::createListMeasures()
{
	// create the list of measures, according to repetitions

	c_part *first_part = score->parts[0];
	if (markList.Count() == 0)
	{
		// no mark list. Play all measures straight forward
		for (unsigned int i = 0; i < first_part->measures.GetCount(); i++)
			measureList.Add(i + 1);
	}
	else
	{
		sortMeasureMarks();
		// follow mark list instructions
		int nbMeasureMarks = lMeasureMarks.GetCount();
		wxArrayInt::iterator iter_markList;
		for (iter_markList = markList.begin(); iter_markList != markList.end(); ++iter_markList)
		{
			int nrMark = *iter_markList;
			if ((nrMark >= 0) && (nrMark < nbMeasureMarks))
			{
				int measureStart = lMeasureMarks.Item(nrMark)->GetData()->number;
				int measureEnd = first_part->measures.GetCount() + 1;
				if (nrMark < (nbMeasureMarks - 1))
				{
					measureEnd = lMeasureMarks.Item(nrMark + 1)->GetData()->number;
				}
				for (int i = measureStart; i < measureEnd; i++)
				{
					/*
					if (i == ( measureEnd - 1))
						measureList.Add((-1)*i);
					else
						measureList.Add(i);
					*/
					measureList.Add(i);
				}
			}
		}
	}
	//wxArrayInt::iterator iter_measureList;
	//for (iter_measureList = measureList.begin(); iter_measureList != measureList.end(); ++iter_measureList)
	//{
	//	int nrMeasure = *iter_measureList;
	//	int i = nrMeasure;
	//}
}
void musicxmlcompile::addExpresseurPart()
{
	c_part *firstPart = (c_part*)(score->parts.GetFirst()->GetData());
	score->part_list->score_parts.Append(new c_score_part(ExpresseurId, "Exp", "X"));
	c_part *part_expresseur = new c_part(ExpresseurId);
	score->parts.Append(part_expresseur);
	// fill the Expresseur part with empty measures
	l_measure::iterator iter_measure;
	for (iter_measure = firstPart->measures.begin(); iter_measure != firstPart->measures.end(); iter_measure++)
	{
		c_measure *measure = *iter_measure;
		c_measure *newMeasure = new c_measure(*measure, false);
		if (measure->number == 1)
		{
			c_attributes *current_attributes = newMeasure->attributes;
			if (current_attributes == NULL)
				current_attributes = new c_attributes();
			if (current_attributes->staff_details)
				delete (current_attributes->staff_details);
			current_attributes->staff_details = new c_staff_details();
			current_attributes->staff_details->staff_lines = 1;
			current_attributes->clefs.DeleteContents(true);
			current_attributes->clefs.Clear();
			current_attributes->staves = 1;
			c_clef *clef = new c_clef();
			clef->line = 1;
			clef->sign = "percussion";
			current_attributes->clefs.Append(clef);
		}
		// delete_bar_label(newMeasure);
		part_expresseur->measures.Append(newMeasure);
	}
	part_expresseur->compile(score->part_list->score_parts.GetCount() - 1);

	compiled_score->part_list->score_parts.Append(new c_score_part(ExpresseurId, "Exp", "X"));
	compiled_score->parts.Append(new c_part(ExpresseurId));
}
void musicxmlcompile::delete_bar_label(c_measure *mmeausre)
{
	// delete double-bars and label
	bool measure_sequence_tobedeleted ;
	l_measure_sequence::iterator iter_measure_sequence;
	for (iter_measure_sequence = mmeausre->measure_sequences.begin(); iter_measure_sequence != mmeausre->measure_sequences.end();)
	{
		c_measure_sequence *current_measure_sequence = *iter_measure_sequence;
		measure_sequence_tobedeleted = false;
		switch (current_measure_sequence->type)
		{
		case t_barline:
		{
			measure_sequence_tobedeleted = true;
			break;
		}
		case t_direction:
		{
			c_direction *direction = (c_direction*)(current_measure_sequence->pt);
			l_direction_type::iterator iter_direction_type;
			bool directiontype_tobedeleted ;
			for (iter_direction_type = direction->direction_types.begin(); iter_direction_type != direction->direction_types.end();)
			{
				c_direction_type *direction_type = *iter_direction_type;
				directiontype_tobedeleted = false;
				switch (direction_type->type)
				{
				case t_segno:
				case t_rehearsal:
					directiontype_tobedeleted = true;
					break;
				case t_words:
				{
					c_words *words = (c_words *)(direction_type->pt);
					wxString s = words->value.Lower();
					if ((s == "fine") || (s.Contains("coda")) || (s.Contains("segno")))
						directiontype_tobedeleted = true;
					break;
				}
				default:
					directiontype_tobedeleted = false;
					break;
				}
				if (directiontype_tobedeleted)
				{
					direction->direction_types.DeleteContents(true);
					iter_direction_type = direction->direction_types.erase(iter_direction_type);
					if (direction->direction_types.GetCount() == 0)
						break;
				}
				else
					iter_direction_type++;
			}
			break;
		}
		}
		if (measure_sequence_tobedeleted)
		{
			mmeausre->measure_sequences.DeleteContents(true);
			iter_measure_sequence = mmeausre->measure_sequences.erase(iter_measure_sequence);
			if (mmeausre->measure_sequences.GetCount() == 0)
				break;
		}
		else
			iter_measure_sequence++;
	}
}
void musicxmlcompile::buildMeasures()
{
	// build the sequence of measures in the compiled parts

	int nrFirstPartVisible = 0;
	l_score_part::iterator iter_compiled_part_list;
	for (iter_compiled_part_list = compiled_score->part_list->score_parts.begin(), nrFirstPartVisible = 0; iter_compiled_part_list != compiled_score->part_list->score_parts.end(); ++iter_compiled_part_list, ++nrFirstPartVisible)
	{
		c_score_part *current_score_part = *iter_compiled_part_list;
		if (current_score_part->view)
			break;
	}

	int nrPart = 0;
	l_part::iterator iter_compiled_part;
	l_part::iterator iter_part;
	for (iter_compiled_part = compiled_score->parts.begin(), iter_part = score->parts.begin(), nrPart = 0; iter_part != compiled_score->parts.end(); ++iter_compiled_part, ++iter_part, ++nrPart)
	{
		c_part *current_part = *iter_part;
		c_part *current_compiled_part = *iter_compiled_part;
		int nbMeasure_current_part = current_part->measures.GetCount();
		int newMeasureNr = 0;
		int nbmeasureList = measureList.GetCount();
		for (int nrmeasureList = 0; nrmeasureList < nbmeasureList; nrmeasureList++)
		{
			int nrMeasure = measureList[nrmeasureList] - 1;
			if ((nrMeasure < 0) || (nrMeasure >= nbMeasure_current_part))
			{
				wxASSERT(false);
				break;
			}
			c_measure *measure = current_part->measures.Item(nrMeasure)->GetData();
			bool barlineTodo = false;
			wxString label;
			if (newMeasureNr == ( nbmeasureList - 1 ))
			{
				barlineTodo = true;
			}
			else
			{
				int nrNextMeasure = measureList[nrmeasureList + 1] - 1;
				if ((nrNextMeasure < 0) || (nrNextMeasure >= nbMeasure_current_part))
				{
					wxASSERT(false);
					break;
				}
				l_measureMark::iterator iter_measure_mark;
				for (iter_measure_mark = lMeasureMarks.begin(); iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark)
				{
					c_measureMark *measureMark = *iter_measure_mark;
					if (!measureMark->name.IsSameAs(END_OF_THE_SCORE))
					{
						if ((measureMark->number - 1) == nrMeasure)
						{
							label = measureMark->name;
						}
						if ((measureMark->number - 1) == nrNextMeasure)
						{
							barlineTodo = true;
						}
					}
				}
			}
			c_measure *newMeasure = new c_measure(*measure);
			measure->repeat++;

			delete_bar_label(newMeasure);
			
			if ((label.IsEmpty() == false) && (nrPart == nrFirstPartVisible))
			{
				// write a label
				if (newMeasure->repeat > 0)
					label.Printf("%s*%d", label, newMeasure->repeat + 1);
				c_words *words = new c_words();
				words->value = label;
				c_direction_type *direction_type = new c_direction_type();
				direction_type->type = t_words;
				direction_type->pt = (void*)(words);
				c_direction *direction = new c_direction();
				direction->placement = "above";
				direction->direction_types.Append(direction_type);
				c_measure_sequence *measure_sequence = new c_measure_sequence();
				measure_sequence->type = t_direction;
				measure_sequence->pt = (void *)(direction);
				newMeasure->measure_sequences.Append(measure_sequence);
			}
			if (barlineTodo)
			{
				// add a barline
				bool barlineFound = false;
				l_measure_sequence::iterator iter_measure_sequence;
				for (iter_measure_sequence = newMeasure->measure_sequences.begin(); iter_measure_sequence != newMeasure->measure_sequences.end(); iter_measure_sequence++)
				{
					c_measure_sequence *current_measure_sequence = *iter_measure_sequence;
					if (current_measure_sequence->type == t_barline)
					{
						c_barline *current_barline = (c_barline *)(current_measure_sequence->pt);
						current_barline->bar_style = (newMeasureNr == (nbmeasureList - 1)) ? "light-heavy" : "light-light";
						barlineFound = true;
					}
				}
				if (!barlineFound)
				{
					c_measure_sequence *measure_sequence = new c_measure_sequence();
					c_barline *barline = new c_barline();
					barline->bar_style = (newMeasureNr == (nbmeasureList - 1)) ? "light-heavy" : "light-light";
					barline->location = "right";
					measure_sequence->pt = (void *)barline;
					measure_sequence->type = t_barline;
					newMeasure->measure_sequences.Append(measure_sequence);
				}
			}
			newMeasure->number = newMeasureNr + 1;
			newMeasureNr++;
			current_compiled_part->measures.Append(newMeasure);
		}
	}
}
void musicxmlcompile::compileScore()
{
	// compile the score and build the notes to play in lMusicxmlevents

	l_part::iterator iter_compiled_part;
	l_score_part::iterator iter_score_part;

	grace.Empty();
	int key_fifths = 0;
	for (iter_compiled_part = compiled_score->parts.begin(), iter_score_part = compiled_score->part_list->score_parts.begin(); iter_compiled_part != compiled_score->parts.end(); ++iter_compiled_part, ++iter_score_part)
	{
		c_part *current_compiled_part = *iter_compiled_part;
		current_compiled_part->idNr = getTrackNr(current_compiled_part->id);
		c_score_part *current_score_part = *iter_score_part;
		l_measure::iterator iter_measure;
		for (iter_measure = current_compiled_part->measures.begin(); iter_measure != current_compiled_part->measures.end(); iter_measure++)
		{
			int current_t = 0;
			c_measure *current_measure = *iter_measure;
			l_measure_sequence::iterator iter_measure_sequence;
			for (iter_measure_sequence = current_measure->measure_sequences.begin(); iter_measure_sequence != current_measure->measure_sequences.end(); iter_measure_sequence++)
			{
				c_measure_sequence *current_measure_sequence = *iter_measure_sequence;
				switch (current_measure_sequence->type)
				{
				case t_note:
				{
					c_note *current_note = (c_note *)(current_measure_sequence->pt);
					// process the note 
					if (current_score_part->play)
						current_t = compileNote(current_compiled_part, current_note, current_measure->number, current_measure->original_number, current_t, current_measure->division_measure, current_measure->division_beat, current_measure->division_quarter, current_measure->repeat, key_fifths);
				}
					break;
					/*
					case t_harmony:
					{
					c_harmony *current_harmony = (c_harmony *)(current_measure_sequence->pt);
					}
					break;
					*/
				case t_backup:
				{
					c_backup *current_backup = (c_backup *)(current_measure_sequence->pt);
					current_t -= current_backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *current_forward = (c_forward *)(current_measure_sequence->pt);
					current_t += current_forward->duration;
				}
					break;
				case t_barline:
					break;
				case t_direction:
					break;
				default:
					break;
				}
			}
		}
	}
}
void musicxmlcompile::compileExpresseurPart()
{
	// add the part for Expresseur, according to lMusicxmlevents

	lMusicxmlevents.Sort(musicXmlEventsCompareStart);

	c_part *part_expresseur = (c_part*)(compiled_score->parts.GetLast()->GetData());;
	l_measure::iterator iter_measure;
	iter_measure = part_expresseur->measures.begin();
	c_measure *currentMeasure = *iter_measure;

	wxString text;

	int currentT = 0;
	l_musicxmlevent::iterator iter_musicxmlevent_from;
	for (iter_musicxmlevent_from = lMusicxmlevents.begin(); iter_musicxmlevent_from != lMusicxmlevents.end(); iter_musicxmlevent_from++)
	{
		c_musicxmlevent *musicxmlevent_from = *iter_musicxmlevent_from;
		if (musicxmlevent_from->text.IsEmpty() == false)
			text = musicxmlevent_from->text;
		if ((musicxmlevent_from->visible == false) || (musicxmlevent_from->starts.IsEmpty()))
			continue;
		int from_start_measureNr = musicxmlevent_from->start_measureNr;
		int from_stop_measureNr = musicxmlevent_from->stop_measureNr;
		int from_startT = musicxmlevent_from->start_twelve_t / 12;
		int from_stopT = musicxmlevent_from->stop_twelve_t / 12;
		if (from_stopT == 0)
		{
			from_stop_measureNr--;
			from_stopT = musicxmlevent_from->twelve_division_measure / 12;
		}
		int to_start_measureNr = from_stop_measureNr + 1;
		int to_stop_measureNr = from_stop_measureNr + 1;
		int to_startT = 0;
		int to_stopT = 0;
		l_musicxmlevent::iterator iter_musicxmlevent_to;
		for (iter_musicxmlevent_to = iter_musicxmlevent_from, iter_musicxmlevent_to++; iter_musicxmlevent_to != lMusicxmlevents.end(); iter_musicxmlevent_to++)
		{
			c_musicxmlevent *musicxmlevent_to = *iter_musicxmlevent_to;
			if (((musicxmlevent_to->visible == false) || (musicxmlevent_to->starts.IsEmpty()))
				|| ((musicxmlevent_to->start_measureNr == from_start_measureNr) && ((musicxmlevent_to->start_twelve_t / 12) == from_startT)))
				continue;;
			to_start_measureNr = musicxmlevent_to->start_measureNr;
			to_stop_measureNr = musicxmlevent_to->stop_measureNr;
			to_startT = musicxmlevent_to->start_twelve_t / 12;
			to_stopT = musicxmlevent_to->stop_twelve_t / 12;
			if (to_stopT == 0)
			{
				to_stop_measureNr--;
				to_stopT = musicxmlevent_to->twelve_division_measure / 12 ;
			}
			break;
		}
		int start_measureNr = from_start_measureNr;
		int stop_measureNr = from_stop_measureNr;
		int startT = from_startT ;
		int stopT = from_stopT;
		if ((to_start_measureNr <= from_stop_measureNr) && (to_startT <= from_stopT))
		{
			stop_measureNr = to_start_measureNr;
			stopT = to_startT;
		}
		while (start_measureNr > currentMeasure->number)
		{
			// finish the current measure with rests
			addNote(currentMeasure, currentT, currentMeasure->division_measure, true, false, false, true, 0,&text);
			// go to next measure
			currentT = 0;
			iter_measure++;
			if (iter_measure == part_expresseur->measures.end())
			{
				wxASSERT(false);
				break;
			}
			currentMeasure = *iter_measure;
		}
		// add rests to fill the gap up to this note
		addNote(currentMeasure, currentT, startT, true, false, false, true, 0, &text);
		currentT = startT;
		bool tie_back = false;
		bool firstNote = true;
		// add the figure-note per measure
		for (int n = start_measureNr; n < stop_measureNr; n++)
		{
			addNote(currentMeasure, startT, currentMeasure->division_measure, false, tie_back, true, firstNote, musicxmlevent_from->nb_ornaments, &text);
			firstNote = false;
			iter_measure++;
			if (iter_measure == part_expresseur->measures.end())
			{
				// wxASSERT(false);
				break;
			}
			currentMeasure = *iter_measure;
			currentT = 0;
			start_measureNr++;
			startT = 0;
			tie_back = true;
		}
		addNote(currentMeasure, currentT, stopT, false, tie_back, false, firstNote, musicxmlevent_from->nb_ornaments, &text);
		firstNote = false;
		currentT = stopT;
	}
	addNote(currentMeasure, currentT, currentMeasure->division_measure, true, false, false, true, 0, &text);

}
void musicxmlcompile::addNote(c_measure *measure, int from_t, int to_t, bool rest, bool tie_back, bool tie_next, bool ifirst_note, int nbOrnaments, wxString *text)
{
	// add symbols for one note [from_t,to_t], to_t <= divisions, inside one measure

	if (from_t == to_t)
		return;

	/*
	if (rest && (from_t == 0) && (to_t == measure->division_measure))
	{
		// full measure rest
		c_rest *rest = new c_rest();
		rest->measure = "yes";
		c_note *note = new c_note();
		note->rest = rest;
		c_measure_sequence *measure_sequence = new c_measure_sequence();
		measure_sequence->pt = (void *)note;
		measure_sequence->type = t_note;
		measure->measure_sequences.Append(measure_sequence);
		return;
	}
	*/

	bool firstNote = ifirst_note;
	int ffrom_t = from_t;
	bool ttie_back = tie_back;
	wxASSERT(measure->division_beat != NULL_INT);
	if ((from_t % measure->division_beat) != 0)
	{
		// insert figures up to next beat
		int t0 = 0;
		while (t0 < from_t)
			t0 += measure->division_beat;
		t0 += measure->division_beat;
		if (t0 < to_t)
		{
			addSymbolNote(measure, t0 - from_t, rest, tie_back, true, firstNote, nbOrnaments, text);
			firstNote = false;
			ttie_back = true;
			ffrom_t = t0;
		}
	}
	// insert figures starting on the beat
	addSymbolNote(measure, to_t - ffrom_t, rest, ttie_back, tie_next, firstNote, nbOrnaments, text);
	firstNote = false;
}
void musicxmlcompile::addSymbolNote(c_measure *measure, int duration, bool rest, bool tie_back, bool tie_next, bool first_note, int nbOrnaments, wxString *text)
{
	c_note *note = NULL ;
	bool firstNote = true;

	int duration_todo = duration;
	int antiLoop = 0;
	while (duration_todo > 0)
	{
		if (antiLoop++ > 10)
			break;

		wxString typeNote;
		int duration_done;
		int dot;
		int tuplet;
		calculateDuration(duration_todo, measure->division_quarter, &duration_done, & typeNote , &dot , &tuplet);
		duration_todo -= duration_done;

		if (typeNote.IsEmpty() == false)
		{
			note = new c_note();

			note->duration = duration_done;
			note->mtype = typeNote;
			note->dots = dot;
			if (tuplet > 1)
			{
				note->time_modification = new c_time_modification();
				switch (tuplet)
				{
				case 5:
					note->time_modification->actual_notes = 5;
					note->time_modification->normal_notes = 4;
					break;
				case 3:
				default:
					note->time_modification->actual_notes = 3;
					note->time_modification->normal_notes = 2;
					break;
				}
			}

			if (rest)
			{
				c_rest *rest = new c_rest();
				note->rest = rest;
			}
			else
			{
				c_pitch *pitch = new c_pitch();
				pitch->unpitched = true;
				pitch->step = "D";
				pitch->octave = 4;
				note->stem = "down";
				note->pitch = pitch;
				if ((firstNote && tie_back) || (!firstNote))
				{
					c_tied *tied = new c_tied();
					tied->stop = true;
					if (note->notations == NULL)
						note->notations = new c_notations();
					note->notations->tied = tied;
					firstNote = false;
				}
				if (duration_todo > 0)
				{
					c_tied *tied = new c_tied();
					tied->start = true;
					if (note->notations == NULL)
						note->notations = new c_notations();
					note->notations->tied = tied;
				}
			}

			if (first_note)
			{
				if (nbOrnaments > 0)
				{
					c_lyric *lyric = new c_lyric();
					lyric->text = wxString::Format("x%d", nbOrnaments + 1);
					//lyric->placement = "below";
					note->lyrics.Append(lyric);

				}
				if (text->IsEmpty() == false)
				{
					c_words *words = new c_words();
					words->value = *text;
					c_direction_type *direction_type = new c_direction_type();
					direction_type->type = t_words;
					direction_type->pt = (void*)(words);
					c_direction *direction = new c_direction();
					direction->placement = "above";
					direction->direction_types.Append(direction_type);
					c_measure_sequence *measure_sequence = new c_measure_sequence();
					measure_sequence->type = t_direction;
					measure_sequence->pt = (void *)(direction);
					measure->measure_sequences.Append(measure_sequence);
					text->Empty();
				}
			}
			c_measure_sequence *measure_sequence = new c_measure_sequence();
			measure_sequence->pt = (void *)note;
			measure_sequence->type = t_note;
			measure->measure_sequences.Append(measure_sequence);
		}
	}

	if ((! rest) && (tie_next) && ( note != NULL ))
	{
		c_tied *tied = new c_tied();
		tied->start = true;
		if (note->notations == NULL)
			note->notations = new c_notations();
		note->notations->tied = tied;
	}
}
void musicxmlcompile::calculateDuration(int duration, int division_quarter, int *durationDone, wxString *typeNote, int *dot, int *tuplet)
{
	*dot = 0;
	*tuplet = 1;

	*durationDone = 4 * division_quarter ;
	if ((*durationDone) <= duration)
	{
		*typeNote = "whole";
		return;
	}
	if ((((2 * division_quarter) * 7) % 4) == 0)
	{
		*durationDone = ((2 * division_quarter) * 7) / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*dot = 2;
			return;
		}
	}
	if ((((2 * division_quarter) * 3) % 2) == 0)
	{
		*durationDone = ((2 * division_quarter) * 3) / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*dot = 1;
			return;
		}
	}
	if ((((4 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((4 * division_quarter) * 2) / 3;
		if ((*durationDone) <= duration)
		{
			*typeNote = "whole";
			*tuplet = 3;
			return;
		}
	}
	*durationDone = 2 * division_quarter;
	if ((*durationDone) <= duration)
	{
		*typeNote = "half";
		return;
	}
	if ((((1 * division_quarter) * 7) % 4) == 0)
	{
		*durationDone = ((1 * division_quarter) * 7) / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*dot = 2;
			return;
		}
	}
	if ((((4 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((4 * division_quarter) * 2) / 5;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*tuplet = 5;
			return;
		}
	}
	if ((((1 * division_quarter) * 3) % 2) == 0)
	{
		*durationDone = ((1 * division_quarter) * 3) / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*dot = 1;
			return;
		}
	}
	if ((((2 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((2 * division_quarter) * 2) / 3;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*tuplet = 3;
			return;
		}
	}
	*durationDone = 1 * division_quarter;
	if ((*durationDone) <= duration)
	{
		*typeNote = "quarter";
		return;
	}
	if (((division_quarter * 7) % (2 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (2 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*dot = 2;
			return;
		}
	}
	if ((((2 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((2 * division_quarter) * 2) / 5;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (2 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (2 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*dot = 1;
			return;
		}
	}
	if ((((1 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((1 * division_quarter) * 2) / 3;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 2) == 0)
	{
		*durationDone = division_quarter / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			return;
		}
	}
	if (((division_quarter * 7) % (4 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (4 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*dot = 2;
			return;
		}
	}
	if ((((1 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((1 * division_quarter) * 2) / 5;
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (4 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (4 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (3 * 2)) == 0)
	{
		*durationDone = (division_quarter * 2) / (3 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 4) == 0)
	{
		*durationDone = division_quarter / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			return;
		}
	}
	if (((division_quarter * 8) % (8 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (8 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*dot = 2;
			return;
		}
	}
	if (((division_quarter * 2) % (2 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (2 * 5);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (8 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (8 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (4 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (4 * 3);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 8) == 0)
	{
		*durationDone = division_quarter / 8;
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			return;
		}
	}
	if (((division_quarter * 7) % (16 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (16 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*dot = 2;
			return;
		}
	}
	if (((division_quarter * 2) % (4 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (4 * 5);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (16 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (16 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (8 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (8 * 3);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 16) == 0)
	{
		*durationDone = division_quarter / 16;
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			return;
		}
	}
	if (((division_quarter * 2) % (8 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (8 * 5);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 2) % (16 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (16 * 3);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*tuplet = 3;
			return;
		}
	}
	if (((division_quarter * 2) % (16 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (16 * 5);
		if ((*durationDone) <= duration)
		{
			*typeNote = "128th";
			*tuplet = 5;
			return;
		}
	}
	if ((division_quarter % 32) == 0)
	{
		*durationDone = duration;
		*typeNote = "128th";
	}
	*durationDone = duration;
	*typeNote = "";
}
bool musicxmlcompile::getInfoEvent(int nrEvent, int *measureNr, int *t480)
{
	if ((nrEvent < 0) || (nrEvent >= nbEvents))
		return false;
	c_musicxmlevent *m = lMusicxmlevents[nrEvent];
	*measureNr = m->start_measureNr;
	*t480 = (480 * m->start_twelve_t) / m->twelve_division_quarter;
	return true;
}
int musicxmlcompile::stringToEventNr(wxString s)
{
	int measureNr = -1;
	int repeat = -1;
	bool absolute = false;
	wxString label =  s;
	wxString srepeat ;
	if (s.Contains("*"))
		label = s.BeforeFirst('*', &srepeat);
	long l;
	if (srepeat.ToLong(&l))
		repeat = l;
	if (s.StartsWith("!"))
	{
		absolute = true;
		label = label.Mid(1);
	}
	if (label.ToLong(&l))
		measureNr = l;
	else
	{
		l_measureMark::iterator iter_measure_mark;
		for (iter_measure_mark = lMeasureMarks.begin(); iter_measure_mark != lMeasureMarks.end(); ++iter_measure_mark)
		{
			c_measureMark *measureMark = *iter_measure_mark;
			if (measureMark->name.IsSameAs(label))
			{
				measureNr = measureMark->number;
				break;
			}
		}
	}
	if (measureNr == -1)
		return -1;
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		if (absolute)
		{
			if (current_musicxmlevent->start_measureNr == measureNr)
				return current_musicxmlevent->nr;
		}
		else
		{
			if (repeat == -1)
			{	
				if (current_musicxmlevent->original_measureNr == measureNr)
					return current_musicxmlevent->nr;
			}
			else
			{
				if ((current_musicxmlevent->original_measureNr == measureNr) && (current_musicxmlevent->repeat == (repeat - 1)))
					return current_musicxmlevent->nr;
			}
		}
	}
	return -1;
}
bool musicxmlcompile::getScorePosition(int nrEvent , int *absolute_measure_nr, int *measure_nr, int *beat, int *t)
{
	if ((nrEvent < 0) || (nrEvent >= nbEvents))
		return false;
	c_musicxmlevent *m = lMusicxmlevents[nrEvent];
	*absolute_measure_nr = m->start_measureNr;
	*measure_nr = m->original_measureNr;
	*beat = m->start_twelve_t / m->twelve_division_beat;
	*t = (m->start_twelve_t % m->twelve_division_beat ) / 12 ;
	return true;
}
wxArrayString  musicxmlcompile::getListOrnament()
{
	wxArrayString a;
	for (int i = o_dynamic; i < o_flagend; i++)
	{
		wxString s = ornamentName[i];
		switch (i)
		{
		case o_dynamic: s = s + "=128 full range , 0 for no dynamic"; break;
		case o_random_delay: s = s + "=t in ms"; break;
		case o_pedal_bar: s = s + "=64 add a pedal on each bar ( 127=full)"; break;
		case o_pedal: s = s + "=64 add a pedal(127=full)"; break;
		case o_lua: s = s + "=special commands like tuning, scale, ..."; break;
		case o_text: s = s + "=my text"; break;
		case o_pianissimo: break ;
		case o_piano: break ;
		case o_mesopiano: break ;
		case o_mesoforte: break ;
		case o_forte: break ;
		case o_fortissimo: break ;
		case o_crescendo: break ;
		case o_diminuendo: break ;
		case o_tenuto: break ;
		case o_staccato: s = s + "=1/2 or 1/3 or 2/3"; break;
		case o_accent: s = s + "=20 to add velocity ( max = 127 )"; break;
		case o_grace: s = s + " up by defaut, or =inverted or=list of pitches like C4/E4,G4"; break;
		case o_mordent: s = s + " normal or =inverted";  break;
		case o_turn: s = s + " normal or =inverted";  break;
		case o_btrill: s = s + " 2 or =nb trills";  break;
		case o_trill: s = s + " 2 or =nb trills";  break;
		case o_arpeggiate: s = s + " up or =down";  break;
		case o_transpose: s = s + "=+/-half-tone"; break;
		case o_delay: s = s + "=t in ms"; break;
		case o_before: break ;
		case o_after: break ;

		default:
			break;
		}
		a.Add(s);
	}
	return a;
}
int musicxmlcompile::measureBeatToEventNr(int measureNr, int beat)
{
	l_musicxmlevent::iterator iter_musicxmlevent;
	for (iter_musicxmlevent = lMusicxmlevents.begin(); iter_musicxmlevent != lMusicxmlevents.end(); iter_musicxmlevent++)
	{
		c_musicxmlevent *current_musicxmlevent = *iter_musicxmlevent;
		int beatEvent = current_musicxmlevent->start_twelve_t / current_musicxmlevent->twelve_division_beat + 1;
		if ((current_musicxmlevent->start_measureNr >= measureNr) && (beatEvent >= beat))
			return current_musicxmlevent->nr;
	}
	return -1;
}
wxString musicxmlcompile::getTitle()
{
	if (isOk() && score->work != NULL)
		return score->work->work_title;
	return wxEmptyString;
}
int musicxmlcompile::getTracksCount()
{
	if (!isOk())
		return 0;
	return (score->part_list->score_parts.GetCount() - 1);
}
wxArrayString musicxmlcompile::getTracksName()
{
	wxArrayString l;

	if (!isOk())
		return l;

	int nr;
	int nb = getTracksCount();
	l_score_part::iterator iter;
	for (iter = score->part_list->score_parts.begin(), nr = 0; nr < nb ; ++iter , nr++)
	{
		c_score_part *current = *iter;
		wxString name = current->part_name;
		l.Add(name);
	}
	return l;
}
wxString musicxmlcompile::getTrackName(int nrTrack)
{
	if (!isOk(true))
		return wxEmptyString;
	wxString s;
	int nbTrack = getTracksCount();
	if ((nrTrack >= 0) && (nrTrack < nbTrack))
	{
		c_score_part *mpart = compiled_score->part_list->score_parts[nrTrack];
		if (mpart->part_alias == NULL_STRING)
			return wxEmptyString;
		return(mpart->part_alias);
	}
	return wxEmptyString;
}
wxString musicxmlcompile::getTrackId(int nrTrack)
{
	if (!isOk())
		return "";

	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return score->part_list->score_parts[nrTrack]->id;

	return "";
}
bool musicxmlcompile::getTrackPlay(int nrTrack)
{
	if (!isOk())
		return false;
	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return(score->part_list->score_parts[nrTrack]->play);
	return false;
}
wxArrayInt musicxmlcompile::getTracksPlay()
{
	if (!isOk())
		return false;
	wxArrayInt a;
	int nr;
	int nb = getTracksCount();
	l_score_part::iterator iter;
	for (iter = score->part_list->score_parts.begin(), nr = 0; nr < nb; ++iter, nr++)
	{
		c_score_part *current = *iter;
		a.Add(current->play);
	}
	return a;
}
bool musicxmlcompile::getTrackDisplay(int nrTrack)
{
	if (!isOk())
		return false;
	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return(score->part_list->score_parts[nrTrack]->view);
	return false;
}
wxArrayInt musicxmlcompile::getTracksDisplay()
{
	if (!isOk())
		return false;
	wxArrayInt a;
	int nr;
	int nb = getTracksCount();
	l_score_part::iterator iter;
	for (iter = score->part_list->score_parts.begin(), nr = 0; nr < nb; ++iter, nr++)
	{
		c_score_part *current = *iter;
		a.Add(current->view);
	}
	return a;
}
int musicxmlcompile::getTrackNr(wxString idTrack)
{
	// return the index of the idTrack ( or wxNOT_FOUND if not found )

	if (!isOk())
		return wxNOT_FOUND;

	int nr;
	int nb = getTracksCount();
	l_score_part::iterator iter;
	for (iter = score->part_list->score_parts.begin(), nr = 0; nr < nb; ++iter, nr++)
	{
		c_score_part *current_score_part = *iter;
		if (current_score_part->id == idTrack)
			return nr;
	}
	return wxNOT_FOUND;
}
