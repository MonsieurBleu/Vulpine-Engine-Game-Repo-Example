
ifeq ($(OS),Windows_NT)
	G_EXEC = Game.exe
else
	G_EXEC = Game.exe
endif

default : install

install :
	$(MAKE) -C ./Engine game

engine :
	$(MAKE) -C ./Engine

run : 
	cd build && $(G_EXEC)
