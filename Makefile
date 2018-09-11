CXX = g++
RM  = rm -rf

CXXFLAGS = -O3 -std=c++11 -Wall -Wextra -Wpedantic -m64 -fopenmp
INCS =
LIBS =
LNKS =

TGTS = skipgram.out

.PHONY: all build clean run

all: $(TGTS)

%.out: src/%.cxx
	$(CXX) $(CXXFLAGS) $< -o $@ $(INCS) $(LIBS) $(LNKS)

clean:
	$(RM) $(TGTS)

run: skipgram.out
	./$<
