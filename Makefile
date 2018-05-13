CXX = g++
CXXFLAGS = -g -std=c++11

all :main clean

main: disk.o filesystem.o main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

gen:
	$(CXX) $(CXXFLAGS) generate_disk.cpp -o $@

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -rf *.dSYM/
