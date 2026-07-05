#!/usr/bin/make -f
SHELL     := /bin/bash
MAKEFLAGS += --silent

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I./implementation/src -I/usr/include/SDL2 -I./implementation/imgui -I./implementation/imgui/backends
LDFLAGS = -lpbc -lgmp -lcrypto -lSDL2 -lGL -ldl

SRCS := $(shell find implementation/src -name "*.cpp")
IMGUI_SRCS := $(wildcard implementation/imgui/*.cpp) $(wildcard implementation/imgui/backends/imgui_impl_sdl2.cpp) $(wildcard implementation/imgui/backends/imgui_impl_opengl3.cpp)

ALL_SRCS = $(SRCS) $(IMGUI_SRCS)
OBJS = $(ALL_SRCS:%=implementation/obj/%.o)

TARGET = implementation/ibemail_app

.PHONY: all rel-1 rel-2 clean veryclean build_app clean_app

all: rel-1 rel-2 build_app

rel-1:
	$(MAKE) -C rel-1

rel-2:
	$(MAKE) -C rel-2

build_app: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

implementation/obj/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(MAKE) -C rel-1 clean
	$(MAKE) -C rel-2 clean
	rm -rf implementation/obj $(TARGET)

veryclean:
	$(MAKE) -C rel-1 veryclean
	$(MAKE) -C rel-2 veryclean
	rm -rf implementation/obj $(TARGET)

clean_app:
	rm -rf implementation/obj $(TARGET)
