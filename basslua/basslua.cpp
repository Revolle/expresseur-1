/**
* \file basslua.c
* \brief Midi-IN Event LUA processor 
* \author Franck Revolle
* \version 1.1
* \date 05 10 2015
// update : 01/12/2016 21:00
* Objective :
*   This module :
*     - manages hardware :
*         - MIDI-IN real-time devices
*         - internal timer
*         - external User-interface
*     - manages an LUA script file to process the hardware events

* External dependency :
*   bass-DLL for midi-in management ( www.un4seen.com )
*
* platform :  MAC PC
*
*/

//////////////////////////////////////////////

#ifdef _WIN32
#define V_PC 1
#define V_CPP 1
#ifdef _WIN64
#define V_PC 1
#define V_CPP 1
#endif
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
#define V_MAC 1
#else
// Unsupported platform
#endif
#elif __linux
// linux
#elif __unix // all unices not caught above
// Unix
#elif __posix
// POSIX
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#ifdef V_PC
#include "stdafx.h"
#include <ctgmath>
#endif
#ifdef V_MAC
#include <math.h>
#endif

#ifdef V_CPP
#include <lua.hpp>
#else
#include <lua.h>
#endif
#include <lauxlib.h>
#include <lualib.h>
#include <bass.h>
#include <bassmidi.h>
#include <bassmix.h>
#include <luabass.h>

#ifdef V_PC
#include <bassasio.h>
#include <mmsystem.h>
#include <assert.h>
#endif
#ifdef V_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>
#include <CoreMIDI/MIDISetup.h>
#include <CoreMIDI/MIDIThruConnection.h>
#include <pthread.h>
#endif

#include "aeffect.h"
#include "aeffectx.h"
#include "vstfxstore.h"

#include "global.h"
#include "basslua.h"

#ifdef V_PC
// disable warning for mismatch lngth between LUA int and C int
#pragma warning( disable : 4244 )
// disable warning for deprecated ( e.g. sprintf )
#pragma warning( disable : 4996 )
#endif


static char pparam[512] = "";
static char pfname[1024] = "";
static long pdatefname = 0;

#define MAXPITCH 128
#define MAXCHANNEL 16
#define MIDIIN_MAX 16

/**
* \union T_midimsg
* \brief store a short Midi Messages ( noteOn, ...)
*
* The data manipulated is a 3 bytes structure.
* It is used for short MIDI messages , like Note-On, Note-Off, Control ...
*/
typedef union t_midimsg
{
	DWORD dwData; /*!< The block of data. */
	BYTE bData[4]; /*!< The data, byte per byte. */
} T_midimsg;

/**
* \struct T_selector
* \brief Group of criteria to trigger an event
*
* It groups a set of notes in a single object, to trigger event in teh LUA script.
*/
#define SELECTORMAXPITCH 5
#define SELECTORMAX 50
typedef struct t_selector
{
	int luaNrAction;
	char op; /*!< operator between the pitch ( o for or , b for between ) */
	int nrDevice; /*!< on this device  */
	int nrChannel; /*!< on this channel  */
	int type_msg; /*!< for this type of msg */
	int pitch[SELECTORMAXPITCH]; /*!< pitches of the selector */
	int nbPitch; /*!< number of pitches */
	bool stopOnMatch;
	char param[128];
} T_selector;

char g_path_in_error_txt[MAXBUFCHAR] = "basslua_log_in.txt";
#define MAXBUFERROR 64
#define MAXLENBUFERROR 1024
#define BUFFER_SIZE 1000 // for wchar translation

#define timer_dt 50 // ms between two timer interrupts on LUA input

#define smidiToOpen "midiinOpen" // global LUA table which contains the MIDI-in to open

#define sinit "init" // luabass function called to init the bass midi out 
#define sonStart "onStart" // LUA function called just after the creation of the LUA stack g_LUAstate ( e.g. to initialise the MIDI settings)
#define sonStop "onStop" // LUA function called before to close the LUA stack g_LUAstate
#define sfree "free" // luabass function called to free the bass midi out 

// LUA-functions called by this DLL 
#define LUAFunctionNoteOn "onNoteOn" // LUA funtion to call when new midiin noteon
#define LUAFunctionNoteOff "onNoteOff" // LUA funtion to call when new midiin noteoff
#define LUAFunctionKeyPressure "onKeypressure" // LUA funtion to call when new midiin keypressure
#define LUAFunctionControl "onControl" // LUA funtion to call when new midiin control
#define LUAFunctionProgram "onProgram" // LUA funtion to call when new midiin program
#define LUAFunctionChannelPressure "onChannelPressure" // LUA funtion to call when new midiin channelpressure
#define LUAFunctionPitchBend "onPitchBend" // LUA funtion to call when new midiin pitchbend
#define LUAFunctionSystemCommon "onSystemeCommon" // LUA funtion to call when new midiin systemcommon
#define LUAFunctionSysex "onSysex" // LUA funtion to call when new midiin sysex
#define LUAFunctionActive "onActive" // LUA funtion to call when new midiin active sense
#define LUAFunctionClock "onClock" // LUA funtion to call when midiin clock

#define onTimer "onTimer" // LUA funtion to call when timer is triggered 
#define onSelector "onSelector" // LUA functions called with noteon noteoff event,a dn add info 





//////////////////////////////
//  static statefull variables
//////////////////////////////

static bool g_midiopened[MIDIIN_MAX]; // list of midiin status. true if midin is open.

static bool g_logMidiInEvent = false;
static char g_chMidiInEvent[256] = "";

static bool g_collectLog = false;
static int nrOutBufLog = 0;
static int nrInBufLog = 0;
#define MAXNBLOGOUT 64
static char bufLog[MAXNBLOGOUT][MAXBUFCHAR];


static bool g_statuspitch[MIDIIN_MAX][MAXCHANNEL][MAXPITCH]; // status of input  pitch

static lua_State *g_LUAstate = 0; // LUA state, loaded with the script which manage midiIn messages

static bool g_process_NoteOn, g_process_NoteOff;
static bool g_process_Control, g_process_Program;
static bool g_process_PitchBend, g_process_KeyPressure, g_process_ChannelPressure;
static bool g_process_Sysex, g_process_SystemCommon, g_process_Clock, g_process_Activesensing;
static bool g_process_Timer ;
static int g_countmidiin = 0;

