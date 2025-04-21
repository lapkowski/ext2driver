CXX ?= g++
MKE2FS ?= /sbin/mke2fs
CXXFLAGS ?= -Wall -Wextra -std=gnu++20 $(if $(DEBUG),-ggdb -D_DEBUG,-Os -s -fno-ident -fno-asynchronous-unwind-tables)
LDFLAGS ?= -lm

BUILDDIR ?= _build
TESTIMG_ROOT ?= testimgroot

SRC_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(wildcard src/*.cpp))

$(BUILDDIR)/driver: $(OBJ_FILES) $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_FILES) $(LDFLAGS)

$(BUILDDIR)/%.o: src/%.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

img: $(BUILDDIR)/testimg.img

$(BUILDDIR)/testimg.img: $(BUILDDIR)
	$(MKE2FS) -F -L '' -N 0 -O ^64bit -d $(TESTIMG_ROOT) -m 5 -r 1 -t ext2 $@ 1G

run: $(BUILDDIR)/testimg.img $(BUILDDIR)/driver
	$(BUILDDIR)/driver $(BUILDDIR)/testimg.img

.PHONY: compile_commands.json
compile_commands.json:
	@echo "Generating compile_commands.json..."
	bear --append -- $(MAKE) -j 1

check:
	@export DEBUG=1; $(MAKE) _check
	@export DEBUG=; $(MAKE) _check
	cat $(BUILDDIR)/cppcheck_log.log
	diff -u $(BUILDDIR)/cppcheck_log.log $(BUILDDIR)/cppcheck_log_debug.log || true

_check: compile_commands.json
	cppcheck --enable=all --project=compile_commands.json --suppress=missingInclude --suppress=internalAstError 2>$(BUILDDIR)$(if $(DEBUG),/cppcheck_log_debug.log,/cppcheck_log.log)
