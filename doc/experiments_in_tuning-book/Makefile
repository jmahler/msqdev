
all: main.pdf

main.dvi: main.tex main.bib
	rubber $<

main.pdf: main.tex main.bib
	rubber --pdf $<

main.ps: main.tex main.bib
	rubber --ps $<

clean:
	-rm -f *.bbl *.toc *.aux *.blg *.pdf *.log *.dvi *.ps
