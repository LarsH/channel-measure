$(MAKE) = tools/make.exe
all: out/all flash.bat

FLASHFILE := out/channel-measure.hex

out:
	@mkdir out

.PHONY:
out/%: out
	$(MAKE) -C out -f ../src/Makefile $*

clean: out/clean
	-rmdir out

.PHONY:
flash.%: $(FLASHFILE)
	tools\stm32flash.exe -w $^ -o 0x0 $*

flash.bat:
	@echo tools\make.exe -f Makefile flash.echo tools\make.exe -f Makefile flash.COM1 > flash.bat

# Linuxtarget!
.PHONY:
pythonflash: $(FLASHFILE)
	python tools/stm32loader.py -p /dev/ttyUSB0 -e -w $^

