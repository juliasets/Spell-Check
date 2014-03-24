SHELL := /bin/bash

G  := g++

CC  := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD  := libdistributed
SC  := SpellCorrector
GC  := git clone -q
LDU := https://github.com/juliasets/libdistributed.git
SCU := https://github.com/elizabethkilson/SpellCorrector.git

.PHONY: default
default: $(LD)/libdistributed.exe $(SC)/spellcorrector.exe

$(LD)/libdistributed.exe:
# 	Ensure that the directory contains what it should
	@if test -d "./$(LD)";  then \
		if test ! -d "./$(LD)/.git";  then \
			rm -rf $(LD); \
			echo ''; \
			echo 'Installing $(LD)...'; \
			$(GC) $(LDU); \
			echo 'Installed $(LD).'; \
			echo ''; \
		fi \
	else \
		echo ''; \
		echo 'Installing $(LD)...'; \
		$(GC) $(LDU); \
		echo 'Installed $(LD)'.; \
		echo ''; \
	fi
#	Actually run make
	@cd $(LD) && $(MAKE)

$(SC)/spellcorrector.exe:
#	Ensure that the directory contains what it should
	@if test -d "./$(SC)" ; then \
		if test ! -d "./$(SC)/.git"; then \
			rm -rf $(SC); \
			echo ''; \
			echo 'Installing $(SC).'; \
			$(GC) $(SCU); \
			echo 'Installed $(SC).'; \
			echo ''; \
		fi \
	else \
		echo ''; \
		echo 'Installing $(SC)...'; \
		$(GC) $(SCU); \
		echo 'Installed $(SC).'; \
		echo ''; \
	fi
#	Actually run make
	@cd $(SC) && $(MAKE);

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

.PHONY: help
help:
	@echo 'Usage:'
	@echo 'make: compiles. If the necessary subdirectories are not installed, installs them.'
	@echo 'make update: updates libdistributed and SpellCorrector libraries.'
	@echo 'make clean: removes all compiled code and subdirectories.'
	@echo 'make cleanish: removes all compiled code without removing subdirectories.'