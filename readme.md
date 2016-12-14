#Expresseur project

documentation , presentation :
www.expresseur.com

1. basslua

    C library started by the GUI, which opens :
    * lua script to catch the midi-event
    
    loads global module :
    *  luachord.lua
    *  luascore.lua
    *  luabass
    
    dependencies :
    * lua5.3 : www.lua.org
    * bass : www.un4see.com 
    * winmm/pc, coremidi/Mac
    
    makefile :
    * use pc|mac /lib or /include
    * library output in mac|pc/release
 
2. luabass :

    C library started by lua script, to render midi-out, SF2, VSTi, Wav
 
    dependencies :
    * lua5.3 : www.lua.org
    * bass, bassmix, bassmidi ( bassasio/PC ) : www.un4seen.com
    * winmm/pc, coremidi/Mac
    * vsti sdk ( include only, dynamic VSTi-dll loaded dynamically on demand, without GUI )

    makefile :
    * use pc/mac /lib or /include
    * library output in mac|pc/release
 
3. expresscmd :

   C exe command-line, which starts basslua

   dependencies :
   * basslua
   
   makefile :
   * use pc/mac /lib or /include
   * output executable command line in mac|pc/release
  
4. expresseur
   
   C++ wxwidgets GUI, which starts basslua
 
   dependencies
   * wxwidgets3.1
   * basslua
   
   makefile :
   * use pc/mac /lib or /include
   * output executable GUI in mac|pc/release
  
5. mac

   contains includes and lib for the makefiles
   * lua5.3
   * bass2.4
   * wxwidgets3.1
   * Vsti2.1
   * mnl1.2 for musical notation  
 
   release contains the mac-bundle :
   * executable expresseur and expresscmd
   * pizzicato.ttf ( true type font for mmnl score notation )
   * expresseur.ico for icone
   * dynamic lybraries
   * lua directory with lua scripts
   * userdoc : score sample and instruments, to be in user's directory

6. pc

   contains includes and lib for the makefiles
   * lua5.3
   * bass2.4
   * wxwidgets3.1
   * Vsti2.1
   * mnl1.2 for musical notation  
 
   release contains the innosetup package :
   * executable expresseur and expresscmd
   * pizzicato.ttf ( true type font for mmnl score notation )
   * expresseur.ico for icone
   * dll
   * lua directory with lua scripts
   * userdoc : score sample and instruments, to be in user's directory
   * asio4all setup
   * loopbe1 setup
   * vcredist setup


  
