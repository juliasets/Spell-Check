SHELL := /bin/bash

G := g++

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := libdistributed
SC := SpellCorrector

.PHONY: default
default: $(LD)/libdistributed.exe $(SC)/spellcorrector.exe

$(LD)/libdistributed.exe:
	cd $(LD) && $(MAKE)

$(SC)/spellcorrector.exe:
	cd $(SC) && $(MAKE)

.PHONY: install
install:
	@echo 'Installing $(LD)...'
	@git clone -q https://github.com/juliasets/libdistributed.git
	@echo 'Installed $(LD)'
	@echo 'Installing $(SC)...'
	@git clone -q https://github.com/elizabethkilson/SpellCorrector.git
	@echo 'Installed $(SC)'

.PHONY: update
update:
	@echo 'Updating $(LD)...'
	@cd libdistributed && git pull -q
	@echo '$(LD) up to date.'
	@echo 'Updating $(SC)...'
	@cd $(SC) && git pull -q
	@echo '$(SC) up to date.'

.PHONY: cleanish
cleanish:
	@rm -rf *.exe *.o *.stackdump *~
	@cd $(LD) && rm -rf *.exe *.o *.stackdump *~
	@cd $(SC) && rm -rf *.exe *.o *.stackdump *~
	@echo 'Tidied up.'

.PHONY: clean
clean:
	@rm -rf *.exe *.o *.stackdump *~
	@rm -rf $(LD)
	@rm -rf $(SC)
	@echo 'Clean.'