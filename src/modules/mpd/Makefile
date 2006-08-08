CC	:= gcc
CFLAGS	:= -ggdb -Wall

# No need to touch stuff beyond this point (hopefully)

INCS	= -I../../include
LIBS	+= 
CFLAGS	+= 
LDFLAGS = -shared
EXPORTS = exports.def

PROG	:= mpd.so

OBJECTS	:= mod-mpd.o libmpdclient.o

# Rules
${PROG}: ${OBJECTS}
	${CC} ${LIBS} ${LDFLAGS} ${OBJECTS} -o ${PROG}

.c.o:
	${CC} ${INCS} ${CFLAGS} -c $<

clean:
	rm ${PROG} *.o
