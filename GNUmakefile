WX_CONFIG := wx-config

CPPFLAGS := -Wall -MD -MP
CXXFLAGS := -std=c++11

EXPRESSEUR := expresseur/expresseur
EXPRESSEUR_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard expresseur/*.cpp))

$(EXPRESSEUR_OBJECTS): CPPFLAGS += -Iluabass -Ibasslua $(shell $(WX_CONFIG) --cxxflags)

all: $(EXPRESSEUR)

$(EXPRESSEUR): $(EXPRESSEUR_OBJECTS)
	$(CXX) -o $(EXPRESSEUR) $(EXPRESSEUR_OBJECTS)

clean:
	$(RM) $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSEUR_OBJECTS) $(EXPRESSEUR)

-include $(EXPRESSEUR:.o=.d)
