all: l1solution
l1solution: l1solution.c	
	gcc -Wall -fsanitize=address,undefined -o l1solution l1solution.c
.PHONY: clean all
clean:
	rm l1solution