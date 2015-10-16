.PHONY: validate default old gprof
.DEFAULT: default
.DEFAULT_GOAL = default

validate:
	cd ../tools/pluginvalidator && ./validate.py

default:
	./build.sh

old:
	./build.sh old

gprof:
	./build.sh gprof
