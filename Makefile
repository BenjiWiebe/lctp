CC=gcc
CFLAGS= -g -c -std=c11 -Wall -DENABLE_DBG=1
LDFLAGS=-g
TEST_SRC=test_datetime.c datetime.c
SOURCES=read.c datetime.c
OBJECTS=$(SOURCES:.c=.o)
TEST_OBJS=$(TEST_SRC:.c=.o)
EXECUTABLE=read
TEST=test_datetime

all: $(SOURCES) $(EXECUTABLE)
test: $(TEST_SRC) $(TEST)

$(TEST): $(TEST_OBJS) 
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY: clean
