CXX = g++
CXXFLAGS = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include
LDFLAGS = -L/opt/homebrew/opt/libomp/lib -lomp

proj01: proj01.cpp
	$(CXX) $(CXXFLAGS) proj01.cpp -o proj01 $(LDFLAGS)

clean:
	rm -f proj01
