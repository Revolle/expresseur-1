WX_CONFIG := wx-config

LUA_VERSION := 5.3.3
LUA_DIR := lua-$(LUA_VERSION)/src

PLATFORM := $(shell uname)

ifeq ($(PLATFORM),Darwin)
    PLATFORM_DIR := mac

    PLATFORM_FLAGS := -mmacosx-version-min=10.9 -arch i386

    DYNLIB_EXT := dylib
    DYNLIB_LDFLAGS := -dynamiclib
else
    $(error Define PLATFORM_DIR containing the required headers and libraries)

    DYNLIB_EXT := so
    DYNLIB_LDFLAGS := -Wl,-z,defs -shared
endif

CPPFLAGS := -Wall -MD -MP
CXXFLAGS := -std=c++11 $(PLATFORM_FLAGS)
LDFLAGS  := $(PLATFORM_FLAGS)

EXPRESSEUR := expresseur/expresseur
EXPRESSEUR_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard expresseur/*.cpp))
$(EXPRESSEUR_OBJECTS): CPPFLAGS += -Iluabass -Ibasslua $(shell $(WX_CONFIG) --cxxflags)

LIBBASSLUA := basslua/libbasslua.$(DYNLIB_EXT)
LIBBASSLUA_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard basslua/*.cpp))

LIBLUABASS := luabass/libluabass.$(DYNLIB_EXT)
LIBLUABASS_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard luabass/*.cpp))

$(LIBBASSLUA_OBJECTS): CPPFLAGS += -Iluabass
$(LIBBASSLUA_OBJECTS) $(LIBLUABASS_OBJECTS): \
    CPPFLAGS += -Iexpresseur -I$(LUA_DIR) -I$(PLATFORM_DIR)/bass/include -I$(PLATFORM_DIR)/vsti/include

BASS_AND_LUA_LIBS := -L$(PLATFORM_DIR)/bass/lib -lbass -lbassmidi -lbassmix $(LUA_DIR)/liblua.a -framework CoreFoundation -framework CorEMIDI

ifeq ($(PLATFORM),Darwin)
all: expresseur/Expresseur.app

expresseur/Expresseur.app: $(EXPRESSEUR) expresseur/Info.plist
	-$(RM) -r $@
	-mkdir -p $@/Contents/MacOS
	cp expresseur/Info.plist $@/Contents
	cp $(EXPRESSEUR) $(LIBLUABASS) $(LIBBASSLUA) $(PLATFORM_DIR)/bass/lib/*.dylib $@/Contents/MacOS
	for f in expresseur libbasslua.dylib; do \
	    install_name_tool \
		-change basslua/libbasslua.dylib @loader_path/libbasslua.dylib \
		-change luabass/libluabass.dylib @loader_path/libluabass.dylib \
		$@/Contents/MacOS/$$f; \
	done
else
all: $(EXPRESSEUR)
endif

$(EXPRESSEUR): $(EXPRESSEUR_OBJECTS) $(LIBBASSLUA) $(LIBLUABASS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(shell $(WX_CONFIG) --libs base,net,core,adv,xml)

$(LIBBASSLUA): $(LIBBASSLUA_OBJECTS)
	$(CXX) $(DYNLIB_LDFLAGS) $(LDFLAGS) -o $@ $^ $(BASS_AND_LUA_LIBS)

$(LIBLUABASS): $(LIBLUABASS_OBJECTS)
	$(CXX) $(DYNLIB_LDFLAGS) $(LDFLAGS) -o $@ $^ $(BASS_AND_LUA_LIBS)

clean:
	$(RM) $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSEUR_OBJECTS) $(EXPRESSEUR) \
	    $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS) $(LIBBASSLUA) \
	    $(LIBLUABASS_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS) $(LIBLUABASS)

.PHONY: all clean

-include $(EXPRESSEUR_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS:.o=.d)
