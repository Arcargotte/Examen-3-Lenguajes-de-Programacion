CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage

# Archivos del programa principal
SRC = Type-Manager.cpp Functions.hpp
OBJ = $(SRC:.cpp=.o)

# Archivos de pruebas
TEST_SRC = Tests.cpp Functions.hpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)

all: main

# Programa principal
main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o Type-Manager $(OBJ)

run: main
	./Type-Manager

# Tests
tests: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o Tests $(TEST_OBJ)
	./Tests

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

coverage: tests
	./Tests
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage-report

clean:
	rm -f *.o Type-Manager Tests coverage.info
	rm -rf coverage-report *.gcda *.gcno
