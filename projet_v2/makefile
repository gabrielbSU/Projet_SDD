main: main.o parse.o hachage.o dynamique.o
	gcc -Wall -o main main.o parse.o hachage.o dynamique.o

main.o: main.c
	gcc -Wall -c main.c -o main.o

parse.o: parse.c
	gcc -Wall -c parse.c -o parse.o

hachage.o: hachage.c
	gcc -Wall -c hachage.c -o hachage.o

dynamique.o: dynamique.c
	gcc -Wall -c dynamique.c -o dynamique.o

clean:
	rm -f *.o main