tty-clock : clock.c

	@echo "*** Building ***"
	@cc -lncurses $^ -o tty-clock -Wall -v
	@echo "*** TTY-Clock build ***"
	@echo ""

install : tty-clock
  
	@echo "*** Moving TTY-Clock ***"
	@cp tty-clock /usr/local/bin/
	@echo "*** Clock moved ***"
	@echo ""
	@echo "*** Chmod TTY-Clock ***"
	@chmod 777 /usr/local/bin/tty-clock
	@echo "*** TTY-Clock is now installed ***"
	@echo ""

uninstall :

	@echo "*** Uninstall TTY-Clock ***"
	@rm -f /usr/local/bin/tty-clock
	@echo "*** TTY-Clock uninstalled :'( ***"

clean :

	@echo "*** Cleaning TTY-clock ***"
	@rm tty-clock
	@echo "*** Your TTY-Clock is clean ;) ***"

