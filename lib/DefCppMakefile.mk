ROOT = ../..
BUILDL = $(BUILD)/lib/$(NAME)
SUBDIRS = $(filter-out Makefile $(shell find . -name "*.*"),$(shell find . -name "*"))
BUILDDIRS = $(addprefix $(BUILDL)/,$(SUBDIRS))
DEPS = $(shell find $(BUILDDIRS) -name "*.d")
STLIB = $(BUILD)/lib$(NAME).a
DYNLIBNAME = lib$(NAME).so
DYNLIB = $(BUILD)/$(DYNLIBNAME)

CFLAGS = $(CPPDEFFLAGS) $(ADDFLAGS)

# sources
CSRC = $(shell find . -name "*.cpp")

# objects
COBJS = $(patsubst %.cpp,$(BUILDL)/%.o,$(CSRC))
CPICOBJS = $(patsubst %.cpp,$(BUILDL)/%_pic.o,$(CSRC))

.PHONY:		all clean

all: $(BUILDDIRS) $(STLIB) $(DYNLIB)

$(STLIB): $(COBJS)
		@echo "	" AR $(STLIB)
		@ar rcs $(STLIB) $(COBJS)
		$(ROOT)/tools/linklib.sh $(STLIB)

$(DYNLIB): $(CPICOBJS)
		@echo "	" LINKING $(DYNLIB)
		@$(CPPC) $(CFLAGS) -shared -Wl,-shared -Wl,-soname,$(DYNLIBNAME) -o $(DYNLIB) \
			$(CPICOBJS) $(ADDLIBS)
		$(ROOT)/tools/linklib.sh $(DYNLIB)

$(BUILDDIRS):
		@for i in $(BUILDDIRS); do \
			if [ ! -d $$i ]; then mkdir -p $$i; fi \
		done;

$(BUILDL)/%.o: %.cpp
		@echo "	" CC $<
		@$(CPPC) $(CFLAGS) -o $@ -c $< -MD

$(BUILDL)/%_pic.o: %.cpp
		@echo "	" CC $<
		@$(CPPC) $(CFLAGS) -fPIC -o $@ -c $< -MD

-include $(DEPS)

clean:
		@echo "===== REMOVING FILES =====";
		rm -f $(COBJS) $(CPICOBJS) $(STLIB) $(DYNLIB) $(DEPS)
