-- update : 31/10/2016 10:00
-- LUA script started by "basslua.dll"
-- "basslua.dll" manages input ( GUI, MIDI-In, timer ) up to LUA
-- the module "luabass.dll" is loaded by default
-- "luabass.dll" manages output ( MIDI-Out ) from LUA
-- function onStart(param) :  called by "basslua.dll" , at the beginning
-- midiinOpen = { 1 } -- LUA table which contains midiIn devices to open, checked regularly by "basslua.dll"
-- function on<event>(...) : called by "luabass.dll" on event
-- function onStop() : called by "basslua.dll" , before to close

instruments = { piano = 0 , accordeon = 21 , guitare = 24 }

function midiOutIsValid(midiout_name)
  -------================
  -- return -1 if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  local invalid = { "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" }
  for i,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end
function midiInIsValid(midiin_name)
  -------===============
  -- return -1 if the midiin is not valid for the GUI
  local s = string.lower(midiin_name)
  local valid = { "sd%-50 midi" }
  local invalid = { "iac" , "loop" , "sd%-50" }
  for i,v in ipairs(valid) do
    if ( string.find(s,v ) ~= nil) then
      return true ;
    end
  end
  for i,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end

function listin()
  for i,v in ipairs(lIn) do
    if midiInIsValid(v) then
      print("Midiin device" ,i,v)
    else
      print("Midiin device" ,i,"("..v..")")
    end
  end
end
function listout()
  for i,v in ipairs(lOut) do
    if midiOutIsValid(v) then
      print("Midiout device" ,i,v)
    else
      print("Midiout device" ,i,"("..v..")")
    end
  end
end

function openin(s)
  local n
  if tonumber(s) and tonumber(s) <= #lIn then
    n = tonumber(s)
  else
    for i,v in ipairs(lIn) do
      if midiInIsValid(v) and string.find(v,s) then
        n = i
      end  
    end
  end
  if n then
    print("midiIn open #" , n , lIn[n])
    midiinOpen = {}
    midiinOpen[1] = n
  else
    print("error : no midiIn matches ",s)
    listin()
  end
end

function openout(s)
  local n
  if tonumber(s)  and tonumber(s) <= #lOut then
    n = tonumber(s)
  else
    for i,v in ipairs(lOut) do
      if midiOutIsValid(v) and string.find(v,s) then
        n = i
      end  
    end
  end
  if n then
    local trackName = "track" .. s
    if luabass.outTrackOpenMidi(1, 1,"",n,3) ~= 0 then
      print("ok")
      print("midiOut open #" , n , lOut[n] , "on Track #1")
    else
      print("error opening midiOut #", n , lOut[n])
    end
  else
    print("error : no midiOut matches ",s)
    listout()
  end
end

function instrument(name)
  if instruments[name] and tonumber(instruments[name]) then
    luabass.outProgram(tonumber(instruments[name]),0,1,121,0)
    print("instrument #",tonumber(instruments[name]),name)
  else
    print("instrument unknown",name)
  end
end

function chord(c)
  local alter = 0 
  local tone = 0
  local third = 4
  local seventh = 12
  local p = 2
  local s = string.sub(c,2,2)
  if s == "#" then alter = 1 ; p = 3 ; end 
  if s == "b" then alter = -1 ; p = 3 ; end 
  s = string.upper(string.sub(c,1,1))
  if s == "C" then tone = 0
  elseif s == "D" then tone = 2
  elseif s == "E" then tone = 4
  elseif s == "F" then tone = 5
  elseif s == "G" then tone = 7
  elseif s == "A" then tone = 9
  elseif s == "B" then tone = 11
  end
  s = string.sub(c,p)
  if s == "m" then third = 3
  elseif s == "7" then seventh = 10
  elseif s == "m7" then third = 3 ; seventh = 10 ; end
  local base = 60 + tone + alter 
  --[[ luabass.outChordSet
	// parameter #1 : unique_id   with -1 : return a unique-id
	// parameter #2 : transpose
	// parameter #3 : [0..] delay in ms between notes
	// parameter #4 : ([0..50..100]) % dvelocity between notes ( 50 all equal, 25 divide by two for next note )
	// parameter #5 & #6 : start# and end# of pitches ( 1 & -1 : all pitches , -1 & 1 : all in reverse order )
	// parameter #7.. : list of pitch
  ]]--

  local id = luabass.outChordSet(-1,0,0,50,1,-1,base,base + third, base + 7,base + seventh)
  if id > 0 then
    local retid = luabass.outChordOn(id,64)
    if id > 0 then
      print("chord",c,"played")
      luabass.outChordOff(id,0,1000)
    else
      print("error playing chord #"..id)
    end
  else
    print("error setting chord #"..id)
  end
end

function sound(wavfile)
  luabass.outSoundPlay(wavfile)
end

function help()
  print("openin <name or #>" )
  print("openout <name or #>" )
  print("listin")
  print("listout")
  print("chord <chordname> ( e.g. C , G7, Dm )" )
  for i,v in pairs(instruments) do
    print("instrument " .. i )
  end
  print("transpose [-12..12]" )
  print("sound <file.wav>")
  print("exit")
  print("help")
end


function onStart(param)
  --
  print("LUA start onStart")
  print()
  print("list of MIDI interfaces :")
  lOut = luabass.outGetMidiList() 
  lIn = luabass.inGetMidiList() 
  print()
  listin()
  print()
  listout()
  print ()
  help()
  print()
  
  --[[
  openin("49")
  openout(3)
  instrument("guitare")
  print()
  --]]
end
function transpose(t)
  luabass.outTranspose(t)
end
function onStop()
 print("LUA stop")
end
function openVi(dll)
  luabass.outTrackOpenVi(1,1,"",dll);
end

local myDelay = 500 ;
function onNoteOn(device,t,channel,pitch,velocity)
  print("LUA noteon",device,t,channel,pitch,velocity)
  luabass.outNoteOn(pitch,velocity,pitch)
  luabass.outNoteOn(pitch + 12,velocity,pitch+128,myDelay)
end
function onNoteOff(device,t,channel,pitch,velocity)
  print("LUA noteoff",device,t,channel,pitch,velocity)
  luabass.outNoteOff(pitch,0,pitch)
  luabass.outNoteOff(pitch + 12,0,pitch+128)
 end
 function onControl(device,t,channel,nrControl,value)
   print("LUA control",device,t,channel,nrControl,value)
 myDelay = 2 * value ;
 end


