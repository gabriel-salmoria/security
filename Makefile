#!/usr/bin/make -f
SHELL     := /bin/bash
MAKEFLAGS += --silent

.PHONY: all rel-1 rel-2 clean veryclean

all: rel-1 rel-2

rel-1:
	$(MAKE) -C rel-1

rel-2:
	$(MAKE) -C rel-2

clean:
	$(MAKE) -C rel-1 clean
	$(MAKE) -C rel-2 clean

veryclean:
	$(MAKE) -C rel-1 veryclean
	$(MAKE) -C rel-2 veryclean
