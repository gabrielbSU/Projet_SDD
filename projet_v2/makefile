main: main.o parser.o hachage.o dynamique.o
	gcc -Wall -o main main.o parser.o hachage.o dynamique.o

main.o: main.c
	gcc -Wall -c main.c -o main.o

parser.o: parser.c
	gcc -Wall -c parser.c -o parser.o

hachage.o: hachage.c
	gcc -Wall -c hachage.c -o hachage.o

dynamique.o: dynamique.c
	gcc -Wall -c dynamique.c -o dynamique.o

clean:
	rm -f *.o main