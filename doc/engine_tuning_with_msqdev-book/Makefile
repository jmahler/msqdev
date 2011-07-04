
DEPS=appA-ve_tuner.tex

all: main.pdf

appA-ve_tuner.tex: appA-ve_tuner.tex.php analyze/ve_tuner.R
	php -f $< > $@

main.dvi: main.tex main.bib $(DEPS)
	rubber $<

main.pdf: main.tex main.bib $(DEPS)
	rubber --pdf $<

main.ps: main.tex main.bib $(DEPS)
	rubber --ps $<

clean:
	-rm -f *.bbl *.toc *.aux *.blg *.log
	-rm -f main.pdf
	-rm -f main.ps
	-rm -f main.dvi

