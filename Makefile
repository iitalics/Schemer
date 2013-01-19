all: schemer
obj/Token.o: Token.cpp
	g++ -Wall --std=c++0x Token.cpp -c -o obj/Token.o
obj/argvments.o: argvments.c
	gcc -Wall --std=c++0x argvments.c -c -o obj/argvments.o
obj/Parser.o: Parser.cpp
	g++ -Wall --std=c++0x Parser.cpp -c -o obj/Parser.o
obj/NativeFunctions.o: NativeFunctions.cpp
	g++ -Wall --std=c++0x NativeFunctions.cpp -c -o obj/NativeFunctions.o
obj/Interpreter.o: Interpreter.cpp
	g++ -Wall --std=c++0x Interpreter.cpp -c -o obj/Interpreter.o
obj/Value.o: Value.cpp
	g++ -Wall --std=c++0x Value.cpp -c -o obj/Value.o
obj/main.o: main.cpp
	g++ -Wall --std=c++0x main.cpp -c -o obj/main.o
schemer: obj/Token.o obj/argvments.o obj/Parser.o obj/NativeFunctions.o obj/Interpreter.o obj/Value.o obj/main.o
	g++ obj/Token.o obj/argvments.o obj/Parser.o obj/NativeFunctions.o obj/Interpreter.o obj/Value.o obj/main.o  --std=c++0x -o schemer
clean:
	rm -f obj/* schemer
rebuild: clean all
