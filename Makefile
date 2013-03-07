$(MAKE) = tools/make.exe
all: out/all

out:
	@mkdir out

.PHONY:
out/%: out
	$(MAKE) -C out -f ../src/Makefile $*

clean: out/clean
	-rmdir out
