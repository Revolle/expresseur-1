// expresscmd.cpp 
// update : 31 / 10 / 2016 10 : 00
// console app.
//
// Simple usage of basslua ans luabass module.
//
// Load a LUA script file through the bassua. 
// This LUA script processes :
//     - MIDI inputs
//     - user's commands from this app
//     - generates MIDI out accordiing to its logic
//

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


#ifdef V_PC
#include "stdafx.h"
#endif

#ifdef V_MAC
#define strcpy_s strcpy
#define gets_s gets
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "basslua.h"
#include "luabass.h"

#define sUsage "type help for list of functions\n"

int main(int argc, char* argv[])
{
	
	// define the default lua-script and its empty parameter
	char fname[1024] = "lua\\expresscmd.lua";
	char param[1024] = "";

	// use arguments of the command-line to change the lua-scipt and its parameters
	if (argc > 1)
		strcpy_s(fname, argv[1]);
	if (argc > 2)
		strcpy_s(param, argv[2]);

	// starts the basslua module with the lua-scipt
	// this command loads :
	//    - the basslua ( with midi-in management )
	//    - the lua-script ( with its music-logic according to midi-inputs).
	//        The lua-script loads these modules  by default :
	//          - luabass ( for midi-output )
	//          - luascore ( to play a score )
	//          - luachord ( to play chords )
	//        The lua-scriptstarts the lua-function onStart(parameters) :
	bool retCode = basslua_open(fname, param, true, 0, NULL ,"expresscmd.log");
	printf("run bass_lua_file=<%s> param=<%s> :  %s\n", fname, param, retCode?"OK":"Error");
	
	// print the usage of this command-line tool
	printf(sUsage);

	// read the inpu
	char ch[1204];
	while (gets_s(ch))
	{
		// read user's command, and forward it to the LUA script through the basslua module
		if ((strcmp(ch, "exit") == 0) || (strcmp(ch, "quit") == 0) || (strcmp(ch, "close") == 0) || (strcmp(ch, "end") == 0))
			break;
		if ((strcmp(ch, "?") == 0) || (strcmp(ch, "help") == 0))
		{
			printf(sUsage);
			strcpy(ch, "help");
		}
		char *pt[20];
		int nb;
		pt[0] = strtok(ch, " ");
		if (pt[0] != NULL)
		{
			nb = 1;
			while ((pt[nb] = strtok(NULL, " ")) != NULL) nb ++;
			char *f[2];
			f[0] = strtok(pt[0], ".");
			f[1] = strtok(NULL, ".");
			char module[256];
			char function[256];
			if (f[1] == NULL)
			{
				strcpy(module, "_G");
				strcpy(function, pt[0]);
			}
			else
			{
				strcpy(module, f[0]);
				strcpy(function, f[1]);
			}
			bool ret_code = false;
			switch (nb)
			{
			case 1: ret_code = basslua_call(module, function, ""); break;
			case 2: ret_code = basslua_call(module, function, "s", pt[1]); break;
			case 3: ret_code = basslua_call(module, function, "ss", pt[1], pt[2]); break;
			case 4: ret_code = basslua_call(module, function, "sss", pt[1], pt[2], pt[3]); break;
			case 5: ret_code = basslua_call(module, function, "ssss", pt[1], pt[2], pt[3], pt[4]); break;
			default: printf("too many arguments\n");
			}
			if (ret_code )
				printf(">Done\n");
			else
				printf(">Error\n");
		}
	}

	printf("close basslua\n");
	basslua_close();

	return 0;
}

