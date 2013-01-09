#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = ttyclock.c
CC ?= gcc
BIN = tty-clock
INSTALLPATH = /usr/local/bin/
CFLAGS ?= -O2 -g
CFLAGS += -Wall $(shell pkg-config --cflags ncurses 2>/dev/null)
LIBS = $(shell pkg-config --libs ncurses 2>/dev/null | echo -lncurses)


tty-clock : ${SRC}

	@echo "build ${SRC}"
	${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} ${SRC} -o ${BIN} ${LIBS}

install : ${BIN}

	@echo "creating target folder in ${DESTDIR}${INSTALLPATH}"
	@mkdir -p "${DESTDIR}${INSTALLPATH}"
	@echo "installing binary file to ${DESTDIR}${INSTALLPATH}${BIN}"
	@cp ${BIN} "${DESTDIR}${INSTALLPATH}"
	@chmod 755 "${DESTDIR}${INSTALLPATH}${BIN}"
	@echo "installed"

uninstall :

	@echo "uninstalling binary file (${DESTDIR}${INSTALLPATH}${BIN})"
	@rm -f "${DESTDIR}${INSTALLPATH}${BIN}"
	@echo "${BIN} uninstalled"

clean :

	@echo "cleaning ${BIN}"
	@rm ${BIN}
	@echo "${BIN} cleaned"

