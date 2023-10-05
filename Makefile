.POSIX:
.SUFFIXES:
OUTDIR=.build
include $(OUTDIR)/config.mk
include $(OUTDIR)/cppcache

seamus: $(seamus_objects)
	@printf 'CCLD\t$@\n'
	@$(CC) $(LDFLAGS) -o $@ $(seamus_objects) $(LIBS)

.SUFFIXES: .c .o

.c.o:
	@printf 'CC\t$@\n'
	@touch $(OUTDIR)/cppcache
	@grep $< $(OUTDIR)/cppcache >/dev/null || \
		$(CPP) $(CFLAGS) -MM -MT $@ $< >> $(OUTDIR)/cppcache
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -f seamus $(seamus_objects)

distclean: clean
	@rm -rf "$(OUTDIR)"

install: all
	mkdir -p \
		$(DESTDIR)$(BINDIR) \
		$(DESTDIR)$(SHAREDIR)/seamus
	install -m755 seamus $(DESTDIR)$(BINDIR)/seamus

.PHONY: clean distclean install
