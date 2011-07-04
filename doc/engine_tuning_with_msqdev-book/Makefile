
all: main.pdf

appA-afr_predict.tex: appA-afr_predict.tex.php analyze/afr_predict.R
	php -f $< > $@

main.dvi: main.tex main.bib appA-afr_predict.tex
	rubber $<

main.pdf: main.tex main.bib appA-afr_predict.tex
	rubber --pdf $<

main.ps: main.tex main.bib appA-afr_predict.tex
	rubber --ps $<

clean:
	-rm -f *.bbl *.toc *.aux *.blg *.log
	-rm -f main.pdf
	-rm -f main.ps
	-rm -f main.dvi

