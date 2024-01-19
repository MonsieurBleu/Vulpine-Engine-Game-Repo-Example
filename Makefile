
ifeq ($(OS),Windows_NT)
	G_EXEC = Game.exe
else
	G_EXEC = Game
endif

default : install

install :
	$(MAKE) -C .\Engine game

reinstall : 
	$(MAKE) -C .\Engine gameReinstall

clean : 
	$(MAKE) -C .\Engine gameClean

cleanall : 
	$(MAKE) -C ./Engine gameClean
	$(MAKE) -C ./Engine clean

vulpine :
	$(MAKE) -C .\Engine reinstall

debug :
	cd build && gdb $(G_EXEC)

run : 
	cd build && $(G_EXEC)
