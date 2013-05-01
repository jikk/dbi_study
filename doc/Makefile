LATEX=latex
BIBTEX=bibtex
DVIPS=dvips -Ppdf
DVIPDF=dvipdf
DVIPDF=dvipdfm
PDFLATEX=pdflatex --shell-escape
GNUPLOT=gnuplot

TEX-FILES = *.tex
FIG-FILES = *.eps
BIB-FILES = dbi.bib
PDF-FILES = empty.pdf
GRH-FILES = bzip_perf.plot

all: clean graph dbi.pdf

graph: 
	gnuplot bzip_perf.plot

dbi.pdf:  ${TEX-FILES} ${BIB-FILES}
	$(PDFLATEX) dbi
	-$(BIBTEX) dbi
	$(PDFLATEX) dbi
	$(PDFLATEX) dbi
	$(PDFLATEX) dbi

clean:
	rm -f *.aux *.bbl *.blg *.log *.dvi *.out *.idx *.bak *~ *.toc *.lof
	rm -f dbi.pdf
