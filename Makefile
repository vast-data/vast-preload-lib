# 
# Use "make help" to find out about configuration options.
#

LIB_NAME           ?= vastpreload
LIB_VER_MAJOR      ?= 1
LIB_VER_MINOR      ?= 0
LIB_VER_PATCHLEVEL ?= 3
LIB_VERSION        ?= $(LIB_VER_MAJOR).$(LIB_VER_MINOR)-$(LIB_VER_PATCHLEVEL)
LIB                ?= $(BIN_PATH)/lib$(LIB_NAME).so
LIB_UNSTRIPPED     ?= $(BIN_PATH)/lib$(LIB_NAME)-unstripped.so

SOURCE_PATH        ?= ./source
BIN_PATH           ?= ./bin
PACKAGING_PATH     ?= ./packaging

INST_PATH          ?= /usr/local/lib
PKG_INST_PATH      ?= /usr/lib

CC                 ?= gcc
CXX                ?= g++
STRIP              ?= strip

CXXFLAGS_BOOST     ?= -DBOOST_SPIRIT_THREADSAFE
LDFLAGS_BOOST      ?= -lboost_filesystem

CCFLAGS_COMMON   = -DLIB_NAME=\"$(LIB_NAME)\" -DLIB_VERSION=\"$(LIB_VERSION)\" $(CXXFLAGS_BOOST) \
	-D_GNU_SOURCE -I $(SOURCE_PATH) \
	-Wunused-variable -Wextra -Wno-unused-parameter -fmessage-length=0 \
	-fno-strict-aliasing -pthread -ggdb -fPIC
CCFLAGS_RELEASE  = -O3 -Wuninitialized
CCFLAGS_DEBUG    = -O0 -D_FORTIFY_SOURCE=2 -DBUILD_DEBUG

CXXFLAGS_COMMON  = -DLIB_NAME=\"$(LIB_NAME)\" -DLIB_VERSION=\"$(LIB_VERSION)\" $(CXXFLAGS_BOOST) \
	-D_GNU_SOURCE -I $(SOURCE_PATH) \
	-Wunused-variable -Woverloaded-virtual -Wextra -Wno-unused-parameter -fmessage-length=0 \
	-fno-strict-aliasing -pthread -ggdb -std=c++14 -fPIC
CXXFLAGS_RELEASE = -O3 -Wuninitialized
CXXFLAGS_DEBUG   = -O0 -D_FORTIFY_SOURCE=2 -DBUILD_DEBUG

LDFLAGS_COMMON   = -shared -fPIC -rdynamic -pthread -ldl $(LDFLAGS_BOOST)
LDFLAGS_RELASE   = -O3
LDFLAGS_DEBUG    = -O0

SOURCES_C        = $(shell find $(SOURCE_PATH) -name '*.c')
SOURCES_CPP      = $(shell find $(SOURCE_PATH) -name '*.cpp')
OBJECTS          = $(SOURCES_C:.c=.o)
OBJECTS         += $(SOURCES_CPP:.cpp=.o)
OBJECTS_CLEANUP  = $(shell find $(SOURCE_PATH) -name '*.o') # separate to clean after C file rename
DEPENDENCY_FILES = $(shell find $(SOURCE_PATH) -name '*.d')

# Release & debug flags for compiler and linker
ifeq ($(BUILD_DEBUG),)
CCFLAGS  = $(CCFLAGS_COMMON)  $(CCFLAGS_RELEASE)  $(CCFLAGS_EXTRA)
CXXFLAGS = $(CXXFLAGS_COMMON) $(CXXFLAGS_RELEASE) $(CXXFLAGS_EXTRA)
LDFLAGS  = $(LDFLAGS_COMMON) $(LDFLAGS_RELASE) $(LDFLAGS_EXTRA)
else
CCFLAGS =  $(CCFLAGS_COMMON)  $(CCFLAGS_DEBUG)  $(CCFLAGS_EXTRA)
CXXFLAGS = $(CXXFLAGS_COMMON) $(CXXFLAGS_DEBUG) $(CXXFLAGS_EXTRA)
LDFLAGS  = $(LDFLAGS_COMMON) $(LDFLAGS_DEBUG) $(LDFLAGS_EXTRA)
endif


all: $(SOURCES_CPP) $(SOURCES_C) $(LIB) 

debug:
	@$(MAKE) BUILD_DEBUG=1 all

$(LIB): $(LIB_UNSTRIPPED)
ifdef BUILD_VERBOSE
	$(STRIP) --strip-debug $(LIB_UNSTRIPPED) -o $(LIB)
else
	@echo [STRIP] $@ 
	@$(STRIP) --strip-debug $(LIB_UNSTRIPPED) -o $(LIB)
endif

$(LIB_UNSTRIPPED): $(OBJECTS) 
ifdef BUILD_VERBOSE
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(LIB_UNSTRIPPED)
else
	@echo [LINK] $@
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(LIB_UNSTRIPPED)
endif

.cpp.o: 
ifdef BUILD_VERBOSE
	$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) -E -MMD -MF$(@:.o=.d) -MT$(@) -o/dev/null
	$(CXX) $(CXXFLAGS) -o $@ -c $(@:.o=.cpp) 
else
	@echo [DEP] $(@:.o=.d)
	@$(CXX) $(CXXFLAGS) -c $(@:.o=.cpp) -E -MMD -MF$(@:.o=.d) -MT$(@) -o/dev/null
	@echo [CXX] $@
	@$(CXX) $(CXXFLAGS) -o $@ -c $(@:.o=.cpp) 
endif

.c.o: 
ifdef BUILD_VERBOSE
	$(CC) $(CCFLAGS) -c $(@:.o=.c) -E -MMD -MF$(@:.o=.d) -MT$(@) -o/dev/null
	$(CC) $(CCFLAGS) -o $@ -c $(@:.o=.c)
