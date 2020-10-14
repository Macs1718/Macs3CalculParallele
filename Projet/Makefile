CXX = g++
LIBS = -lm -lpthread
CXXFLAGS = -std=c++11 -fPIC  -fopenmp
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -Weffc++  -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O3 -march=native
endif

ALL=StatistiqueTexte.exe Orthonormalisation.exe enveloppe_convexe.exe

default: help

all: $(ALL)

clean:
	@rm -fr *.o *.exe *~

%.exe: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

StatistiqueTexte.exe: StatistiqueTexte.cpp
Orthonormalisation.exe: Orthonormalisation.cpp
enveloppe_convexe.exe: enveloppe_convexe.cpp

help:
	@echo "Available targets : "
	@echo "    all             : compile all executables"
	@echo "    StatiqueTexte.exe  : Compile text statistic executable"
	@echo "    Orthonormalisation.exe : Compile orthonormalisation executable"
	@echo "    enveloppe_convexe.exe     : Compile convex hull executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"


# Rapport
%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css -o $@
