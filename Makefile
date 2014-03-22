SHELL := /bin/bash

G := g++

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := libdistributed
SC := SpellCorrector

.PHONY: default
default: $(LD)/libdistributed.exe $(SC)/spellcorrector.exe

$(LD)/libdistributed.exe:
	cd $(LD) && make

$(SC)/spellcorrector.exe:
	cd $(SC) && make

.PHONY: install
install:
	git clone https://github.com/juliasets/libdistributed.git
	git clone https://github.com/elizabethkilson/SpellCorrector.git

.PHONY: update
update:
	cd libdistributed && git pull
	cd $(SC) && git pull

.PHONY: cleanish
cleanish:
	rm -rf *.exe *.o *.stackdump *~

.PHONY: clean
clean:
	rm -rf *.exe *.o *.stackdump *~
	rm -rf $(LD)
	rm -rf $(SC)