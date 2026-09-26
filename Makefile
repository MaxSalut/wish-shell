CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17

wish: wish.cpp
	$(CXX) $(CXXFLAGS) -o wish wish.cpp

debug: wish.cpp
	$(CXX) $(CXXFLAGS) -DDEBUG -o wish wish.cpp

clean:
	rm -f wish
	rm -rf tests-out
