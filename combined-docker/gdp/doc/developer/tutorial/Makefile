# Needs pandoc to be installed. 
#

ALL_HTML=	gdp-tutorial-part1.html gdp-tutorial-part2.html
ALL_PDF=	gdp-tutorial-part1.pdf gdp-tutorial-part2.pdf
CSS=		pandoc.css

#all: html pdf
all: html

html: gdp-tutorial-part1.html gdp-tutorial-part2.html

pdf: gdp-tutorial-part1.pdf gdp-tutorial-part2.pdf

${ALL_HTML} ${ALL_PDF}: ${CSS}

.SUFFIXES: .md .html .pdf

PANDOC=		pandoc
PANFLAGS=	-s -c ${CSS} --self-contained --toc --toc-depth=5

.md.html:
	${PANDOC} ${PANFLAGS} -o $@ $<

.md.pdf:
	${PANDOC} ${PANFLAGS} -o $@ $<
