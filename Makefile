BINDIR=$(DESTDIR)/usr/bin

all:

install:
	install -d $(BINDIR)
	install -T man.sh $(BINDIR)/man
	ln -sf man $(BINDIR)/manpath
	ln -sf man $(BINDIR)/apropos
	ln -sf man $(BINDIR)/whatis
	install -d $(DESTDIR)/usr/share/man/man1
	install -m644 man.1 $(DESTDIR)/usr/share/man/man1
	install -m644 manpath.1 $(DESTDIR)/usr/share/man/man1
	install -d $(DESTDIR)/usr/share/man/man5
	install -m644 man.conf.5 $(DESTDIR)/usr/share/man/man5
