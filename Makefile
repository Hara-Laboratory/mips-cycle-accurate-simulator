CC=g++
CFLAGS=-c -g -O2
LDFLAGS=-lelf
SOURCES=core.cpp fetch.cpp decode.cpp exec.cpp mem.cpp wb.cpp reg_file.cpp mem_file.cpp read_file.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sim
 
all:	$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS) *~ dump 

run:
	./$(EXECUTABLE)
