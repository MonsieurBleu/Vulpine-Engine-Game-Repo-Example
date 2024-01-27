
ifeq ($(OS),Windows_NT)
	G_EXEC = Game.exe
else
	G_EXEC = Game
endif

default : install

install :
	@$(MAKE) -C ./Engine game --no-print-directory

reinstall : 
	@$(MAKE) -C ./Engine gameReinstall --no-print-directory

clean : 
	@$(MAKE) -C ./Engine gameClean --no-print-directory

cleanall : 
	@$(MAKE) -C ./Engine gameClean --no-print-directory
	@$(MAKE) -C ./Engine clean --no-print-directory

vulpine :
	@$(MAKE) -C ./Engine reinstall --no-print-directory

debug :
	@cd build && gdb ./$(G_EXEC)

run : 
ifeq ($(OS),Windows_NT)
	cd build && $(G_EXEC)
else
	cd build && ./$(G_EXEC)
endif
