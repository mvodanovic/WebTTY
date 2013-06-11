### Makefile configuration ###
webttyd_NAME := webttyd
webttyd_INCLUDE_DIRS := ../inc
webttyd_LIBRARY_DIRS :=
webttyd_LIBRARIES := ssh ssh_threads pthread
webttyd_OBJ_DIR := obj
webttyd_DESTDIR := /usr/local/bin


### -- NO EDITING BELOW THIS POINT -- ###


### Variable declarations ###
webttyd_C_SRCS := $(wildcard *.c)
webttyd_C_OBJS := ${webttyd_C_SRCS:.c=.o}
webttyd_CXX_SRCS := $(wildcard *.cpp)
webttyd_CXX_OBJS := ${webttyd_CXX_SRCS:.cpp=.o}
webttyd_OBJS := $(webttyd_C_OBJS) $(webttyd_CXX_OBJS)
webttyd_INCS := $(shell find $(foreach includedir,$(webttyd_INCLUDE_DIRS),$(includedir)) -name '*.h')
ifndef $(DESTDIR)
	DESTDIR := $(webttyd_DESTDIR)
endif


### Variable augmentations ###
CPPFLAGS += $(foreach includedir,$(webttyd_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(webttyd_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(webttyd_LIBRARIES),-l$(library))


### Phonies ###
.PHONY: all clean distclean install uninstall help


### Target: all ###
all: $(webttyd_NAME)

$(webttyd_CXX_OBJS): $(webttyd_INCS)
	$(CXX) -c -o $(webttyd_OBJ_DIR)/$@ $*.cpp $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

$(webttyd_C_OBJS): $(webttyd_INCS)
	$(CC) -c -o $(webttyd_OBJ_DIR)/$@ $*.c $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

$(webttyd_NAME): $(webttyd_OBJS)
	$(CXX) -o $(webttyd_NAME) $(addprefix $(webttyd_OBJ_DIR)/,$(webttyd_OBJS)) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)


### Target: clean ###
clean:
	@- $(RM) $(webttyd_NAME) $(webttyd_OBJ_DIR)/*.o


### Target: distclean ###
distclean: clean


### Target: install ###
install:
	@ cp $(webttyd_NAME) $(DESTDIR)


### Target: uninstall ###
uninstall:
	@ $(RM) $(DESTDIR)/$(webttyd_NAME)


### Target: help ###
help:
	@echo "Targets:"
	@echo "  all       - compile & build what is necessary"
	@echo "  install   - install to appropriate directories"
	@echo "  uninstall - uninstall from system"
	@echo "  clean     - cleanup objects & executables"
	@echo "  distclean - alias of \"clean\""
