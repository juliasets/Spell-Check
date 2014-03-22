SHELL := /bin/bash

G := g++

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := $(G) --std=c++11 -Wall -Wextra --pedantic

.PHONY: default
default: libdistributed SpellCorrector

libdistributed:
	wget -O libdistributed.zip https://github.com/juliasets/libdistributed/archive/master.zip
	unzip -o libdistributed.zip
	rm libdistributed.zip
	mv libdistributed-master libdistributed

SpellCorrector:
	wget -O spellcorrector.zip https://github.com/elizabethkilson/SpellCorrector/archive/master.zip
	unzip -o spellcorrector.zip
	rm spellcorrector.zip
	mv SpellCorrector-master spellcorrector

.PHONY: clean
clean:
	rm -rf *~