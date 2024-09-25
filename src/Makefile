CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

EXECUTABLE = IFJ24

all: $(EXECUTABLE)
	@echo "Project compiled successfuly!"

$(EXECUTABLE): $(wildcard *.c)
	$(CC) $(CFLAGS) -o $@ $^

%o : %c
	$(CC) $(CFLAGS) -c $<

run:
	@./$(EXECUTABLE)

clean:
	rm -f *.o $(EXECUTABLE)

.PHONY: all run clean
