PROGRAM = main
CC 	= gcc
CFLAGS  = -g -Wall
LDFLAGS = -lGL -lGLU -lglut

$(PROGRAM): main.o
	$(CC) -o $(PROGRAM) main.o $(LDFLAGS) -lm 
	
.PHONY: clean dist

clean: 
	-rm *.o $(PROGRAM) 

dist: clean
	-tar -chvj -C .. -f ../$(PROGRAM).tar.bz2 $(PROGRAM)
