#!/bin/sh
WINEDEBUG="fixme-all,process_begin-all" wine cmd /c "./make.bat $*"
