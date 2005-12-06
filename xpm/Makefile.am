install-data-local:
	if [ ! -d $(datadir)/$(PIXMAPS) ]; then mkdir -p $(datadir)/$(PIXMAPS); fi

	chmod 755 $(datadir)/$(PIXMAPS)

	install -o root -g root -m 644 *.xpm *.png $(datadir)/$(PIXMAPS)
