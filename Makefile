#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = ttyclock.c
CC ?= gcc
BIN = tty-clock
PREFIX ?= /usr/local
INSTALLPATH = ${DESTDIR}${PREFIX}/bin
MANPATH = ${DESTDIR}${PREFIX}/share/man/man1

ifeq ($(shell sh -c 'which ncurses6-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS ?= -Wall -g -I $$(ncurses6-config --includedir)
	LDFLAGS ?= -L $$(ncurses6-config --libdir) $$(ncursesw6-config --libs)
else ifeq ($(shell sh -c 'which ncursesw6-config>/dev/null 2>/dev/null && echo y'), y)
		CFLAGS ?= -Wall -g -I $$(ncursesw6-config --includedir)
		LDFLAGS ?= -L $$(ncursesw6-config --libdir) $$(ncursesw6-config --libs)
endif

tty-clock : ${SRC}

	@echo "build ${SRC}"
	@echo "CC ${CFLAGS} ${LDFLAGS} ${SRC}"
	@${CC} ${CFLAGS} ${SRC} -o ${BIN} ${LDFLAGS}

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