T_selector g_selectors[SELECTORMAX];
int g_selectormax = 0;

static double g_current_t = 0.0; // time in s for the timer

static int actionMode = modeChord;

#ifdef V_PC
// timer to trigger LUA regularly
static HANDLE g_timer = NULL;
// mutex to protect the input ( from GUI, timer, and Mid-in , to LUA )
static HANDLE g_mutex_in = NULL;
#endif
#ifdef V_MAC
// timer to trigger LUA regularly
static CFRunLoopTimerRef g_timer = 0 ;
// mutex to protect the access of the midiin process
static pthread_mutex_t g_mutex_in;
#endif

voidcallback fcallback;
void nullf()
{

}
static int pitchbend_value(T_midimsg u)
{
	return((int)(u.bData[2]) * (int)(0x80) + (int)(u.bData[1]) - (int)(0x2000));
}
static int pitch_to_white_key(int pitchin, int pitch0, int *sharp)
{
	// return the white key index of pitchin, Relatively to pitch0
	// sharp is set to one if pitch0 is a black key

	int o0 = pitch0 / 12;
	int oi = pitchin / 12;

	int m0 = pitch0 % 12;
	int mi = pitchin % 12;

	int ri, r0;
	*sharp = 0;

	switch (m0)
	{
	case 0: r0 = 0; break;
	case 1: case 2: r0 = 1; break;
	case 3: case 4: r0 = 2; break;
	case 5: r0 = 3; break;
	case 6: case 7: r0 = 4; break;
	case 8: case 9: r0 = 5; break;
	case 10: case 11: r0 = 6; break;
	default: r0 = 0; break;
	}
	switch (mi)
	{
	case 0: ri = 0; break;
	case 1: ri = 0; *sharp = 1; break;
	case 2: ri = 1; break;
	case 3: ri = 1; *sharp = 1; break;
	case 4: ri = 2; break;
	case 5: ri = 3; break;
	case 6: ri = 3; *sharp = 1; break;
	case 7: ri = 4; break;
	case 8: ri = 4; *sharp = 1; break;
	case 9: ri = 5; break;
	case 10: ri = 5; *sharp = 1; break;
	case 11: ri = 6; break;
	default: ri = 0; break;
	}
	int r = ri - r0 + 7 * (oi - o0);
	return r;
}

/////////////////////////////////////////////////////
// FOR INPUT ( GUI, timer, Midin ===> LUA )
/////////////////////////////////////////////////////


