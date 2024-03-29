CXX = g++
CXXFLAGS = -std=c++11 -O0 -g -Wall
LDFLAGS = -lm

.PHONY: all zip clean clean-all

all: STDM

zip: STDM.cpp StdmMux.cpp StdmMux.hpp StdmSource.cpp StdmSource.hpp Makefile readme.txt
	zip William_Paape_lab4.zip $^

STDM: STDM.o StdmMux.o StdmSource.o
	$(CXX) $^ $(LDFLAGS) -o $@

STDM.o: STDM.cpp StdmMux.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

StdmMux.o: StdmMux.cpp StdmMux.hpp StdmSource.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

StdmSource.o: StdmSource.cpp StdmSource.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) STDM *.o

clean-all: clean
	$(RM) William_Paape_lab4.zip

