install-data-local:
	if [ ! -d $(datadir)/$(FIXTURES) ]; then mkdir -p $(datadir)/$(FIXTURES); fi

	chmod 755 $(datadir)/$(FIXTURES)

	install -o root -g root -m 644 *.deviceclass $(datadir)/$(FIXTURES)
