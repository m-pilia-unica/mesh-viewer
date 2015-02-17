all:
	if [ ! -e ./bin ]; then; mkdir bin; fi
	gcc -o ./bin/viewer main.c components.c -lGL -lGLU -lglut -lm

debug:
	if [ ! -e ./bin ]; then; mkdir bin; fi
	gcc -o ./bin/viewer main.c components.c -lGL -lGLU -lglut -lm -D __DEBUG__

doc:
	doxygen Doxyfile

clean:
	rm -rf ./doc ./bin/*
