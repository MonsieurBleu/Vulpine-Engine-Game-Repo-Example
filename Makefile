
ifeq ($(OS),Windows_NT)
	G_EXEC = Game.exe
else
	G_EXEC = Game
endif

MAKE_FLAGS = --no-print-directory
MAKE_PARALLEL = -j 16 -k

default : install

install : 
	@$(MAKE) -C ./Engine game $(MAKE_FLAGS) $(MAKE_PARALLEL)

reinstall : 
	@$(MAKE) -C ./Engine gameClean $(MAKE_FLAGS)
	@$(MAKE) -C ./Engine game $(MAKE_FLAGS) $(MAKE_PARALLEL)

clean : 
	@$(MAKE) -C ./Engine gameClean $(MAKE_FLAGS)

cleanall : 
	@$(MAKE) -C ./Engine gameClean $(MAKE_FLAGS)
	@$(MAKE) -C ./Engine clean $(MAKE_FLAGS)

vulpine :
	@$(MAKE) -C ./Engine clean $(MAKE_FLAGS)
	@$(MAKE) -C ./Engine install $(MAKE_FLAGS) $(MAKE_PARALLEL)

debug :
	@cd build && gdb ./$(G_EXEC)

run : 
ifeq ($(OS),Windows_NT)
	cd build && $(G_EXEC)
else
	cd build && ./$(G_EXEC)
endif
