all:	petsciishooter.prg

run:	petsciishooter.prg
	x64 petsciishooter.prg

clean:
	-rm petsciishooter.prg
	-rm petsciishooter.lst
	-rm petsciishooter.lbl
	-rm petsciishooter.o

petsciishooter.prg petsciishooter.lst petsciishooter.lbl petsciishooter.o: petsciishooter.c
	cl65 -t c64 -Ln petsciishooter.lbl -Osir -Cl --listing petsciishooter.lst --add-source -o petsciishooter.prg petsciishooter.c