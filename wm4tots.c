/*
 * wm4tots.c
 * 
 * Tue Sep 17 23:19:04 PDT 2013
 *
 * Written by math4tots.
 * Inspired by TinyWM (https://github.com/mackstann/tinywm)
 * 
 * Just a minimal window manager, with features added as I want
 * them.
 *
 * In the spirit of TinyWM, I will try to make this code as readable as
 * possible for n00bies.
 */


#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX(a,b) ((a)>(b)?(a):(b))

/* ---------------- Global variables ------------------- */
/*
 * Bad idea? Probably. But I really don't intend for this program to grow
 * beyond this file.
 *
 * "EH, SCREW GOOD PRACTICE. HOW BAD CAN IT BE?"
 *
 * It's ok, velociraptors aren't real.
 */
int argc;
char** argv;
Display * dpy;
Window root;
XEvent ev;
XWindowAttributes attr;
XButtonEvent start; /* use this to save state during move/resize */


/* ---------------- function prototypes ----------------- */
void give_up(char* last_words);
void leave_a_message(char* message);
int  user_asked_for_help();
void print_help_message();
void ask_for_a_display();
void ask_to_intercept_messages();
void handle_events();


/* ---------------- main -------------------------------- */
int main(int argc_local, char** argv_local) {
  argc = argc_local;
  argv = argv_local;

  if (user_asked_for_help()) print_help_message();

  else {
    ask_for_a_display();
    ask_to_intercept_messages();
    handle_events();
  }
  return 0;
}

/* ---------------- function definitions ---------------- */
void give_up(char* last_words) {
  /*
   * Sometimes there is nothin a poor window manager can do but give up.
   */
  leave_a_message(last_words);
  exit(1);
}

void leave_a_message(char* message) {
  /*
   * Since we don't have a terminal, we can't always see what's up now can we?
   * So instead we leave a message in a file.
   * Right now it's pretty much hardcoded. Might fix this later.
   */
  FILE * fout = fopen("$HOME/.config/wm4tots/log.txt","a");
  fprintf(fout, "%s\n", message);
  fclose(fout);
}

int user_asked_for_help() {
  /*
   * We say that the user asked for help if any of the arguments is "help",
   * "-help", "--help", or "-h".
   */

  char * help[] = {"help", "-help", "--help", "-h"};
  int i, j, asked, num_help;

  num_help = sizeof(help) / sizeof(char*);
  asked = 0;

  for(i = 0; (i < argc) && !asked; i++) {
    for (j = 0; (j < num_help) && !asked; j++)
      asked = (strcmp(argv[i],help[j]) == 0);
  }

  return asked;
}

void print_help_message() {
  /*
   * Explain to the user how to use this program!
   * Right now I'm too lazy to actually put in a helpful message, but
   * ideally it should talk about how the order of arguments are important
   * (because well, I'm too lazy to parse arguments, and using libraries might
   * make be less portable), that if any of the arguments are a "help" signal,
   * you will get this message, and a nice usage message explaining the order
   * of the arguments.
   */

  printf("User has asked for help!\n");
  printf("Usage: %s [display]\n", argv[0]);
}

void ask_for_a_display() {
  /*
   * We need a display upon which we may display all the windows.
   * If we ask nicely, X might just give us one to play with.
   * If X doesn't give us a display, then we can do little but give up.
   */
  dpy = XOpenDisplay( (argc>1) ? argv[1] : 0 );

  if (dpy == 0)
    give_up("X wouldn't give me a display!");

  root = DefaultRootWindow(dpy);
}

void ask_to_intercept_messages() {
  /*
   * In order to know what's going on, X has to tell you what's going on.
   * Well there are so many XEvents, that X will only tell you if you ask.
   * Here we ask X to let us know when certain things happen.
   */

  /* Alt + F1 */
  XGrabKey(dpy,
	   XKeysymToKeycode(dpy, XStringToKeysym("F1")),
	   Mod1Mask,
	   root,
	   True,
	   GrabModeAsync,
	   GrabModeAsync);
  /* Alt + tab */
  XGrabKey(dpy,
	   XKeysymToKeycode(dpy, XStringToKeysym("tab")),
	   Mod1Mask,
	   root,
	   True,
	   GrabModeAsync
	   ,GrabModeAsync);

  /* Alt + q */
  XGrabKey(dpy,
	   XKeysymToKeycode(dpy, XStringToKeysym("q")),
	   Mod1Mask,
	   root,
	   True,
	   GrabModeAsync
	   ,GrabModeAsync);

  /* Alt + left mouse click (mouse click 1) */
  /*
   * We want to be messaged when
   * 1) pressed
   * 2) released
   * 3) mouse moves around while Alt + left click is pressed
   * (Mod1Mask seems to mean Alt)
   */
  XGrabButton(dpy,
	      1,
	      Mod1Mask,
	      root,
	      True,
	      ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
	      GrabModeAsync,
	      GrabModeAsync,
	      None,
	      None);

  /* Alt + right mouse click (mouse click 3) */
  XGrabButton(dpy,
	      3,
	      Mod1Mask,
	      root,
	      True,
	      ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
	      GrabModeAsync,
	      GrabModeAsync,
	      None,
	      None);
}


void handle_events() {
  /*
   * Here we wait for an event, then handle it,
             wait for an event, then handle it,
	     wait for an event, then handle it,
	     ...
   * 5EVER
   *
   * Supposedly there are more flexible and sophisticated ways of doing this,
   * but for right now, I'm gonna do it the tinywm way.
   */

  int xdiff, ydiff;

  start.subwindow = None;
  for(;;) {

    XNextEvent(dpy,&ev);
    
    if (ev.type == KeyPress && ev.xkey.subwindow != None) {

      XRaiseWindow(dpy,ev.xkey.subwindow);

      /* if (ev.xkey.keycode == q) { */
	/*
	 * I was hoping I could get window to exit here, but apparently this
	 * is not necessarily native to X...
	 * I need to read up on ICCCM and EWMH
	 */
      /*      } else if (ev.xkey.keycode == tab) { */
	/*
	 * TODO
	 * cycle through windows 
	 */
      /*      } else if (ev.xkey.keycode == f1) { */
      /*      } */

    }


    else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
      /*
       * Due to exactly the messages we asked for, we know if we get here,
       * that a window was alt-clicked. In this case, we want to raise the
       * window to the top, and then save the dimensions in case we are
       * trying to move/resize.
       */
      XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
      start = ev.xbutton;
    }

    else if (ev.type == MotionNotify && start.subwindow != None) {
      /*
       * If we are here, then, we are alt-clicking and dragging.
       * We use XCheckTypedEvent to skip to the most recent drag event,
       * then we move and resize the window accordingly.
       */
      while (XCheckTypedEvent(dpy,MotionNotify,&ev));
      xdiff = ev.xbutton.x_root - start.x_root;
      ydiff = ev.xbutton.y_root - start.y_root;
      XMoveResizeWindow(dpy,
			start.subwindow,
			attr.x + (start.button==1 ? xdiff : 0),
			attr.y + (start.button==1 ? ydiff : 0),
			MAX(1,attr.width + (start.button==3 ? xdiff : 0)),
			MAX(1,attr.height + (start.button==3 ? ydiff : 0)));
    }
    else if (ev.type == ButtonRelease) {
      start.subwindow = None;
    }
  }
}
