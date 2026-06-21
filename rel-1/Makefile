#!/usr/bin/make -f
SHELL     := /bin/bash
.ONESHELL:
MAKEFLAGS += --silent

AUX_DIR := aux_files
MAIN    := main

LATEXMK := latexmk \
	-pdf \
	-shell-escape \
	-interaction=nonstopmode \
	-synctex=1 \
	-auxdir=$(AUX_DIR) \
	-bibtex \
	-f

.PHONY: all clean veryclean

all:
	mkdir -p $(AUX_DIR)
	$(LATEXMK) $(MAIN).tex

clean:
	rm -rf $(AUX_DIR)

veryclean: clean
	rm -f $(MAIN).pdf $(MAIN).synctex.gz
