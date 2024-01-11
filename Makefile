
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

engine :
	$(MAKE) -C ./Engine

run : 
	cd build && $(G_EXEC)
