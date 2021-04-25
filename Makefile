CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=gnu++11 -I src -I test

SRCDIR = src
TESTDIR = test
OBJDIR = obj
EXE = hw_02
TEST_EXE = hw_02_test

all: $(EXE)

test: $(TEST_EXE)

$(EXE): $(OBJDIR)/main.o $(OBJDIR)/huffman.o
	$(CXX) $(OBJDIR)/main.o $(OBJDIR)/huffman.o -o $(EXE)

$(TEST_EXE): $(OBJDIR)/test.o $(OBJDIR)/huffman.o
	$(CXX) $(OBJDIR)/test.o $(OBJDIR)/huffman.o -o $(TEST_EXE)

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp $(SRCDIR)/huffman.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $(SRCDIR)/main.cpp -o $(OBJDIR)/main.o

$(OBJDIR)/huffman.o: $(SRCDIR)/huffman.cpp $(SRCDIR)/huffman.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $(SRCDIR)/huffman.cpp -o $(OBJDIR)/huffman.o

$(OBJDIR)/test.o: $(TESTDIR)/test.cpp $(SRCDIR)/huffman.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $(TESTDIR)/test.cpp -o $(OBJDIR)/test.o

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(EXE) $(TEST_EXE)

.PHONY: all test clean