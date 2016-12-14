local pitches = 
{
  { {"C","E","G"} , {"G","E","C"} } ,
  { {"B","D","G"} , {"G","D","B"} } ,
}
local s = pitches[2]
local t = s[1]
print(table.unpack(t))
