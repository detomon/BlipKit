DOC_CSS_DIR = doc/css

doc: Doxyfile $(DOC_CSS_DIR)/*.css html
	sass $(DOC_CSS_DIR)/main.scss > $(DOC_CSS_DIR)/main.css
	doxygen

SUBDIRS = src examples test
DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = \
	README.md \
	LICENSE \
	VERSION \
	LOGO \
	autogen.sh
