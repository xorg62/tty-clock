#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = ttyclock.c
CC ?= gcc
BIN = tty-clock
PREFIX ?= /usr/local
INSTALLPATH = ${DESTDIR}${PREFIX}/bin
MANPATH = ${DESTDIR}${PREFIX}/share/man/man1

ifeq ($(shell sh -c 'which ncurses5-config&>/dev/null; $?'), 0)
	CFLAGS ?= -Wall -g -I $$(ncurses5-config --includedir)
	LDFLAGS ?= -L $$(ncurses5-config --libdir) $$(ncursesw5-config --libs)
else ifeq ($(shell sh -c 'which ncursesw5-config&>/dev/null; $?'), 0)
	CFLAGS ?= -Wall -g -I $$(ncursesw5-config --includedir)
	LDFLAGS ?= -L $$(ncursesw5-config --libdir) $$(ncursesw5-config --libs)
else ifeq ($(shell sh -c 'brew ls --versions ncurses'), ncurses 6.0)
	CFLAGS ?= -Wall -g -D_DARWIN_C_SOURCE -I/usr/local/Cellar/ncurses/6.0/include
	LDFLAGS ?= -L/usr/local/Cellar/ncurses/6.0/lib -lncursesw
else
$(error Your build environment is not supported)
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

