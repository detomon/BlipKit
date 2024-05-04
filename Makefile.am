DOCS_DIR = docs
SUBDIRS = src examples test dev/step_phases dev/tone_periods
DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = \
	README.md \
	LICENSE \
	VERSION \
	LOGO \
	autogen.sh \
	Doxygen \
	ChangeLog \
	CMakeLists.txt \
	docs

docs: Doxyfile
	doxygen

clean-local:
	-rm -rf $(DOCS_DIR)
