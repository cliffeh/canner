CAN=../src/can
STATIC_DIR=static
STATIC_FILES=$(shell find $(STATIC_DIR) -type f)

all: site site-static

site: site.c
	$(CC) -o $@ $< -levent

site-static: site.c
	$(CC) -o $@ $< -Wl,-Bstatic -levent -Wl,-Bdynamic

site.c: $(CAN) $(STATIC_FILES)
	$(CAN) $(STATIC_DIR) > $@

$(CAN):
	$(MAKE) -C ../src can

clean:
	rm -f site.c

realclean: clean
	rm -f site site-static

distclean: realclean
