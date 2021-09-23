BINDIR=$(DESTDIR)/usr/bin
LIBEXECDIR=$(DESTDIR)/usr/libexec/man

all: makewhatis zsoelim

makewhatis: makewhatis.c
	$(CC) $(CFLAGS) -o $@ $(shell pkg-config --cflags --libs libbsd-overlay) -D_GNU_SOURCE $^

zsoelim: zsoelim.c
	$(CC) $(CFLAGS) -o $@ $(shell pkg-config --cflags --libs libbsd-overlay) $^ -D_XOPEN_SOURCE=500

clean:
	$(RM) makewhatis zsoelim

install: all
	install -d $(BINDIR)
	install makewhatis $(BINDIR)
	test \! -d $(BINDIR)/man
	install man.sh $(BINDIR)/man
	ln -sf man $(BINDIR)/manpath
	ln -sf man $(BINDIR)/apropos
	ln -sf man $(BINDIR)/whatis
	install -d $(LIBEXECDIR)
	install zsoelim $(LIBEXECDIR)
	install -d $(DESTDIR)/usr/share/man/man1
	install -m644 apropos.1 $(DESTDIR)/usr/share/man/man1
	install -m644 makewhatis.1 $(DESTDIR)/usr/share/man/man1
	install -m644 man.1 $(DESTDIR)/usr/share/man/man1
	install -m644 manpath.1 $(DESTDIR)/usr/share/man/man1
	ln -sf apropos.1 $(DESTDIR)/usr/share/man/man1/whatis.1
	install -d $(DESTDIR)/usr/share/man/man5
	install -m644 man.conf.5 $(DESTDIR)/usr/share/man/man5
