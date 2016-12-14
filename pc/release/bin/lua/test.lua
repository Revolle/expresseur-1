function lit(s)
words={}
for w in string.gmatch(s,"%g+") do
  table.insert(words,w)
end
return words
end
print(table.concat( lit("") , "|" ))
print(table.concat( lit("toto") , "|" ))
  print(table.concat( lit("tata titi") , "|" ))
    print(table.concat( lit("toto 9 titi") , "|" ))