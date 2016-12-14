--[[
Created: 10/02/2015
-- update : 23/11/2016 18:00 


This LUA script is started by basslua.

Basslua manages input ( GUI, MIDI-In, timer ). These inputs are sent to this LUA script.

Basslua loads in addition, by default, these modules, accessible as global :
- luabass : C-LUA-Module for midi output.
- luachord.lua : script-LUA-module to interpret text chords.
- luascore.lua : script-LUA-module to interpret score.

Function onStart(param) : called by basslua ,when this LUA script is started.
Function onStop() : called by basslua , before to close this LUA script.

basslua uses these tables :
- midiinOpen = { 1, 3 } : LUA table which contains midiIn deviceNrs to open. Checked regularly by basslua.
- info.value = "text to display in the gui" : LUA string, to be displayed in the GUI. Checked regularly by basslua.
- info.next = 1 : LUA value to increment-decrement the file in the GUI list. Checked regularly by basslua.
- values = { {},..} : table of values which can be tuned in the GUI. 
    Read by the GUI through basslua.
    Example :
    values = {
      -- callFunction is used by GUI, through basslua. e.g. to change MIDI parameters
      -- the GUI, through basslua, will add fields values[valueName]=value
      { name = "ctrl7 Bass" , value=60 , callFunction = ctrl7Bass , help="volume Midi track Bass"  } }

- tracks = { {},..} : table of tracks which can be tuned in the mixer of the GUI. 
    Read by the GUI through basslua.
    Example :
    tracks = { 
       -- callFunction is used by GUI, through basslua. E.G to set the trackNr in the midiout-processor
       callFunction = function (nameTrack, nrTrack) luabass.setVarMidiOut(nameTrack,nrtrack) end  ,
       -- the GUI, through basslua, will add fields tracks[trackName]=TrackNr
       { name = "chord-bass" , help = "track volume for the bass for the chords"  } }
- actions = { {},..} : table of actions which can be used in thh GUI. Example :
    Read by the GUI through basslua.
    Example :
     actions = { 
     -- <name> : displayed in the GUI
     -- <icone> : if file icone.bmp exists, action is displyed in the toolbar
     -- <help> : help displayed in the GUI
     -- <shortcut> : if it exists, action is displayed in the menu bar ( e.g. HOME, END, DOWN, LEFT, RIGHT, CTRL+LEFT , ALT+RIGHT, SHIFT+HOME... )
     -- callFunction is called when the selector is triggered ( by a midi event, or a keystroke )
     -- callFunction parameters are :
     --   time : float, timestamp of the event
     --   uidKey : integer, unique id of the event ( composed of the selector id , channel, and picth )
     --   channel[1..16] of the event
     --   pitch[0..127] : integer, pitch of the event ( or control nr )
     --   velocity[0..127] : integer, velocity of the event ( or control value )
     --   paramString : string , parameter set in the GUI for this selector
     --   indexKey[1..n] : integer, index of the picth within the range of the selector. 1 means the start.
     --   medianeKey[-n/2..0..n/2] : integer, index of the pitch within the the range of the selector. 0 means the middle.
     --   whiteIndex : integer,  idem indexKey, but taking in account only "white keys"
     --   whiteMediane : integer,  idem medianeKey, but taking in account only "white keys"
     --   black[0,1] : integer,  0 means white key. 1 means black key.
 
Functions on<event>(...) : LUA functions to take actions on midi events
  called by luabass.dll on midi or timer event.
  These functions return nothing
    onNoteOn(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer velocity 0..127). 
    onNoteOff(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer velocity 0..127)
    onKeypressure(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer value 0..127)
    onControl(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer controlNr 0..127, integer value 0..127)
    onProgram(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer programNr 0..127)
    onChannelPressure(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer value 0..127 )
    onPitchBend(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer value 0..127*127 )
    onSystemeCommon(integer deviceNr 1.. , float timestamp, integer value1 0..127, integer value2 0..127 , integer value3 0..127 )
    onSysex(integer deviceNr 1.. , float timestamp , string asciiHexSysex e.g. FE4F7C4A... )
    onActive(integer deviceNr 1.. , float timestamp )
    onClock(integer deviceNr 1.. , float timestamp )
    onTimer(float timestamp)

This LUA script can starts another LUA script, to manage midi-out events. cf. luabass.onMidiout(LUAfile)
The script should contains ones of these functions :
    onNoteOn(integer trackNr 1.. , integer pitch 0..127, integer velocity 0..127)
    onNoteOff(integer trackNr 1.. , integer pitch 0..127, integer velocity 0..127)
    onKeyPressure(integer trackNr 1.. , integer pitch 0..127, integer value 0..127)
    onControl(integer trackNr 1.. , integer controlNr 0..127, integer value 0..127)
    onProgram(integer trackNr 1.. , integer programNr 0..127)
    onChannelPressure(integer trackNr 1.. , integer value 0..127)
    onPitchBend(integer trackNr 1.. , integer LSB 0..127, integer MSB 0..127)
These functions must return a list of zero or many MIDI messages, to send on midi-out. One message is 4 values :
  integer trackNr 1.., 
  string typeMessage ( NoteOn, noteOff, keyPressure, Control, Program, channelPressure, pitchBend )
  integer value1 0..127 ( pitch for note, programNr, controlNr, LSB for pitchbend )
  integer value2 0.127 ( velcoity for note, 0 fro program, cnotrolValue, MSB for pitchben )

--]]

-- Validity of a Midi-Out device, for the GUI  ( used by the GUI )
function midiOutIsValid(midiout_name)
  -------================
  -- return false if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  local invalid = { "teensy", "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" , "key25" , "key49" }
  for inil,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end
-- Validity of a Midi-In device, for the GUI  ( used by the GUI )
function midiInIsValid(midiin_name)
  -------===============
  -- return false if the midiin is not valid for the GUI
  local s = string.lower(midiin_name)
  local valid = { "sd%-50 midi" }
  local invalid = { "iac" , "loop" , "sd%-50" }
  for inil,v in ipairs(valid) do
    if ( string.find(s,v ) ~= nil) then
      return true ;
    end
  end
  for inil,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end

function onControl(deviceNr, timestamp, channel, controlNr, value)
  -- forward Controls from MIDI-in to Track#1
  luabass.outControl(controlNr,value)
end

-- list of values, for the GUI ( throug basslua )
values = {
  -- callFunction is used by GUI, through basslua. E.G to change MIDI parameters
  -- basslua, will add fields values[valueName]=value
  { name = "chord delay" , defaultValue=10 , help="Chord improvisation : delay between notes, in ms" },
  { name = "chord decay" , defaultValue=40 , help="Chord improvisation : decay beween notes, 64 = no decay" },
  { name = "scale decay" , defaultValue=0 , help="Scale improvisation : decay beween notes of the chord, 64 = no decay" },
}

-- list of the tracks, for the GUI
tracks = { 
  -- callFunction is used by GUI, through basslua. E.G to set the trackNr in the midiout-processor
  callFunction = function (nameTrack, nrTrack) luabass.setVarMidiOut(nameTrack,nrtrack) end  ,
  -- the GUI, through basslua, will add fields tracks[trackName]=TrackNr
  { name = "chord-bass" , help = "bass for improvisation"  } , 
  { name = "chord-background" , help = "background chords for improvisation"  } , 
  { name = "chord-chord" , help = "chords for improvisation"  } ,
  { name = "chord-scale" , help = "scale for improvisation"  } ,
}

function allNoteOff( t, bid, ch, pitch, velo )
  if ( velo or 64 ) > 0 then luabass.outAllNoteOff("n") end
end
function mainVolume( t, bid, ch, pitch, velo )
  if ( velo or 64 ) > 0 then luabass.outSetVolume(velo) end
end
function nextFile( t, bid, ch, pitch, velo )
  if ( velo or 64 ) > 0 then info.next = 1 end
end
function previousFile( t, bid, ch, pitch, velo )
  if ( velo or 64 ) > 0 then info.next = -1 end
end
-- list of actions for the GUI ( throug basslua )
  -- <name> : displayed in the GUI
  -- <icone> : if file icone.bmp exists, action is displyed in the toolbar
  -- <help> : help displayed in the GUI
  -- <shortcut> : if it exists, action is displayed in the menu bar 
  --           ( e.g. HOME, END, DOWN, LEFT, RIGHT, CTRL+LEFT , ALT+RIGHT, SHIFT+HOME, SPACE... )
  -- callFunction is called when the selector is triggered ( by a midi event, or a keystroke )
  --    ( callScore and callChord will be preferred when the "mode" "Score" or "Chord" is set by the GUI throug basslua )
  -- callFunction parameters are :
  --   time : float, timestamp
  --   uidKey : integer, unique id of the event ( composed of the selector id ,channel , and picth )
  --   channel[1..16] : integer, channel of the event ( or control nr , or program nr )
  --   pitch[0..127] : integer, pitch of the event ( or control nr , or program nr )
  --   velocity[0..127] : integer, velocity of the event ( or control value )
  --   paramString : string , pamaeter set in the selector
  --   indexKey[1..n] : index of the picth within the range of the selector. 1 means the minimum of the range
  --   medianeKey[-n/2..0..n/2] : index of the pitch within the the range of the selector. 0 means the middle of the range
  --   whiteIndex : idem indexKey, but taking in account only "white keys"
  --   whiteMediane : idem medianeKey, but taking in account only "white keys"
  --   black[0,1] : 0 means white key. 1 means black key.
actions = { 
  {name="global/all note off", callFunction = allNoteOff ,help="all note off", shortcut = "BACK" , icone = "all_note_off" },
  {name="global/main volume", callFunction = mainVolume },
  {name="global/previous file",  help="go to previous file of the list", callFunction = previousFile  },
  {name="global/next file", help="go to next file of the list", 
    callFunction = nextFile , shortcut = "TAB", icone = "next_file" },
  {name="move/previous move", help="go to the position of the previous move",
    callScore = luascore.previousPos,callChord=luachord.previousPos,shortcut = "ALT+UP",icone = "previous_move" },
  {name="move/first part", help="go to beginning of the score",
    callScore = luascore.firstPart,callChord=luachord.firstPart,shortcut = "CTRL+UP",icone = "first_part" },
  {name="move/previous part", help="go to previous part of the score",
    callScore = luascore.previousPart,callChord=luachord.previousPart,shortcut="SHIFT+UP",icone="previous_part" },
  {name="move/previous measure",help="go to previous section/measure of the score",
    callScore = luascore.previousMeasure, callChord = luachord.previousSection, shortcut = "UP" , icone = "previous_section"},
  {name="move/previous chord",help="go to previous chord of the score",
    callScore=luascore.previousEvent,callChord=luachord.previousChord,shortcut="LEFT",icone="previous_chord"},
  {name="move/next chord",help="go to next chord of the score",
    callScore=luascore.nextEvent,callChord=luachord.nextChord,shortcut="RIGHT",icone="next_chord" },
  {name="move/next measure", help="go to next section/measure of the score",
    callScore = luascore.nextMeasure, callChord = luachord.nextSection , shortcut = "DOWN" , icone = "next_section" },
  {name="move/next part", help="go to next part of the score",
    callScore = luascore.nextPart,callChord=luachord.nextPart,shortcut = "SHIFT+DOWN", icone = "next_part" },
  {name="move/last part", help="go to last part of the score",
    callScore = luascore.lastPart, callChord = luachord.lastPart , shortcut = "CTRL+DOWN", icone = "last_part" },
  {name="move/repeat part",callScore = luascore.repeatPart, callChord = luachord.repeatPart },
  {name="move/first part smooth",callScore=luascore.firstPartSmooth,callChord=luachord.firstPartSmooth},
  {name="move/repeat part smooth",callScore=luascore.repeatPartSmooth,callChord=luachord.repeatPartSmooth},
  {name="move/next part smooth",callScore=luascore.nextPartSmooth,callChord=luachord.nextPartSmooth },
  {name="move/last part smooth",callScore=luascore.lastPartSmooth,callChord=luachord.lastPartSmooth},
  {name="score/play", callFunction = luascore.play} ,
  {name="chord/change chord on noteon", callFunction = luachord.changeChordOn },
  {name="chord/change chord on noteoff", callFunction = luachord.changeChordOff },
  {name="chord/change chord white/black", callFunction = luachord.alternateChord },
  {name="chord/play scale chord", callFunction = luachord.playScaleChord },
  {name="chord/play scale penta", callFunction = luachord.playScalePenta },
  {name="chord/play chord brush up", callFunction = luachord.playChordUp },
  {name="chord/play chord brush down", callFunction = luachord.playChordDown },
  {name="chord/play background", callFunction = luachord.playBackground },
  {name="chord/play bass", callFunction = luachord.playBass },
  {name="chord/play walking bass", callFunction = luachord.playWalkingBass },
  {name="chord/pedal scale", callFunction = luachord.pedalScale },
  {name="chord/pedal chord", callFunction = luachord.pedalChord },
  {name="chord/pedal background", callFunction = luachord.pedalBackground },
  {name="chord/pedal bass", callFunction = luachord.pedalBass },
  {name="chord/octave scale", callFunction = luachord.octaveScale },
  {name="chord/octave chord", callFunction = luachord.octaveChord },
  {name="chord/octave background", callFunction = luachord.octaveBackground },
  {name="chord/octave bass", callFunction = luachord.octaveBass },
  }



 

