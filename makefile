
cc = gcc 

a.out: lex.yy.c syntax.tab.c const.o util.o ir.o sematics.o mips.o
	$(cc) syntax.tab.c lex.yy.c \
	   const.o util.o sematics.o ir.o mips.o

sematics.o: sematics.c compiler.h ir.h
	$(cc) -c sematics.c

util.o: util.c compiler.h
	$(cc) -c util.c

const.o: const.c
	$(cc) -c const.c

ir.o: ir.c ir.h
	$(cc) -c ir.c

mips.o: mips.c ir.h arch/mips32.h codeblk.c fr.c mips_debug.c
	$(cc) -c mips.c

lex.yy.c: clex.l 
	flex clex.l

syntax.tab.c: syntax.y
	bison -d syntax.y

lex:	lex.yy.c
	$(cc) -lfl lex.yy.c

run:
	./a.out text out.s


clean:
	rm *\.o
