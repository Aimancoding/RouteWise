CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -Isrc

all: routewise tests

routewise: src/main.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -o routewise src/main.cpp

test_member1: tests/test_member1.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -o $@ tests/test_member1.cpp
test_member2: tests/test_member2.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -o $@ tests/test_member2.cpp
test_member3: tests/test_member3.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -o $@ tests/test_member3.cpp

tests: test_member1 test_member2 test_member3

check: tests
	./test_member1 > /dev/null && echo "Member 1 tests OK"
	./test_member2 | tail -1
	./test_member3 | tail -1

clean:
	rm -f routewise test_member1 test_member2 test_member3
