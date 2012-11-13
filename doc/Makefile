LATEX=latex
BIBTEX=bibtex
DVIPS=dvips -Ppdf
DVIPDF=dvipdf
DVIPDF=dvipdfm
PDFLATEX=pdflatex --shell-escape

TEX-FILES = *.tex
FIG-FILES = *.eps
BIB-FILES = dbi.bib
PDF-FILES = empty.pdf

all: dbi.pdf

dbi.pdf:  ${TEX-FILES} ${BIB-FILES}
	$(PDFLATEX) dbi
	-$(BIBTEX) dbi
	$(PDFLATEX) dbi
	$(PDFLATEX) dbi
	$(PDFLATEX) dbi

clean:
	rm -f *.aux *.bbl *.blg *.log *.dvi *.out *.idx *.bak *~ *.toc *.lof
	rm -f dbi.pdf
