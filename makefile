ORIGINAL_CC=$(CC)
# export ORIGINAL_CC
all:
	gmake -f makefile.gmake
clean:
	gmake -f makefile.gmake clean