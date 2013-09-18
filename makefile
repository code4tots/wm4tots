all: wm4tots

wm4tots:
	gcc wm4tots.c -lX11 -o wm4tots

install: wm4tots
	mv wm4tots $(HOME)/programs/wm4tots/

clean:
	rm wm4tots