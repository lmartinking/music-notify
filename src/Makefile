CC	:= gcc
CFLAGS	:= -ggdb -Wall

# No need to touch stuff beyond this point (hopefully)
PKG_PATH := /usr/local/lib/pkgconfig
PKG_DEPS := "libnotify gtk+-2.0"

PACKAGE	:= music-notify
PO_DIR	:= ./po
PO_FILE := ${PO_DIR}/${PACKAGE}.po

INCS	= -I./include
LIBS	+= $(shell PKG_CONFIG_PATH=${PKG_PATH} pkg-config ${PKG_DEPS} --libs)
CFLAGS	+= $(shell PKG_CONFIG_PATH=${PKG_PATH} pkg-config ${PKG_DEPS} --cflags) 

PROG	:= music-notify

OBJECTS	:= music-notify.o backend.o popup.o show-song.o conf.o

# Rules
${PROG}: ${OBJECTS}
	${CC} ${LIBS} ${OBJECTS} -o ${PROG}

.c.o:
	${CC} ${CFLAGS} ${INCS} -c $<

translate:
	${CC} -E ${CFLAGS} ${INCS} *.c | xgettext -L C -d ${PACKAGE} -
	msgmerge -U ${PO_DIR}/en.po ${PACKAGE}.po

clean:
	rm *.o ${PROG}
