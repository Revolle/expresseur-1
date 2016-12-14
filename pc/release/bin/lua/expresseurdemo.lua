--[[
Created: 10/02/2015
-- update : 22/11/2016 19:00


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

-- local statefull variable
local mode = ""

local transpose = 0 -- global transposition in semi-tone
local playBlack -- last kind of key played 
local idPlayChord = {} -- id of the chords waiting for a noteoff in playChord
local idWalkingBass = {} -- id of the walking bass waiting for a noteoff in playWalkinBass
local idBass -- id of the bass chord
local idBackground -- id of the background chord
local idBackgroundChord = 0 -- last chord known by playGround
local idArpeggio = {} -- id of the chords waiting for a noteoff in arpeggio
local idArpeggioChord = 0 -- last chord known by playArpeggio
local pitchArpeggio = {} -- pitches of the arppeggio to play for the current chord
local pscale = {} -- pitch to switch off for the scale 
local onscale = {} -- pitch on for the scale 
local oldscale = {} -- pitch on for the scale 
local alteration -- alteration for a black key +/-1

-- Validity of a Midi-Out device, for the GUI  ( used by the GUI )
function midiOutIsValid(midiout_name)
  -------================
  -- return false if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  local invalid = { "teensy", "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" }
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
  luabass.outControl(controlNr,value)
end

-- list of values, for the GUI ( throug basslua )
values = {
  -- callFunction is used by GUI, through basslua. E.G to change MIDI parameters
  -- basslua, will add fields values[valueName]=value
  { name = "chord delay" , defaultValue=10 , help="Chord improvisation : delay beween notes, in ms" },
  { name = "chord decay" , defaultValue=40 , help="Chord improvisation : decay beween notes, 64 = no decay" },
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

--=========================================
-- function to drive the play of the chords
--=========================================
function scaleOff()
  for pitch = 53 , 78 , 1 do
    if pscale[pitch] then
      if onscale[pitch] == false  then
        luabass.outNoteOff(pscale[pitch],0,pitch,0,tracks["chord-scale"])
        pscale[pitch] = nil
      else
        oldscale[pitch] = true
      end
    end
  end
end

function onBass(velocity)
  if id bass then
    luabass.outChordOff(idBass)
  end
  luabass.outChordSet(idBass,12,0,64,1,1,table.unpack(luachord.getIndexPitches("bass", 1, 0)))
  luabass.outChordOn(idBass,(velocity or 64),0,tracks["chord-bass"])
end
function onBackground(velocity,ip,reverse)
  local p = ip or 0
  luabass.outChordOff(idBackground - p)

  if reverse then
    luabass.outChordSet(idBackground - p, 12,
      values["chord delay"].value , values["chord decay"].value,
      -1,1,table.unpack(luachord.getIndexPitches("chord",0,0)))
  else
    luabass.outChordSet(idBackground - p, 12 ,
      values["chord delay"] , values["chord decay"],
      1,-1,table.unpack(luachord.getIndexPitches("chord",0,0)))
  end
  --local t = luachord.getIndexPitches("chord", 1, 0)
  --local pitch = " pitch=" 
  --for i,v in ipairs(t) do
  --  pitch = pitch .. v .. ","
  --end
  --luabass.logmsg("onBackground track=" .. tracks["chord-background"] .. " velocity=" .. velocity .. pitch)
  luabass.outChordOn(idBackground - p,(velocity or 64),0,tracks["chord-background"])
end

function playGround(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  ------===========
  -- basic way to to play bass and background 
  -- changing from white to black keys ( and vice-versa ) : go to next chord
  if velocity == 0 then
    if black == playBlack then
      luabass.outChordOff(idBass) -- for legato bass, use selector "note on-only"  
    end
    return
  end
 
 if black ~= playBlack  then -- invert black and white : change the chord
    luachord.nextChord()
    playBlack = black
  end
  
  if luachord.getIdChord() ~= idBackgroundChord then -- New Chord : start bass and background
    idBackgroundChord = luachord.getIdChord()
    if idBackgroundChord > 0 then
      onBass(velocity)
      onBackground(velocity)
    else
      luabass.outChordOff(idBass)
      luabass.outChordOff(idBackground)
    end
  else
    onBass(velocity) 
  end
end


function playWalkingBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
---------=================
  if whiteindex > 4 then return end 
  if velocity == 0 then
    if idWalkingBass[bid] then
      luabass.outChordOff(idWalkingBass[bid])
    end
    return
  idWalkingBass[bid] =  luabass.outChordSet(-1, 12,0,0,1,1,table.unpack(luachord.getIndexPitches("bass",whiteindex,black)))
  luabass.outChordOn(idWalkingBass[bid],velocity,0,tracks["chord-bass"])
  end
end

function playChords(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  ------===========
  -- Basic way to to play bass, background , and chord 
  -- Changing from white to black keys ( and vice-versa ) : go to next chord
  -- If the chord is a new one : play the bass and the background 
  -- Right ( or Left ) ,  #1 2 3 4 : play the chord with more and more delay, brush down ( or up )
  -- Right ( or Left ) ,  #5 6 7 8 : idem with one octave upper
   
  if  velocity == 0  then -- noteoff for the key #bid
    --luabass.logmsg("noteoff chord " .. idPlayChord[bid] )
    if idPlayChord[bid] then
      luabass.outChordOff(idPlayChord[bid])
      idPlayChord[bid] = nil
    end
    return
  end
  if luachord.isRestart() then playBlack = 0 end
  if black ~= playBlack  then -- invert black and white : change the chord
    --luabass.logmsg("nextchord / invert")
    scaleOff()
    luachord.nextChord()
    playBlack = black
  end
  
  if  luachord.isNoChord() then -- No Chord : stop bass and background
    --luabass.logmsg("noChord")
    scaleOff()
    luabass.outChordOff(idBass)
    luabass.outChordOff(idBackground)
    return
  end
  
  if luachord.isNewChord() then -- New Chord : start bass and background
    --luabass.logmsg("new chord")
    scaleOff()
    onBass(velocity)
    onBackground(velovity)
  end

  -- calculate the chord to play
  local firstPitch = 1
  local lastPitch = -1
  local ratioDelay = whitemediane
  if ( whitemediane < 0 ) then -- brush down
    firstPitch = -1
    lastPitch = 1
    ratioDelay = whitemediane * -1
  end
  
  local octaveShift = 0 
  if ( ratioDelay >= 5 ) then -- octave up
    octaveShift = 1
    ratioDelay = ratioDelay - 5
  end
  
  --luabass.logmsg("playchord track " .. tracks["chord-chord"])
  idPlayChord[bid] = luabass.outChordSet(-1, 12 * octaveShift,
        values["chord delay"] * 2 * ratioDelay , values["chord decay"],
        firstPitch,lastPitch,table.unpack(luachord.getIndexPitches("chord",0,0)))
  -- play the chord
  luabass.outChordOn(idPlayChord[bid],velocity,0,tracks["chord-chord"])

end

function offArpeggio()
  --luabass.logmsg("offArpeggio")
  for i,v in pairs(idArpeggio) do
    --luabass.logmsg("off Arpeggio #"..v)
    luabass.outChordOff(v)
  end
  idArpeggio = {}
end
function playArpeggio(time,bid,ch,pitch,velocity,param,index,mediane,whiteindex,whitemediane,black)
  ------==============
  -- White keys = chord notes. Black key = chord note plus/minus half-tone
  -- Arpegii are "legato". First key switch off the current arpegii.
  
  if luachord.getIdChord() ~= idArpeggioChord then  
    idArpeggioChord = luachord.getIdChord()
    offArpeggio()
  end
  
  if idArpeggio[bid] then
    luabass.outChordOff(idArpeggio[bid])
  end
  idArpeggio[bid] = luabass.outChordSet(-1, 0 ,
      values["brush delay"] , values["brush decay"],
      1,-1,table.unpack(luachord.getIndexPitches("chord",whitemediane,black)))
  if idArpeggio[bid] then
    --luabass.logmsg("on  Arpeggio #"..idArpeggio[bid])
    luabass.outChordOn(idArpeggio[bid],velocity,0,tracks["chord-chord"])
  end
end

local prevPlayChordPitch , prevBlack 
local modeSound = 0

function allOff()
  scaleOff()
  allNoteOff()
  modeSound = 0
end
function playChord(time,bid,ch,pitch,velocity,param,index,mediane,whiteindex,whitemediane,black)
  if velocity > 0 then
    luabass.outChordOff(idBass)
    luabass.outChordOff(idBackground)
    onBass(64)
    onBackground(32)
  else
    scaleOff()
    luachord.nextChord(time,bid,ch,pitch,64)
  end
end
function demo(time,bid,ch,pitch,velocity,param,index,mediane,whiteindex,whitemediane,black)
  -- C1 D1  1..5: play score
  if pitch == 36 or pitch == 38 or pitch < 6 then luascore.play(time,bid,ch,pitch,velocity) end
  -- C#1 : dacapo score
  if pitch == 37  and velocity > 0 then luascore.firstPart(time,bid,ch,pitch,velocity) ;allOff()  end
  -- D#1 : dacapo chord
  if pitch == 39  and velocity > 0 then luachord.firstPart(time,bid,ch,pitch,velocity) ; allOff()  end
  if pitch == 40 then playChord(time,bid,ch,pitch,velocity) end
  
  -- Bb4 : off
  if pitch == 82  and velocity > 0 then allNoteOff() end
  -- A4 : toggle sound note
  if pitch == 81  and velocity > 0 then 
    modeSound =  modeSound + 1
    if modeSound == 3 then modeSound = 1 end
  end
  -- F1 E2 : play chord
  if pitch > 40 and pitch < 53 and velocity > 0 and modeSound == 0 then
    if prevPlayChordPitch then
      playChords(time,bid,ch,pitch,0,param,index,mediane,whiteindex,prevPlayChordPitch - 46 ,prevBlack)
    end
    prevPlayChordPitch = pitch
    prevBlack = black 
    playChords(time,bid,ch,pitch,velocity,param,index,mediane,whiteindex,pitch - 46 ,black)
  end
  -- F2 F4 : play scale
  if pitch > 53 and pitch < 78 then
    local tscale = luachord.getIndexPitches("chord",whitemediane,black)
    if tscale and type(tscale) == "table" and #tscale > 0 then
      p = tscale[1]
      if velocity == 0 then
        --luabass.logmsg("noteoff#"..pitch)
        onscale[pitch] = false
        if black == 1 or oldscale[pitch] then
          if pscale[pitch] then
            --luabass.logmsg("  off#"..pitch.."=>"..pscale[pitch].."(black="..black..")")
            luabass.outNoteOff(pscale[pitch],0,pitch,0,tracks["chord-scale"])
            pscale[pitch] = nil
            oldscale[pitch] = nil
          end
        end
      else
        --luabass.logmsg("noteon#"..pitch.."=>"..p)
        if pscale[pitch] then
          --luabass.logmsg("  offon#"..pscale[pitch])
          luabass.outNoteOff(pscale[pitch],0,pitch,0,tracks["chord-scale"])
        end
        luabass.outNoteOn(p,velocity,pitch,0,tracks["chord-scale"])
        pscale[pitch] = p
        onscale[pitch] = true
      end
    end
  end
end


-- list of actions for the GUI ( throug basslua )
actions = { 
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

  {name="global/all note off", callFunction = allNoteOff ,help="all note off", shortcut = "BACK" , icone = "all_note_off" },
  {name="global/main volume", callFunction = mainVolume },
  {name="global/track volume", callFunction = trackVolume },
  {name="global/track mute", callFunction = trackMute },
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
  {name="chord/play ground", callFunction = playGround },
  {name="chord/play chord", callFunction = playChord },
  {name="chord/play arpeggio", callFunction = playArpeggio },
  {name="chord/play walking bass", callFunction = playWalkingBass },
  {name="chord/set brush delay", callFunction = setBrushDelay } ,
  }



 

