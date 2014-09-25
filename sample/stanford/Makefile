###############################################################################
# SPECtools95 
#   Version 0.2
#   This makefile should NOT be changed other than to customize for a 
#   particular benchmark.  Users may overide variables by placing them
#   in the Makefile.spec file.  Or by using the "runspec" program to 
#   build the Makefile.spec file for them.

###############################################################################
#	BENCHMARK	-- standard definitions for this benchmark

NUMBER    = 099
NAME      = Hello
EXEBASE   = Hello
OFILES    = Hello Bubble FFT Intmm Mm Perm Puzzle Queens Quick Towers Trees
LIBS      = /usr/home/tanvir-a/proj-lap/lib/csim-lib/_start.o /usr/home/tanvir-a/proj-lap/lib/csim-lib/_sub.o -L/usr/home/tanvir-a/proj-lap/lib/gcc/frv-elf/4.4.6/
LD        = frv-elf-ld -static -M /usr/home/tanvir-a/proj-lap/lib/csim-lib/_map -noinhibit-exec


###############################################################################
#	DEFINES		-- standard definitions for SPEC
#                          these may be overriden in Makefile.spec
BENCHMARK = $(NUMBER).$(NAME)
EXT       = 
OBJS      = $(addsuffix $(OBJ),$(OFILES))
CC        = frv-elf-gcc -mgpr-32 -mfpr-32 -msoft-float -mno-dword -mno-double -mno-media -mno-muladd -mpack -mvliw-branch -mfixed-cc -mno-cond-move -mno-cond-exec -mno-multi-cond-exec -mno-nested-cond-exec -macc-4 -mno-scc -mcpu=fr550 -O2

CFLAGS    = $(EXTRA_CFLAGS) $(FDO_FLAGS) $(OPTIMIZE)
FC        = f77
FFLAGS    = $(EXTRA_FFLAGS) $(FDO_FLAGS) $(OPTIMIZE)
LDFLAGS   = $(EXTRA_LDFLAGS) $(FDO_LDFLAGS)
LDOPT     = -o $@
OPTIMIZE  = $(EXTRA_OPTIMIZE)
OBJOPT    = -c -o $@
OBJ       = .o
SPECSHELL = /bin/sh
ECHO      = echo
PATHSEP   = /
PARENTDIR = ..
BINARY    = FRV-Bubble FRV-FFT FRV-Intmm FRV-Mm FRV-Perm FRV-Puzzle FRV-Queens FRV-Quick FRV-Towers FRV-Trees FRV-Hello

###############################################################################
#	TARGETS		-- Rules to build benchmark executables
all:    $(EXTRA_TARGETS) $(BINARY)

.SUFFIXES: $(OBJ)
.c$(OBJ):
	$(CC) $(OBJOPT) $(CFLAGS) $<
.s$(OBJ):
	$(CC) $(OBJOPT) $(CFLAGS) $<

# template build target, should work for simple builds
$(BINARY): $(OBJS)
	$(LD) $(LDFLAGS) $(LIBS) Hello.o  $(EXTRA_LIBS) -lgcc -o FRV-Hello
	$(LD) $(LDFLAGS) $(LIBS) Bubble.o $(EXTRA_LIBS) -lgcc -o FRV-Bubble
	$(LD) $(LDFLAGS) $(LIBS) FFT.o    $(EXTRA_LIBS) -lgcc -o FRV-FFT
	$(LD) $(LDFLAGS) $(LIBS) Intmm.o  $(EXTRA_LIBS) -lgcc -o FRV-Intmm
	$(LD) $(LDFLAGS) $(LIBS) Mm.o     $(EXTRA_LIBS) -lgcc -o FRV-Mm
	$(LD) $(LDFLAGS) $(LIBS) Perm.o   $(EXTRA_LIBS) -lgcc -o FRV-Perm
	$(LD) $(LDFLAGS) $(LIBS) Puzzle.o $(EXTRA_LIBS) -lgcc -o FRV-Puzzle
	$(LD) $(LDFLAGS) $(LIBS) Queens.o $(EXTRA_LIBS) -lgcc -o FRV-Queens
	$(LD) $(LDFLAGS) $(LIBS) Quick.o  $(EXTRA_LIBS) -lgcc -o FRV-Quick
	$(LD) $(LDFLAGS) $(LIBS) Towers.o $(EXTRA_LIBS) -lgcc -o FRV-Towers
	$(LD) $(LDFLAGS) $(LIBS) Trees.o  $(EXTRA_LIBS) -lgcc -o FRV-Trees

# to clean things up
.PHONY: clean clobber
clean:
ifeq ($(OBJ),.c)
	rm -rf core *.err *.out
else
	rm -rf $(OBJS) core *.err *.out
endif

clobber: clean
	rm -rf $(BINARY)
