sim: shell.c sim.c decode_core.c instructions_functions.c opcodes.c
	gcc -g -O0 $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o *~ sim
