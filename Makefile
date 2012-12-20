#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = ttyclock.c
CC = cc
BIN = tty-clock
PREFIX ?= /usr/local
INSTALLPATH = ${DESTDIR}${PREFIX}/bin
MANPATH = ${DESTDIR}${PREFIX}/share/man/man1
CFLAGS = -Wall -g -I $$(ncurses5-config --includedir)
LDFLAGS = -L $$(ncurses5-config --libdir) $$(ncurses5-config --libs)


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

