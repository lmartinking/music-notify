CC	:= gcc
CFLAGS	:= -ggdb -Wall

# No need to touch stuff beyond this point (hopefully)

INCS	= -I../../include
LIBS	+= $(shell xmms-config --libs)
CFLAGS	+= $(shell xmms-config --cflags)
LDFLAGS = -shared
EXPORTS = exports.def

PROG	:= xmms.so

OBJECTS	:= mod-xmms.o

# Rules
${PROG}: ${OBJECTS}
	${CC} ${LIBS} ${LDFLAGS} ${OBJECTS} -o ${PROG}

.c.o:
	${CC} ${INCS} ${CFLAGS} -c $<

clean:
	rm ${PROG} *.o
