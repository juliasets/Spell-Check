SHELL := /bin/bash

G  := g++

CC  := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD  := $(G) --std=c++11 -Wall -Wextra --pedantic -lboost_system -pthread
LB  := ../libdistributed
SC  := ../SpellCorrector
GC  := git clone -q
LBU := https://github.com/juliasets/libdistributed.git
SCU := https://github.com/elizabethkilson/SpellCorrector.git
LIBS := -L$(LB) -ldistributed -lboost_system -pthread -lsqlite3

.PHONY: default
default: $(LB)/libdistributed.a $(SC)/spellcorrector.exe client slave

$(LB)/libdistributed.a:
# 	Ensure that the directory contains what it should
	@if test -d "$(LB)";  then \
		if test ! -d "./$(LB)/.git";  then \
			rm -rf $(LB); \
			echo ''; \
			echo 'Installing $(LB)...'; \
			cd ..; \
			$(GC) $(LBU); \
			cd Spell-Check; \
			echo 'Installed $(LB).'; \
			echo ''; \
		fi \
	else \
		echo ''; \
		echo 'Installing $(LB)...'; \
		cd ..; \
		$(GC) $(LBU); \
		cd Spell-Check; \
		echo 'Installed $(LB)'.; \
		echo ''; \
	fi
#	Actually run make
	@cd $(LB) && $(MAKE) libdistributed.a

$(SC)/spellcorrector.exe:
#	Ensure that the directory contains what it should
	@if test -d "$(SC)" ; then \
		if test ! -d "./$(SC)/.git"; then \
			rm -rf $(SC); \
			echo ''; \
			echo 'Installing $(SC).'; \
			cd ..; \
			$(GC) $(SCU); \
			cd Spell-Check; \
			echo 'Installed $(SC).'; \
			echo ''; \
		fi \
	else \
		echo ''; \
		echo 'Installing $(SC)...'; \
		cd ..; \
		$(GC) $(SCU); \
		cd Spell-Check; \
		echo 'Installed $(SC).'; \
		echo ''; \
	fi
#	Actually run make
	@cd $(SC) && $(MAKE);

.PHONY: test
test: master-test slave client
	./master-test & ./SpellCheckClient 127.0.0.1 30000 & ./SpellCheckSlave 127.0.0.1 30000 &

master-test: Master-test.cpp $(LB)/Master.o $(LB)/Master.hpp
	$(CC) Master-test.cpp
	$(LD) -o master-test Master-test.o $(LIBS)

client: SpellCheckClient.o
	$(LD) -o SpellCheckClient SpellCheckClient.o $(LIBS)

SpellCheckClient.o: SpellCheckClient.cpp $(LB)/Client.hpp $(LB)/utility.hpp
	$(CC) SpellCheckClient.cpp

slave: SpellCheckSlave.o
	$(LD) -o SpellCheckSlave SpellCheckSlave.o $(SC)/threadedSpellCorrector.o $(SC)/corrector.o $(SC)/string_functions.o $(LIBS)

SpellCheckSlave.o: SpellCheckSlave.cpp $(LB)/Slave.hpp $(LB)/utility.hpp
	$(CC) SpellCheckSlave.cpp

.PHONY: killtest
killtest: 
	pkill SpellCheckSlave & pkill SpellCheckClient & pkill master-test &

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
	@rm -rf worker
	@echo 'Clean.'

.PHONY: help
help:
	@echo 'Usage:'
	@echo 'make: compiles. If the necessary subdirectories are not installed, installs them.'
	@echo 'make update: updates libdistributed and SpellCorrector libraries.'
	@echo 'make clean: removes all compiled code and subdirectories.'
	@echo 'make cleanish: removes all compiled code without removing subdirectories.'
