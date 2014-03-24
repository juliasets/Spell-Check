SHELL := /bin/bash

G  := g++

CC  := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD  := $(G) --std=c++11 -Wall -Wextra --pedantic -lboost_system -pthread
LB  := libdistributed
SC  := SpellCorrector
GC  := git clone -q
LBU := https://github.com/juliasets/libdistributed.git
SCU := https://github.com/elizabethkilson/SpellCorrector.git

.PHONY: default
default: $(LB)/libdistributed.exe $(SC)/spellcorrector.exe client

$(LB)/libdistributed.exe:
# 	Ensure that the directory contains what it should
	@if test -d "./$(LB)";  then \
		if test ! -d "./$(LB)/.git";  then \
			rm -rf $(LB); \
			echo ''; \
			echo 'Installing $(LB)...'; \
			$(GC) $(LBU); \
			echo 'Installed $(LB).'; \
			echo ''; \
		fi \
	else \
		echo ''; \
		echo 'Installing $(LB)...'; \
		$(GC) $(LBU); \
		echo 'Installed $(LB)'.; \
		echo ''; \
	fi
#	Actually run make
	@cd $(LB) && $(MAKE)

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

client: client.o utility.o
	$(LD) -o client client.o $(LB)/libdistributed.o utility.o

client.o: client.cpp $(LB)/libdistributed.hpp
	$(CC) client.cpp

utility.o: $(LB)/utility.hpp $(LB)/utility_macros.hpp $(LB)/utility.cpp
	$(CC) $(LB)/utility.cpp

.PHONY: update
update:
	@echo 'Updating $(LB)...'
	@cd libdistributed && git pull -q
	@echo '$(LB) up to date.'
	@echo 'Updating $(SC)...'
	@cd $(SC) && git pull -q
	@echo '$(SC) up to date.'

.PHONY: cleanish
cleanish:
	@rm -rf *.exe *.o *.stackdump *~
	@cd $(LB) && rm -rf *.exe *.o *.stackdump *~
	@cd $(SC) && rm -rf *.exe *.o *.stackdump *~
	@echo 'Tidied up.'

.PHONY: clean
clean:
	@rm -rf *.exe *.o *.stackdump *~
	@rm -rf $(LB)
	@rm -rf $(SC)
	@rm -rf client
	@echo 'Clean.'

.PHONY: help
help:
	@echo 'Usage:'
	@echo 'make: compiles. If the necessary subdirectories are not installed, installs them.'
	@echo 'make update: updates libdistributed and SpellCorrector libraries.'
	@echo 'make clean: removes all compiled code and subdirectories.'
	@echo 'make cleanish: removes all compiled code without removing subdirectories.'
