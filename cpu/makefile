main: main.o parser.o hachage.o dynamique.o cpu.o
	gcc -Wall -o main main.o parser.o hachage.o dynamique.o cpu.o

main.o: main.c logger.h
	gcc -Wall -c main.c -o main.o

parser.o: parser.c parser.h logger.h
	gcc -Wall -c parser.c -o parser.o

hachage.o: hachage.c hachage.h logger.h
	gcc -Wall -c hachage.c -o hachage.o

dynamique.o: dynamique.c dynamique.h logger.h
	gcc -Wall -c dynamique.c -o dynamique.o

cpu.o: cpu.c cpu.h logger.h
	gcc -Wall -c cpu.c -o cpu.o

clean:
	rm -f *.o main