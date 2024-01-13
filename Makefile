
ifeq ($(OS),Windows_NT)
	G_EXEC = Game.exe
else
	G_EXEC = Game.exe
endif

default : install

install :
	$(MAKE) -C ./Engine game

reinstall : 
	$(MAKE) -C ./Engine gameReinstall

clean : 
	$(MAKE) -C ./Engine gameClean

vulpine :
	$(MAKE) -C ./Engine reinstall

debug :
	cd build && gdb $(G_EXEC)

run : 
	cd build && $(G_EXEC)
