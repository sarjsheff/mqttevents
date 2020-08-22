CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lmosquitto
SOURCES=mqttevents.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mqttevents

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)

# $@ - target
# $< - proccessed file
