LIB_CLEANS := $(addsuffix /clean, $(LIBS))

.PHONY: all clean $(LIBS) $(LIB_CLEANS)

all: $(LIBS)

clean: $(LIB_CLEANS)

$(LIBS):
	$(MAKE) -C$@

$(LIB_CLEANS):
	$(MAKE) clean -C$(dir $@)
