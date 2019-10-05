dragonshell:
	g++ dragonshell.cc -o dragonshell
compile:
	 g++ -Wall -g -c dragonshell.cc -o dragonshell.o
clean:
	rm dragonshell
	rm dragonshell.o