static void log_init(const char *fname)
{
	if ((fname != NULL) && (strlen(fname) > 0))
	{
		strcpy(g_path_in_error_txt, fname);
		strcat(g_path_in_error_txt, "_in.txt");
	}
	FILE * pFile = fopen(g_path_in_error_txt, "w");;
	if (pFile == NULL) return;
	fprintf(pFile, "log luabass in\n");
	fclose(pFile);
}
int mlog(const char * format, ...)
{
	char msg[MAXBUFCHAR];
	va_list args;
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	FILE * pFile = fopen(g_path_in_error_txt, "a");;
	if (pFile != NULL)
	{
		fprintf(pFile, "%s", msg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	if (g_collectLog)
	{
		strcpy(bufLog[nrInBufLog], msg);
		nrInBufLog++;
		if (nrInBufLog >= MAXNBLOGOUT)
			nrInBufLog = 0;
	}
	return -1;
}
void lock_mutex_in()
{
#ifdef V_PC
	if (g_mutex_in)
		WaitForSingleObject(g_mutex_in, INFINITE);
#endif
#ifdef V_MAC
	pthread_mutex_lock(&g_mutex_in);
#endif

}
void unlock_mutex_in()
{
#ifdef V_PC
	if (g_mutex_in)
		ReleaseMutex(g_mutex_in);
#endif
#ifdef V_MAC
	pthread_mutex_unlock(&g_mutex_in);
#endif
}
int action_table(const char *module, const char *table, const int index, const char* field, char*svalue, int *ivalue, int action)
{
	// if index >= 0 : works on module.table[index+1].field
	// if index < 0 , works on module.table.field
	// action is made of :
	//		0x1<<0 : get field value in svalue or *ivalue
	//		0x1<<1 : set svalue|*ivalue in field
	//		0x1<<2 : field(index[,svalue][,*ivalue]) at this index
	//		0x1<<3 : field(index[,svalue][,*ivalue]) of the table
	//		0x1<<4 : set nil in field
	// return action done
	if (g_LUAstate == NULL)
		return 0;
	int retCode = 0;
	if (lua_getglobal(g_LUAstate, module) != LUA_TTABLE)
	{
		mlog("basslua_table : module %s is not available in LUA script", module);
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if (lua_getfield(g_LUAstate, -1, table) != LUA_TTABLE)
	{
		if (strcmp(table, tableInfo) != 0)
			mlog("basslua_table : table %s is not available in module %s ", table, module);
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if ((index >= 0) && (lua_geti(g_LUAstate, -1, index + 1) != LUA_TTABLE))
	{
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if ((action & tableSetKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// set values
		if (svalue)
		{
			lua_pushstring(g_LUAstate, svalue);
			lua_setfield(g_LUAstate, -2, field);
		}
		if (ivalue)
		{
			lua_pushinteger(g_LUAstate, *ivalue);
			lua_setfield(g_LUAstate, -2, field);
		}
		retCode |= tableSetKeyValue;
	}
	if ((action & tableGetKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// get values
		lua_getfield(g_LUAstate, -1, field);
		if (svalue)
			*svalue = '\0';
		if (lua_isstring(g_LUAstate, -1) && svalue)
		{
			strcpy(svalue, lua_tostring(g_LUAstate, -1));
			retCode |= tableGetKeyValue;
		}
		if (ivalue)
			*ivalue = 0;
		if (lua_isinteger(g_LUAstate, -1) && ivalue)
		{
			*ivalue = (int)lua_tointeger(g_LUAstate, -1);
			retCode |= tableGetKeyValue;
		}
		lua_pop(g_LUAstate, 1); // pop field
	}
	if ((action & tableNilKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// nil values
		lua_pushnil(g_LUAstate);
		lua_setfield(g_LUAstate, -2, field);
		retCode |= tableNilKeyValue;
	}
	if ((action & tableCallKeyFunction) && (field != NULL))
	{
		// call lua_function([svalue][,ivalue]) of this item
		if (lua_getfield(g_LUAstate, -1, field) == LUA_TFUNCTION)
		{
			int nbArg = 0;
			if (svalue)
			{
				lua_pushstring(g_LUAstate, svalue);
				nbArg++;
			}
			if (ivalue)
			{
				lua_pushinteger(g_LUAstate, *ivalue);
				nbArg++;
			}
			if (lua_pcall(g_LUAstate, nbArg, 0, 0) != LUA_OK)
			{
				mlog("basslua_table mlog calling item callfunction in %s %s : %s", module, table, lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
			else
			{
				retCode |= tableCallKeyFunction;
			}
		}
	}
	if ((action & tableCallTableFunction) && (field != NULL))
	{
		// call lua_function(index[,svalue][,ivalue]) of this table
		if (lua_getfield(g_LUAstate, -2, field) == LUA_TFUNCTION)
		{
			lua_pushinteger(g_LUAstate, index + 1);
			int nbArg = 1;
			if (svalue)
			{
				lua_pushstring(g_LUAstate, svalue);
				nbArg++;
			}
			if (ivalue)
			{
				lua_pushinteger(g_LUAstate, *ivalue);
				nbArg++;
			}
			if (lua_pcall(g_LUAstate, nbArg, 0, 0) != LUA_OK)
			{
				mlog("basslua_table : mlog calling table callfunction in %s %s : %s", module, table, lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
			else
			{
				retCode |= tableCallTableFunction;
			}
		}
	}
	lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
	return retCode;
}
bool basslua_call(const char *module, const char *function, const char *sig, ...)
{
	// call module.function with parameters as described in first parameter, with syntax [arguments]*>[results]*:
	// d : double
	// i : integer
	// b : boolean
	// s : char*
	// > : end of descriptor of input argument, followed by descriptor of results
	// example : 
	//    basslua_call("_G","func","dd>dd",x,y,&w,&z)
	//       call the LUA funtion in module global
	//       dd>dd : the function has two integers as input ( x , y ) , and returns two integers ( &w, &z )
	lock_mutex_in();
	bool retCode = true;
	if (g_LUAstate == NULL)
		retCode = false;
	else
	{
		if (lua_getglobal(g_LUAstate, module) == LUA_TTABLE)
		{
			if (lua_getfield(g_LUAstate, -1, function) == LUA_TFUNCTION)
			{
				va_list vl;
				int narg, nres; // number of arguments ans results
				va_start(vl, sig);
				for (narg = 0; *sig; narg++) // for each argument
				{
					luaL_checkstack(g_LUAstate, 1, "lua_call : too many arguments");
					switch (*sig++)
					{
					case 'd': // double argument
						lua_pushnumber(g_LUAstate, va_arg(vl, double));
						break;
					case 'i':
						lua_pushinteger(g_LUAstate, va_arg(vl, int));
						break;
					case 'b':
						lua_pushboolean(g_LUAstate, (bool)va_arg(vl, int));
						break;
					case 's':
					{
								char buf[5000];
								strcpy(buf, va_arg(vl, char*));
								lua_pushstring(g_LUAstate, buf);
								break;
					}
					case '>':
						goto endargs;
					default:
						mlog("basslua_call : Invalid ergument descriptor , calling %s", function);
					}
				}
			endargs:
				nres = (int)strlen(sig);
				if (lua_pcall(g_LUAstate, narg, nres, 0) != LUA_OK)
				{
					mlog("basslua_call : mlog calling LUA function %s :err=%s", function, lua_tostring(g_LUAstate, -1));
					lua_pop(g_LUAstate, 1);
				}
				else
				{
					int nrReturnedParam = 1;
					nres = -nres;
					while (*sig) // repeat for each result
					{
						switch (*sig++)
						{
						case 'd':
						{
									int isnum;
									double n = lua_tonumberx(g_LUAstate, nres, &isnum);
									if (!isnum)
										mlog("basslua_call : result#%d should be number in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									*va_arg(vl, double *) = n;
									break;
						}
						case 'i':
						{
									int isnum;
									int n = (int)lua_tointegerx(g_LUAstate, nres, &isnum);
									if (!isnum)
										mlog("basslua_call : result#%d should be integer in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									*va_arg(vl, int *) = n;
									break;
						}
						case 'b':
						{
									if (lua_isboolean(g_LUAstate, nres))
									{
										bool n = (lua_toboolean(g_LUAstate, nres)) ? true : false;
										*va_arg(vl, bool *) = n;
									}
									else
										mlog("basslua_call : result#%d should be boolean in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									break;
						}
						case 's':
						{
									const char* s = lua_tostring(g_LUAstate, nres);
									if (s == NULL)
										mlog("basslua_call : result#%d should be string in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									else
										strcpy(va_arg(vl, char *), s);
									break;
						}
						default:
							mlog("basslua_call : result#%d not recognized in %s", nrReturnedParam, function);
						}
						nres++;
						nrReturnedParam++;
					}
				}
				va_end(vl);
			}
			else
			{
				mlog("basslua_call : function %s is not available in module %s ", function, module);
				retCode = false;
				lua_pop(g_LUAstate, 1); // popup non function
			}
		}
		else
		{
			mlog("module %s is not available in LUA script", module);
			retCode = false;
		}
		lua_pop(g_LUAstate, 1); // pop the module
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
	}
	unlock_mutex_in();
	return retCode;
}
static void runAction(int nrAction,double time, int nr_selector, int nrChannel, int d1, int d2, const char *param, int index, int mediane, int whiteIndex, int whiteMediane, int sharp)
{
	//mlog("runaction #%d", nrAction);
	bool fok = false;
	if (lua_getglobal(g_LUAstate, tableActions) == LUA_TTABLE)
	{
		if (lua_geti(g_LUAstate, 1, nrAction + 1) == LUA_TTABLE)
		{
			bool functionOK = false;
			if (lua_getfield(g_LUAstate, 2, fieldCallFunction) == LUA_TFUNCTION)
			{
				functionOK = true;
				// mlog("callFunction ok");
			}
			else
			{
				lua_pop(g_LUAstate, 1); // pop the bad value
				// try with the Function<mode>
				if (actionMode == modeScore)
				{
					functionOK = (lua_getfield(g_LUAstate, 2, fieldCallScore) == LUA_TFUNCTION);
					// mlog("callScore %s", functionOK?"ok":"ko");

				}
				if (actionMode == modeChord)
				{
					functionOK = (lua_getfield(g_LUAstate, 2, fieldCallChord) == LUA_TFUNCTION);
					// mlog("callChord %s", functionOK ? "ok" : "ko");
				}
			}
			if ( functionOK )
			{
				lua_pushnumber(g_LUAstate, time);
				int bid = nr_selector * 127 * 17 + (nrChannel+1) * 17 + d1;
				lua_pushinteger(g_LUAstate, bid);
				lua_pushinteger(g_LUAstate, nrChannel + 1);
				lua_pushinteger(g_LUAstate, d1);
				lua_pushinteger(g_LUAstate, d2);
				if (param)
					lua_pushstring(g_LUAstate, param);
				else
					lua_pushstring(g_LUAstate, "");
				lua_pushinteger(g_LUAstate, index);
				lua_pushinteger(g_LUAstate, mediane);
				lua_pushinteger(g_LUAstate, whiteIndex);
				lua_pushinteger(g_LUAstate, whiteMediane);
				lua_pushinteger(g_LUAstate, sharp);
				if (lua_pcall(g_LUAstate, 11, 0, 0) == LUA_OK) // call and pop function & parameters
				{
					fok = true ;
					fcallback();
				}
				else
				{
					mlog("erreur call LUA %s, action= %d ", lua_tostring(g_LUAstate, -1), nrAction);
					lua_pop(g_LUAstate, 1);
				}
			}
			else
			{
				lua_pop(g_LUAstate, 1); // pop fcallfunction
				mlog("erreur : no LUA function for action= %d ",  nrAction);
			}
		}
		lua_pop(g_LUAstate, 1); // pop luaAction table
	}
	lua_pop(g_LUAstate, 1); // pop table actions
}
void selectorSearch(double time, int nrDevice, int nrChannel, int type_msg, int d1, int d2)
{
	// search a note/program/control ( in midimsg u ) , within the selectors
	// the selectors are created with the LUA funtion selector()
	// if this note/program/control matches a selector, it calls the LUA functions
	//     midibetween() : for between selector
	//			parameters :
	//				sid : unique id of the selector ( specified in the creation )
	//				double time
	//              bid : unique id of the note which trigger this selector
	//              channel : channel of the note
	//				pitch : pitch of the note
	//              velocity : velocity of the note ( 0 for noteff )
	//              index : index of the note within the selector ( 1=first , 2=second, ... )
	//              mediane : index of the note from the middle of the selector ( 0 = middle )
	//              white-key index : index of the white_key within the selector
	//              white-key mediane : index of the white_key from the middle of the selector
	//              sharp : 1 for black key , else 0 
	//     midior() : for or selector
	//			parameters :
	//              bid : unique id of the note which trigger ths selector
	//              channel : channel of the note
	//				pitch : pitch of the note
	//              velocity : velocity of the note ( 0 for noteff )
	//              index : index of the note within the selector ( 1=first , 2=second, ... )
	//              0
	//              0
	//              0
	//              0


	// mlog("selectorSearch %d", d1);
	for (int nr_selector = 0; nr_selector < g_selectormax; nr_selector++)
	{
		T_selector *s = &(g_selectors[nr_selector]);
		if (((s->type_msg == type_msg) || (s->type_msg == NOTEONOFF && ((type_msg == NOTEON) || (type_msg == NOTEOFF))))
			&& ((s->nrDevice == -1) || (s->nrDevice == nrDevice))
			&& ((s->nrChannel == -1) || (s->nrChannel == nrChannel))
			)
		{
			int sharp;
			switch (s->op)
			{
			case 'b': //between
				if ((s->nbPitch == 2) &&
					(d1 >= s->pitch[0]) && (d1 <= s->pitch[1]))
				{
					int v = pitch_to_white_key(d1, s->pitch[0], &sharp);
					int w = pitch_to_white_key(d1, (s->pitch[1] + s->pitch[0]) / 2, &sharp);
					runAction(s->luaNrAction, time, nr_selector, nrChannel, d1, d2, s->param,
						d1 - s->pitch[0] + 1, d1 - (s->pitch[1] + s->pitch[0]) / 2 + 1,
						v, w, sharp);
					if (s->stopOnMatch)
						return;
				}
				break;
			case 'o': //or
				for (int m = 0; m < s->nbPitch; m++)
				{
					if (d1 == s->pitch[m])
					{
						runAction(s->luaNrAction, time, nr_selector, nrChannel, d1, d2, s->param,
							m + 1, 0,
							0, 0, 0);
						if (s->stopOnMatch)
							return;
					}
				}
				break;
			default:
				break;
			}
		}
	}
}
static void initSelector()
{
	for (int i = 0; i < SELECTORMAX; i++)
	{
		T_selector *t = &(g_selectors[i]);
		t->luaNrAction = 0;
		t->op = 'b';
		t->nrDevice = -1;
		t->nrChannel = -1;
		t->type_msg = NOTEONOFF;
		for (int j = 1; j < SELECTORMAXPITCH; j++)
			t->pitch[j] = 0;
		t->nbPitch = 0;
		t->stopOnMatch = false;
		t->param[0] = '\0';
	}
	g_selectormax = 0;
}
static void midi_init()
{
	lua_pushnil(g_LUAstate);
	lua_setglobal(g_LUAstate, smidiToOpen);
}
static void pitch_init()
{
	for (int n = 0; n < MIDIIN_MAX; n++)
	{
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			for (int p = 0; p < MAXPITCH; p++)
				g_statuspitch[n][c][p] = false;
		}
	}
}
void midiprocess_msg(int midinr, double time, void *buffer, DWORD length)
{
	// process this new midiin mg with the midiin-LUA-thread , using the expected LUA-function midixxx()
	// Construct the short MIDI message.	
	T_midimsg u;
	u.dwData = 0;
	switch (length)
	{
	case 0: return;
	case 1: u.bData[0] = ((BYTE*)buffer)[0];  break;
	case 2: u.bData[0] = ((BYTE*)buffer)[0];  u.bData[1] = ((BYTE*)buffer)[1]; break;
	default: u.bData[0] = ((BYTE*)buffer)[0];  u.bData[1] = ((BYTE*)buffer)[1];  u.bData[2] = ((BYTE*)buffer)[2]; break;
	}

	g_current_t = time;

	// mlog("receive length=%d", length);

	switch (u.bData[0])
	{
	case SYSEX:
		// sysex
		if (! g_process_Sysex)	return; // g_process_ sysex
		lua_getglobal(g_LUAstate, LUAFunctionSysex);
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		{
			char *sysexAscii = (char*)malloc(length * 6 + 1);
			sysexAscii[0] = '\0';
			char s[10];
			BYTE *ptbuf = (BYTE*)buffer;
			for (unsigned int n = 0; n < length; n++, ptbuf++)
			{
				sprintf(s, "%0.2X ", *ptbuf);
				strcat(sysexAscii, s);
			}
			if (g_collectLog)
			{
				char s[256];
				sprintf(s, "<midiin device#%d @%f ,sysex = %s\n", midinr + 1, time, sysexAscii);
				mlog(s);
			}
			lua_pushstring(g_LUAstate, sysexAscii);
			if (lua_pcall(g_LUAstate, 3, 0, 0) != LUA_OK)
			{
				mlog("erreur call  LUA %s , err: %s", LUAFunctionSysex, lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
			free(sysexAscii);
		}
		return;
	case ACTIVESENSING:
		if (! g_process_Activesensing) return; // g_process_ active sensing messages 
		lua_getglobal(g_LUAstate, LUAFunctionActive);
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		if ( lua_pcall(g_LUAstate, 2, 0, 0) != LUA_OK )
		{
			mlog("erreur call  LUA %s , err: %s", LUAFunctionActive, lua_tostring(g_LUAstate, -1));
			lua_pop(g_LUAstate, 1);
		}
		return;
	case CLOCK:
		if (! g_process_Clock) return; // g_process_ clock messages 
		lua_getglobal(g_LUAstate, LUAFunctionClock);
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		if ( lua_pcall(g_LUAstate, 2, 0, 0)!= LUA_OK )
		{
			mlog("erreur call  LUA %s , err: %s", LUAFunctionClock, lua_tostring(g_LUAstate, -1));
			lua_pop(g_LUAstate, 1);
		}
		return;
	default: break;
	}
	Byte type_msg = u.bData[0] >> 4;
	Byte channel = u.bData[0] & 0x0F;
	if ((type_msg == NOTEON) && (u.bData[2] == 0))
	{
		type_msg = NOTEOFF;
		u.bData[0] = (NOTEOFF << 4) + channel;
	}

	if (g_collectLog)
	{
		if ((type_msg == PROGRAM) || (type_msg == CONTROL) || (type_msg == NOTEON) || (type_msg == NOTEOFF))
		{
			char s[256];
			sprintf(s, "<midiin device#%d channel#%d %0.2X/%0.2d/%0.2d @%f", midinr + 1, channel +1, u.bData[0], u.bData[1], u.bData[2], time);
			mlog(s);
		}
	}

	if (g_logMidiInEvent)
	{
		char chTypeMsg[64] = "";
		switch (type_msg)
		{
		case PROGRAM:
			strcpy(chTypeMsg, sprogram);
			break;
		case NOTEON:
			strcpy(chTypeMsg, snoteononly);
			break;
		case NOTEOFF:
			strcpy(chTypeMsg, snoteonoff);
			break;
		case CONTROL:
			strcpy(chTypeMsg, scontrol);
			break;
		default:
			break;
		}
		if (*chTypeMsg != '\0')
		{
			sprintf(g_chMidiInEvent, "%12s device=%2d channel=%2d d1=%3d d2=%3d", chTypeMsg, midinr + 1, channel + 1, u.bData[1], u.bData[2]);
		}
	}
	switch (type_msg)
	{
	case CHANNELPRESSURE : 
		if (! g_process_ChannelPressure) return;  
		lua_getglobal(g_LUAstate, LUAFunctionChannelPressure);
		break;
	case KEYPRESSURE: 
		if (! g_process_KeyPressure) return;  
		lua_getglobal(g_LUAstate, LUAFunctionKeyPressure);
		break;
	case SYSTEMCOMMON: 
		if (! g_process_SystemCommon) return;
		lua_getglobal(g_LUAstate, LUAFunctionSystemCommon);
		break;
	case CONTROL:
		selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (!g_process_Control) return;
		lua_getglobal(g_LUAstate, LUAFunctionControl);
		break;
	case PROGRAM:
		selectorSearch(time, midinr, channel, type_msg, u.bData[1], 0);
		if (!g_process_Program) return;
		lua_getglobal(g_LUAstate, LUAFunctionProgram);
		break;
	case NOTEOFF :
		if (g_statuspitch[midinr][channel][u.bData[1]] == false)
		{
			mlog("double note-off %d", u.bData[1]);
			return; 
		}
		g_statuspitch[midinr][channel][u.bData[1]] = false;
		selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (!g_process_NoteOff) return;
		lua_getglobal(g_LUAstate, LUAFunctionNoteOff);
		break;
	case NOTEON:
		if (g_statuspitch[midinr][channel][u.bData[1]] == true)
		{
			mlog("double note-on %d", u.bData[1]);
			return; 
		}
		g_statuspitch[midinr][channel][u.bData[1]] = true;
		selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (!g_process_NoteOn) return;
		lua_getglobal(g_LUAstate, LUAFunctionNoteOn);
		break; // return note_off for note-on with velocity==0
	case PITCHBEND : 
		if (! g_process_PitchBend) return; 
		lua_getglobal(g_LUAstate, LUAFunctionPitchBend);
		break;
	default: 
		mlog("unexpected MIDI msg %d", type_msg);
		return;
		break;
	}

	lua_pushinteger(g_LUAstate, midinr + 1);
	lua_pushnumber(g_LUAstate, time);
	int nbParam = 0;
	switch (type_msg)
	{
	case SYSTEMCOMMON:
		lua_pushinteger(g_LUAstate, u.bData[0]); 
		lua_pushinteger(g_LUAstate, u.bData[1]); 
		lua_pushinteger(g_LUAstate, u.bData[2]); 
		nbParam = 5;
		break;
	case CHANNELPRESSURE:
	case PROGRAM:
		lua_pushinteger(g_LUAstate, channel + 1); // channel
		lua_pushinteger(g_LUAstate, u.bData[1]); // program#
		nbParam = 4;
		break;
	case PITCHBEND:
		lua_pushinteger(g_LUAstate, channel + 1); // channel
		lua_pushinteger(g_LUAstate, pitchbend_value(u)); // pitchend value
		nbParam = 4;
		break;
	case NOTEON:
		lua_pushinteger(g_LUAstate, channel + 1); // channel
		lua_pushinteger(g_LUAstate, u.bData[1]); // pitch#
		lua_pushinteger(g_LUAstate, u.bData[2]); // velocity for note, value for control , or msb pitchbend
		nbParam = 5;
		break;
	default:
		lua_pushinteger(g_LUAstate, channel + 1); // channel
		lua_pushinteger(g_LUAstate, u.bData[1]); // control#
		lua_pushinteger(g_LUAstate, u.bData[2]); // value
		nbParam = 5;
		break;
	}
	if (lua_pcall(g_LUAstate, nbParam, 0, 0) != LUA_OK)
	{
		mlog("erreur calling LUA on_midi_msg, err: %s", lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
	}
}
void CALLBACK midinewmsg(DWORD device, double time, void *buffer, DWORD length, void *ptuser)
{
	lock_mutex_in();
    VstIntPtr intptr = (VstIntPtr)ptuser ;
    int user = (int)intptr;
	midiprocess_msg((int)user, time, buffer, length);
	unlock_mutex_in();
}
#ifdef V_MAC
CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal)
{
    // Obtain the name of an endpoint without regard for whether it has connections.
    // The result should be released by the caller.
    CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
    CFStringRef str;
    
    // begin with the endpoint's name
    str = NULL;
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
    if (str != NULL)
    {
        CFStringAppend(result, str);
        CFRelease(str);
    }
    
    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity(endpoint, &entity);
    if (entity == 0)
        // probably virtual
        return result;
    
    if (CFStringGetLength(result) == 0) {
        // endpoint name has zero length -- try the entity
        str = NULL;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
        if (str != NULL) {
            CFStringAppend(result, str);
            CFRelease(str);
        }
    }
    // now consider the device's name
    MIDIDeviceRef device = 0;
    MIDIEntityGetDevice(entity, &device);
    if (device == 0)
        return result;
    
    str = NULL;
    MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
    if (str != NULL) {
        // if an external device has only one entity, throw away
        // the endpoint name and just use the device name
        if (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2) {
            CFRelease(result);
            return str;
        } else {
            // does the entity name already start with the device name?
            // (some drivers do this though they shouldn't)
            // if so, do not prepend
            if (CFStringCompareWithOptions(str /* device name */,
                                           result /* endpoint name */,
                                           CFRangeMake(0, CFStringGetLength(str)), 0) != kCFCompareEqualTo) {
                // prepend the device name to the entity name
                if (CFStringGetLength(result) > 0)
                    CFStringInsert(result, 0, CFSTR(" "));
                CFStringInsert(result, 0, str);
            }
            CFRelease(str);
        }
    }
    return result;
}
void notifyProc(const MIDINotification *message, void *refCon)
{
    // event on MAC hardware config
    // ?
}
#endif
#ifdef V_MAC
void completionSysex(MIDISysexSendRequest *request)
{
    free((BYTE*)(request->data));
    free(request);
}
#endif
static void midiclose_device(int n)
{
	if (g_midiopened[n] )
	{
		//out_errori("BASS_MIDI_InStop %d\n", n);
		BASS_MIDI_InStop(n);
		//out_errori("BASS_MIDI_InFree %d\n", n);
		BASS_MIDI_InFree(n);
		//out_errori("midiinclose end %d\n", n);
		g_midiopened[n] = false;
	}
}
static void midiclose_devices()
{
	for (int n = 0; n < MIDIIN_MAX; n++)
		midiclose_device(n);
}
static int midiopen_device(int nr_device)
{
	if (g_midiopened[nr_device])
		return nr_device; // already open
    VstIntPtr intptr = nr_device ;
    void *voidptr = (void*)intptr ;
	if (BASS_MIDI_InInit(nr_device, (MIDIINPROC *)midinewmsg, voidptr) == TRUE)
	{
		if (BASS_MIDI_InStart(nr_device) == TRUE)
		{
			g_midiopened[nr_device] = true;
			mlog("Information : midiIn open device#%d : OK", nr_device + 1);
		}
		else
		{
			mlog("mlog BASS_MIDI_InStart device#%d ; err=%d", nr_device + 1, BASS_ErrorGetCode());
			return -1;
		}
	}
	else
	{
		mlog("mlog BASS_MIDI_InInit device%d ; err=%d", nr_device + 1, BASS_ErrorGetCode());
		return -1;
	}
	return nr_device;
}
static void midiopen_devices()
{
	if (lua_getglobal(g_LUAstate, smidiToOpen) == LUA_TTABLE)
	{
		midiclose_devices();
		/* table is in the stack at index 1 */
		lua_pushnil(g_LUAstate);  /* first key */
		while (lua_next(g_LUAstate, 1) != 0) 
		{
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			if (lua_isinteger(g_LUAstate, -1))
			{
				int nr_device = (int)lua_tointeger(g_LUAstate, -1) - 1;
				midiopen_device(nr_device);
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(g_LUAstate, 1);
		}
	}
	lua_pop(g_LUAstate, 1);
	midi_init();
}
static void filter_set()
{
	// validate the LUA functions available to process the MIDI messages
	g_process_Sysex = (lua_getglobal(g_LUAstate, LUAFunctionSysex) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Sysex) mlog("Information : bassLUA function %s registered", LUAFunctionSysex);

	g_process_Activesensing = (lua_getglobal(g_LUAstate, LUAFunctionActive) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Activesensing) mlog("Information : bassLUA function %s registered", LUAFunctionActive);

	g_process_Clock = (lua_getglobal(g_LUAstate, LUAFunctionClock) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Clock) mlog("Information : bassLUA function %s registered", LUAFunctionClock);

	g_process_ChannelPressure = (lua_getglobal(g_LUAstate, LUAFunctionChannelPressure) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_ChannelPressure) mlog("Information : bassLUA function %s registered", LUAFunctionChannelPressure);

	g_process_KeyPressure = (lua_getglobal(g_LUAstate, LUAFunctionKeyPressure) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_KeyPressure) mlog("Information : bassLUA function %s registered", LUAFunctionKeyPressure);

	g_process_Control = (lua_getglobal(g_LUAstate, LUAFunctionControl) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Control) mlog("Information : bassLUA function %s registered", LUAFunctionControl);

	g_process_SystemCommon = (lua_getglobal(g_LUAstate, LUAFunctionSystemCommon) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_SystemCommon) mlog("Information : bassLUA function %s registered", LUAFunctionSystemCommon);

	g_process_Program = (lua_getglobal(g_LUAstate, LUAFunctionProgram) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Program) mlog("Information : bassLUA function %s registered", LUAFunctionProgram);

	g_process_NoteOff = (lua_getglobal(g_LUAstate, LUAFunctionNoteOff) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_NoteOff) mlog("Information : bassLUA function %s registered", LUAFunctionNoteOff);

	g_process_NoteOn = (lua_getglobal(g_LUAstate, LUAFunctionNoteOn) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_NoteOn) mlog("Information : bassLUA function %s registered", LUAFunctionNoteOn);

	g_process_PitchBend = (lua_getglobal(g_LUAstate, LUAFunctionPitchBend) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_PitchBend) mlog("Information : bassLUA function %s registered", LUAFunctionPitchBend);

	g_process_Timer = (lua_getglobal(g_LUAstate, onTimer) == LUA_TFUNCTION);
	lua_pop(g_LUAstate, 1);
	if (g_process_Timer) mlog("Information : bassLUA function %s registered", onTimer);

}
static void init_mutex()
{
    // create a mutex to manipulate safely the inputs ( timer , midiin, gui ) up to LUA
#ifdef V_PC
	g_mutex_in = CreateMutex(NULL, FALSE, NULL);
	if (g_mutex_in == NULL)
		mlog("CreateMutex : error init_mutex");
#endif
#ifdef V_MAC
	if (pthread_mutex_init(&g_mutex_in, NULL) != 0)
		mlog("pthread_mutex_init : error init_mutex");
#endif
}
static void free_mutex()
{
#ifdef V_PC
	CloseHandle(g_mutex_in);
#endif
#ifdef V_MAC
	pthread_mutex_destroy(&g_mutex_in);
#endif
}
static void process_timer()
{
	if (g_process_Timer)
	{
		lua_getglobal(g_LUAstate, onTimer);
		lua_pushnumber(g_LUAstate, g_current_t);
		g_current_t = g_current_t + (float)(timer_dt) / 1000.0;
		if (lua_pcall(g_LUAstate, 1, 0, 0) != LUA_OK)
		{
			mlog("erreur calling LUA %s, err: %s", onTimer, lua_tostring(g_LUAstate, -1));
			lua_pop(g_LUAstate, 1);
		}
	}
	if (g_countmidiin > 10)
	{
		midiopen_devices(); // check if there is any new midiin device to open
		g_countmidiin = 0;
	}
	g_countmidiin++;
}
#ifdef V_PC
VOID CALLBACK timer_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	lock_mutex_in();
	process_timer();
	unlock_mutex_in();
}
#endif
#ifdef V_MAC
// void (*CFRunLoopTimerCallBack) ( CFRunLoopTimerRef timer, void *info );
void timer_callback(CFRunLoopTimerRef timer, void *info)
{
    lock_mutex_in();
    process_timer();
    unlock_mutex_in();
}
#endif
static void timer_init()
{
	// create a periodic timer to flush-out the queud midiout msg
#ifdef V_PC
	if (!CreateTimerQueueTimer(&g_timer, NULL, timer_callback, 0, timer_dt, timer_dt, 0))
		mlog("mlog creating timer");
#endif
#ifdef V_MAC
	g_timer = CFRunLoopTimerCreate(NULL, (float)(timer_dt) / 1000.0, (float)(timer_dt) / 1000.0, 0, 0, timer_callback, NULL);
	if (g_timer == 0)
		mlog("mlog creating timer");
#endif
}
static void free_timer()
{
#ifdef V_PC
    DeleteTimerQueueTimer(NULL, g_timer, NULL);
#endif
}
static void init()
{
	pitch_init();
	initSelector();
	srand((unsigned)time(NULL));
	filter_set();
	init_mutex();
	timer_init();
}
static void free()
{
	free_timer();
	free_mutex();
	midiclose_devices();
}

bool basslua_getMidiinEvent(char *buf)
{
	lock_mutex_in();
	bool retCode = false;
	if (buf == NULL)
	{
		g_logMidiInEvent = false;
	}
	else
	{
		*buf = '\0';
		g_logMidiInEvent = true;
		if (strlen(g_chMidiInEvent) > 0)
		{
			strcpy(buf, g_chMidiInEvent);
			g_chMidiInEvent[0] = '\0'; 
			retCode = true;
		}
	}
	unlock_mutex_in();
	return (retCode);
}
void basslua_setSelector(int i, int luaNrAction, char op, int nrDevice, int nrChannel, const char *type_msg, const int *pitch, int nbPitch, bool stopOnMatch , const char* param)
{
	lock_mutex_in();
	if (luaNrAction < 0)
	{
		g_selectormax = i;
	}
	else
	{
		T_selector *t = &(g_selectors[i]);
		t->luaNrAction = luaNrAction;
		t->op = op;
		t->nrDevice = nrDevice;
		t->nrChannel = nrChannel;
		if (strcmp(type_msg, snoteonoff) == 0)
			t->type_msg = NOTEONOFF;
		if (strcmp(type_msg, snoteononly) == 0)
			t->type_msg = NOTEON;
		if (strcmp(type_msg, scontrol) == 0)
			t->type_msg = CONTROL;
		if (strcmp(type_msg, sprogram) == 0)
			t->type_msg = PROGRAM;
		for (int j = 0; j < nbPitch; j++)
			t->pitch[j] = pitch[j];
		t->nbPitch = nbPitch;
		t->stopOnMatch = stopOnMatch;
		strcpy(t->param, param);
		if (i >= g_selectormax)
			g_selectormax = i + 1;
	}
	unlock_mutex_in();
}
void basslua_setMode(int mode)
{
	lock_mutex_in();
	actionMode =  mode ;
	unlock_mutex_in();
}
void basslua_selectorSearch(int nrDevice, int nrChannel, int type_msg, int p, int v)
{
	lock_mutex_in();
	selectorSearch(g_current_t, nrDevice, nrChannel, type_msg, p, v);
	unlock_mutex_in();
}
int basslua_table(const char *module, const char *table, const int index, const char* field, char*svalue, int *ivalue, int action)
{
	lock_mutex_in();
	int retCode = action_table(module, table, index, field, svalue, ivalue, action);
	unlock_mutex_in();
	return (retCode);

}

bool basslua_getLog(char *buf)
{
	lock_mutex_in();
	bool retCode = false;
	if (buf == NULL)
	{
		char bufnull[256];
		bool r;
		basslua_call(moduleLuabass, soutGetLog, "i>bs", 0,&r,bufnull);
		g_collectLog = false;
	}
	else
	{
		g_collectLog = true;
		if (nrOutBufLog != nrInBufLog)
		{
			strcpy(buf, bufLog[nrOutBufLog]);
			retCode = true;
			nrOutBufLog++;
			if (nrOutBufLog >= MAXNBLOGOUT)
				nrOutBufLog = 0;
		}
		else
		{
			basslua_call(moduleLuabass, soutGetLog, "i>bs", 1, &retCode, buf);
		}
	}
	unlock_mutex_in();
	return (retCode);
}
bool basslua_openMidiIn(int *nrDevices, int nbDevices)
{
	lock_mutex_in();
	bool retCode = true;
	midiclose_devices();
	for (int n = 0; n < nbDevices; n++)
	{
		if (midiopen_device(nrDevices[n]) == false)
			retCode = false;
	}
	unlock_mutex_in();
	return (retCode);
}

/**
* \fn void basslua_open()
* \brief Open the dedicated midiin-LUA-thread to process midiin msg.
* This function must be called at the beginning, by any external C Module.
* It starts an LUA thread, and call the LUA function onStart(param).
* \param fname : LUA script file to launch. This LUA script must have some predefined functions
* \param param : parameter given to the LUA function onStart
* \return -1 if mlog. 0 if no mlog.
**/
bool basslua_open(const char* fname, const char* param, bool reset, long datefname, voidcallback ifcallback , const char *logpath)
{
	if (( reset == false ) && (strcmp(param, pparam) == 0) && (strcmp(fname, pfname) == 0)&& (datefname == pdatefname))
		return true ;
	strcpy(pparam, param);
	strcpy(pfname, fname);
	pdatefname = datefname;

	if (ifcallback == NULL)
		fcallback = nullf;
	else
		fcallback = ifcallback;

	basslua_close();

	log_init(logpath);

	// mutex are not yet available
	// open the dedicated midiin-LUA-thread to process midiin msg
	g_LUAstate = luaL_newstate(); // newthread 
	luaL_openlibs(g_LUAstate);
	
	if (luaL_loadfile(g_LUAstate, fname) != LUA_OK)
	{
		mlog("basslua_open mlog lua_loadfile <%s>", lua_tostring(g_LUAstate, -1));
		return(false);
	}
	
	// require the "luabass" module for Midi-out
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleLuabass);
	if ( lua_pcall(g_LUAstate, 1, 1,0) != LUA_OK )
	{
		mlog("basslua_open mlog require %s <%s>", moduleLuabass, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return false;
	}
	if (! lua_istable(g_LUAstate, -1))
	{
		mlog("basslua_open mlog require %s : not a table", moduleLuabass);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleLuabass);
	
	// require the "chord" module for chord interpretation
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleChord);
	if (lua_pcall(g_LUAstate, 1, 1, 0) != LUA_OK)
	{
		mlog("basslua_open mlog require %s <%s>", moduleChord, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return  false;
	}
	if (!lua_istable(g_LUAstate, -1))
	{
		mlog("basslua_open mlog require %s : not a table", moduleChord);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleChord);

	// require the "score" module for score interpretation
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleScore);
	if (lua_pcall(g_LUAstate, 1, 1, 0) != LUA_OK)
	{
		mlog("basslua_open mlog require %s <%s>", moduleScore, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return  false;
	}
	if (!lua_istable(g_LUAstate, -1))
	{
		mlog("basslua_open mlog require %s : not a table", moduleScore);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleScore);

	// create the "info" table to receive instructions
	lua_newtable(g_LUAstate);
	lua_setglobal(g_LUAstate, tableInfo);

	// run the script
	if (lua_pcall(g_LUAstate, 0, 0, 0) != LUA_OK)
	{
		mlog("basslua_open mlog lua_pcall <%s>", lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return false;
	}
	
	// init the static variable of this dll
	init();

	// init the luabass module
	basslua_call(moduleLuabass, sinit, "s", (logpath == NULL) ? "" : logpath); // ask for the luabass.init out-process : init the module 

	// init the lua script
	basslua_call(moduleGlobal, sonStart, "s", param); // ask for the _G.onStart() : init user's settings

	mlog("information : basslua_open(%s) OK", fname);
	return (true);
}
/**
* \fn void basslua_close()
* \brief close the dedicated IN-LUA-thread which processes events from midiin, in-timer and gui.
* This function must be called at the end, by any external C Module.
* It call the LUA function onStop(), and stops the LUA thread,.
**/
void basslua_close()
{
	if (g_LUAstate)
	{
		lock_mutex_in(); // mutex should be available at this stage
		basslua_call(moduleLuabass, soutAllNoteOff, "s", "a");
		basslua_call(moduleGlobal, sonStop, "");
		basslua_call(moduleLuabass, sfree, "");
		lua_close(g_LUAstate);
	}
	free();
	g_LUAstate = 0;
	// unlock_mutex_in("basslua_open"); // mutex are not more available at this stage
}
