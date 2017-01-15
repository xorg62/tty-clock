#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = ttyclock.c
CC ?= gcc
BIN = tty-clock
PREFIX ?= /usr/local
INSTALLPATH = ${DESTDIR}${PREFIX}/bin
MANPATH = ${DESTDIR}${PREFIX}/share/man/man1

ifeq ($(shell sh -c 'which ncurses5-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -g $$(ncurses5-config --cflags)
	LDFLAGS += $$(ncurses5-config --libs)
else ifeq ($(shell sh -c 'which ncursesw5-config>/dev/null 2>/dev/null && echo y'), y)
		CFLAGS += -Wall -g $$(ncursesw5-config --cflags)
		LDFLAGS += $$(ncursesw5-config --libs)
else
	CFLAGS += -Wall -g $$(pkg-config --cflags ncurses)
	LDFLAGS += $$(pkg-config --libs ncurses)
endif

tty-clock : ${SRC}

	@echo "building ${SRC}"
	${CC} ${CFLAGS} ${SRC} -o ${BIN} ${LDFLAGS}

install : ${BIN}

	@echo "installing binary file to ${INSTALLPATH}/${BIN}"
	@mkdir -p ${INSTALLPATH}
	@cp ${BIN} ${INSTALLPATH}
	@chmod 0755 ${INSTALLPATH}/${BIN}
	@echo "installing manpage to ${MANPATH}/${BIN}.1"
	@mkdir -p ${MANPATH}
	@cp ${BIN}.1 ${MANPATH}
	@chmod 0644 ${MANPATH}/${BIN}.1
	@echo "installed"

uninstall :

	@echo "uninstalling binary file (${INSTALLPATH})"
	@rm -f ${INSTALLPATH}/${BIN}
	@echo "uninstalling manpage (${MANPATH})"
	@rm -f ${MANPATH}/${BIN}.1
	@echo "${BIN} uninstalled"

clean :

	@echo "cleaning ${BIN}"
	@rm -f ${BIN}
	@echo "${BIN} cleaned"

