
APP_NAME = Renderer
APP_VER_MAJOR = 0
APP_VER_MINOR = 1
APP_VER_BUILD = 292


DEFINES = -DAPP_NAME=\"$(APP_NAME)\" -DAPP_VER_MAJOR=$(APP_VER_MAJOR) -DAPP_VER_MINOR=$(APP_VER_MINOR) -DAPP_VER_BUILD=$(APP_VER_BUILD)

CFLAGS = -g3 -Wall -Wextra -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -pthread -lpthread
#-pg -no-pie #Enable profiling data out gmon.out

EMSFLAGS = -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -pthread -sPTHREAD_POOL_SIZE=8 

# Increment APP_VER_BUILD number by 1
increment_version:
	$(eval APP_VER_BUILD := $(shell echo $$(($(APP_VER_BUILD) + 1))))
	@sed -i 's/^\(APP_VER_BUILD = \).*/\1$(APP_VER_BUILD)/' Makefile
	$(shell touch src\application.c)



all: application.exe emscripten

emscripten: build\application_em.o build\window_em.o build\simulation_em.o
	C:\Users\dwtys\emsdk\upstream\emscripten\emcc $(CFLAGS) -sASSERTIONS -sSTACK_SIZE=1048576 --emrun -Wextra build\window_em.o build\simulation_em.o build\application_em.o -o application.js  $(EMSFLAGS) --preload-file Resources -sEXPORTED_RUNTIME_METHODS=cwrap -sTOTAL_MEMORY=536870912 

build\application_em.o: src\application.c
	C:\Users\dwtys\emsdk\upstream\emscripten\emcc $(CFLAGS) -c src\application.c -o build\application_em.o $(EMSFLAGS)  $(DEFINES)

build\window_em.o: src\window.c
	C:\Users\dwtys\emsdk\upstream\emscripten\emcc $(CFLAGS) -c src\window.c -o build\window_em.o  $(EMSFLAGS) -mavx -msimd128


run: application.exe
	build\$(APP_NAME).exe

application.exe: increment_version build\application.o build\window.o
	gcc $(CFLAGS) -o build\$(APP_NAME).exe build\window.o build\application.o -L.\dependencies\lib -lgdi32 -lSDL2 -lSDL2_ttf -lm -lSDL2_image

build\application.o: src\application.c
	gcc $(CFLAGS) -c src\application.c -o build\application.o $(DEFINES)

build\window.o: src\window.c
	gcc $(CFLAGS) -c src\window.c -o build\window.o -mavx


clean: 
	cmd //C del build\application.o build\window.o build\application_em.o build\window_em.o build\$(APP_NAME).exe gmon.out
