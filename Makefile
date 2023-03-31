CFLAGS = -std=c11 -Wall -Wextra
LDFLAGS = -lglfw -lm -lcglm -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS_DIR = src/shaders

.PHONY: test clean debug all executable

all: executable

debug: CFLAGS += -ggdb
debug: executable

release: CFLAGS += -DNDEBUG
release: executable

executable: Run
	./Run

src/aids.o: src/aids.c src/aids.h
	cc $(CFLAGS) -c -o src/aids.o $<

# Shaders
$(SHADERS_DIR)/vert.spv: $(SHADERS_DIR)/shader.vert
	glslc $< -o $@

$(SHADERS_DIR)/frag.spv: $(SHADERS_DIR)/shader.frag
	glslc $< -o $@

SHADER_FILES = $(SHADERS_DIR)/vert.spv $(SHADERS_DIR)/frag.spv

clean:
	rm -rf Run ./src/shaders/*.spv ./src/*.o

Run: src/main.c src/aids.o $(SHADER_FILES)
	cc $(CFLAGS) -o Run src/main.c $(LDFLAGS)
