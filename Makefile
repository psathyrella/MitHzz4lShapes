ifneq ( $(HAVE_MAKEDEFS), 1 ) 
	include ../Makefile.defs
endif

LIB = lib/libShapes.a
OBJ = sinfo.o Correl.o SigModel.o Nuisance.o Unfolder.o Splotholder.o UpDownHists.o Model.o splotter.o M4lModel.o QqzzModel.o GgzzModel.o LljjModel.o
EXE = runSplot.exe
CCFLAGS += -std=c++0x -Wall -I  interface -I../Util/interface -I../NonMCBackground/interface -I../DataStruct/interface -I../Public/interface -I${CMSSW_RELEASE_BASE}/src -I$(CMSSW_BASE)/src/HiggsAnalysis/CombinedLimit/interface
LDFLAGS += -Llib -lShapes -L../Util/lib -lUtil -L../DataStruct/lib -lDataStruct -L../NonMCBackground/lib -lNonMCBackground -lMitHzz4lPublic -lZZMatrixElementMELA -lRooFit -lRooFitCore -lRooStats -lHiggsAnalysisCombinedLimit -L${CMSSW_BASE}/src/RooUnfold -lRooUnfold

### all ###################################################

all : lib exe

### lib ###################################################

lib : $(LIB)
$(LIB) : $(addprefix src/, $(OBJ) )
	ar cr $@ $^

%.o : %.cc
	$(CC) $(CCFLAGS) -c $< -o $@

### test ###################################################

exe : $(addprefix bin/, $(EXE) )

#bin/diffyX.exe :  src/diffyX.cc ../Util/lib/libUtil.a
#	$(CC) $(CCFLAGS) $< -o $@ $(LDFLAGS)

bin/runSplot.exe :  src/runSplot.cc ${LIB}
	$(CC) $(CCFLAGS) $< -o $@ $(LDFLAGS)

### clean ###################################################

clean : 
	rm -f src/*.o
	rm -f src/*~ 
	rm -f lib/*.a
	rm -f bin/*.exe

### depend ###################################################

SRC = $(patsubst %.o, %.cc, $(addprefix src/, $(OBJ) ) )
SRC += $(patsubst %.exe, %.cc, $(addprefix src/, $(EXE) ) )
depend : $(SRC)
	@ makedepend -f Makefile.dep -- $(CCFLAGS) -- $^

### depend rules follow ######################################

-include Makefile.dep
