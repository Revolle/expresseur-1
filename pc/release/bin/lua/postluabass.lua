-- update : 1/11/2016 10:00
-- postprocess the midiout of luabass
transpose = 0
function onNoteon(nrTrack,pitch,velocity)
  return nrTrack,"Noteon",pitch+transpose,velocity
end
function onNoteoff(nrTrack,pitch,velocity)
  return nrTrack,"Noteoff",pitch+transpose,velocity
end
function getTranspose()
  return transpose
end