else 
	@echo [DEP] $(@:.o=.d)
	@$(CC) $(CCFLAGS) -c $(@:.o=.c) -E -MMD -MF$(@:.o=.d) -MT$(@) -o/dev/null
	@echo [CC] $@
	@$(CC) $(CCFLAGS) -o $@ -c $(@:.o=.c)
endif

clean: clean-packaging
ifdef BUILD_VERBOSE
	rm -rf $(OBJECTS_CLEANUP) $(DEPENDENCY_FILES) $(LIB) $(LIB_UNSTRIPPED)
	make -j1 -C test/ clean BUILD_VERBOSE=1
else
	@echo "[DELETE] OBJECTS, DEPENDENCY_FILES, BINARIES"
	@rm -rf $(OBJECTS_CLEANUP) $(DEPENDENCY_FILES) $(LIB) $(LIB_UNSTRIPPED)
	@make -j1 -C test/ clean
endif

clean-packaging:
ifdef BUILD_VERBOSE
	rm -rf \
		$(PACKAGING_PATH)/BUILDROOT \
		$(PACKAGING_PATH)/RPMS/* $(PACKAGING_PATH)/SPECS/rpm.spec
	bash -c "rm -rf $(PACKAGING_PATH)/$(LIB_NAME)*.{deb,ddeb,build,buildinfo,changes}"
else
	@echo "[DELETE] PACKAGING_FILES"
	@rm -rf \
		$(PACKAGING_PATH)/BUILDROOT \
		$(PACKAGING_PATH)/RPMS/* $(PACKAGING_PATH)/SPECS/rpm.spec
	@bash -c "rm -rf $(PACKAGING_PATH)/$(LIB_NAME)*.{deb,ddeb,build,buildinfo,changes}"
endif

install: all
	@echo 'Installing library...'
	install -p -m u=rwx,g=rx,o=rx $(LIB) $(PKG_INST_PATH)/

uninstall:
	@echo 'Removing library...'
	rm -f $(PKG_INST_PATH)/lib$(LIB_NAME).so

# prepare generic part of build-root (not the .rpm or .deb specific part)
prepare-buildroot: | all clean-packaging
	@echo "[PACKAGING] PREPARE BUILDROOT"

	mkdir -p $(PACKAGING_PATH)/BUILDROOT/$(PKG_INST_PATH)

	# copy main lib
	cp --preserve $(LIB) $(PACKAGING_PATH)/BUILDROOT/$(PKG_INST_PATH)

rpm: | prepare-buildroot
	@echo "[PACKAGING] PREPARE RPM PACKAGE"

	cp $(PACKAGING_PATH)/SPECS/rpm.spec.template $(PACKAGING_PATH)/SPECS/rpm.spec
	sed -i "s/__NAME__/$(LIB_NAME)/" $(PACKAGING_PATH)/SPECS/rpm.spec
	sed -i "s/__VERSION__/$(LIB_VER_MAJOR).$(LIB_VER_MINOR).$(LIB_VER_PATCHLEVEL)/" \
		$(PACKAGING_PATH)/SPECS/rpm.spec
	
	rpmbuild $(PACKAGING_PATH)/SPECS/rpm.spec --bb --define "_topdir $(PWD)/$(PACKAGING_PATH)" \
		--define "__spec_install_pre /bin/true" --buildroot=$(PWD)/$(PACKAGING_PATH)/BUILDROOT
	
	@echo
	@echo "All done. Your package is here:"
	@find $(PACKAGING_PATH)/RPMS -name $(LIB_NAME)*.rpm

deb: | prepare-buildroot
	@echo "[PACKAGING] PREPARE DEB PACKAGE"

	cp -r $(PACKAGING_PATH)/debian $(PACKAGING_PATH)/BUILDROOT
	
	cp $(PACKAGING_PATH)/BUILDROOT/debian/control.template \
		$(PACKAGING_PATH)/BUILDROOT/debian/control

	sed -i "s/__NAME__/$(LIB_NAME)/" $(PACKAGING_PATH)/BUILDROOT/debian/control
	
	cd $(PACKAGING_PATH)/BUILDROOT && \
		EDITOR=/bin/true VISUAL=/bin/true debchange --create --package $(LIB_NAME) --urgency low \
			--noquery --newversion "$(LIB_VER_MAJOR).$(LIB_VER_MINOR).$(LIB_VER_PATCHLEVEL)" \
			"Custom package build."
	
	cd $(PACKAGING_PATH)/BUILDROOT && \
		debuild -b -us -uc
	
	@echo
	@echo "All done. Your package is here:"
	@find $(PACKAGING_PATH) -name $(LIB_NAME)*.deb

test:
	make -j1 -C test/

help:
	@echo 'Optional Build Arguments:'
	@echo '   CXX=<PATH>            Path to an alternative C++ compiler. (Default: g++)'
	@echo '   BUILD_VERBOSE=1       Enable verbose build output.'
	@echo '   INSTALL_PREFIX=<PATH> Installation root directory. (Default: "/")'
	@echo
	@echo 'Makefile Targets:'
	@echo '   all (default)         Compile the code and create the library.'
	@echo '   clean                 Cleanup build artifacts.'
	@echo '   install               Locally install the library and config files.'
	@echo '   uninstall             Remove locally installed library and config files.'
	@echo '   rpm                   Create RPM package file.'
	@echo '   deb                   Create Debian package file.'
	@echo '   help                  Print this help text.'

.PHONY: clean test

# Include dependency files
ifneq ($(DEPENDENCY_FILES),)
include $(DEPENDENCY_FILES)
endif

