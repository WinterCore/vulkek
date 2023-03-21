CFLAGS = -std=c11 -Wall -Wextra -ggdb
LDFLAGS = -lglfw -lm -lcglm -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

Run: src/main.c
	cc $(CFLAGS) -o Run src/main.c $(LDFLAGS)

.PHONY: test clean

test: Run
	./Run

clean:
	rm -f Run
