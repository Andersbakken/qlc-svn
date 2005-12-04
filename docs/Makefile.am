install-data:
	if [ ! -d $(prefix)/$(DOCUMENTS) ]; then mkdir -p $(prefix)/$(DOCUMENTS); fi
	if [ ! -d $(prefix)/$(DOCUMENTS)/$(PACKAGE) ]; then mkdir -p $(prefix)/$(DOCUMENTS)/$(PACKAGE); fi
	install -o root -g root -m 644 *.html *.css $(prefix)/$(DOCUMENTS)/$(PACKAGE)
	
	if [ ! -d $(prefix)/$(DOCUMENTS)/$(PACKAGE)/pics ]; then mkdir -p $(prefix)/$(DOCUMENTS)/$(PACKAGE)/pics; fi
	install -o root -g root -m 644 pics/*.* $(prefix)/$(DOCUMENTS)/$(PACKAGE)/pics
