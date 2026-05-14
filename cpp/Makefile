CXX = x86_64-w64-mingw32-g++
WINDRES = windres

CXXFLAGS = -static -O2 -mwindows -municode -std=c++17 -DUNICODE -D_UNICODE
LDFLAGS = -lcomctl32 -lshell32 -lole32 -lgdi32 -lcomdlg32 -luser32 -lshlwapi

TARGET = SilentParamQuery.exe
SRCS = main.cpp scanner.cpp batgen.cpp batchrun.cpp
RES_RC = resource.rc
RES_O = resource.o
TOOLS_RC = tools_resource.rc
TOOLS_O = tools_resource.o

.PHONY: all clean

all: $(TARGET)

$(RES_O): $(RES_RC) resource.h 大内静探.ico
	$(WINDRES) $(RES_RC) -o $(RES_O) --input-format=rc --output-format=coff

$(TOOLS_O): $(TOOLS_RC)
	$(WINDRES) $(TOOLS_RC) -o $(TOOLS_O) --input-format=rc --output-format=coff

$(TARGET): $(SRCS) $(RES_O) $(TOOLS_O) database.h scanner.h batgen.h batchrun.h resource.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(RES_O) $(TOOLS_O) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(RES_O) $(TOOLS_O)
