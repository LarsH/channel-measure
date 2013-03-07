$(MAKE) = tools/make.exe
all: out/all

out:
	@mkdir out

.PHONY:
out/all: out
	$(MAKE) -C out -f ../src/Makefile

.PHONY:
clean:
	$(MAKE) -C out -f ../src/Makefile clean
	-rmdir out
