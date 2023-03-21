CFLAGS = -std=c11 -Wall -Wextra
LDFLAGS = -lglfw -lm -lcglm -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

.PHONY: test clean debug all executable

all: executable

debug: CFLAGS += -DDEBUG -ggdb
debug: executable

executable: Run
	./Run

clean:
	rm -f Run

Run: src/main.c
	cc $(CFLAGS) -o Run src/main.c $(LDFLAGS)
