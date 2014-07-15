CFLAGS = -std=c99
.PHONY: clean

vm: vm.c
	gcc $(CFLAGS) vm.c -o vm

clean:
	rm -rf vm *.o
