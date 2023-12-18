#include <X11/extensions/XKB.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

Atom wm_name;
Atom string;

void toUpper(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] -= 32;
  }
}

void printGroup(Display *dpy, int grp, int caps_state, unsigned int mods) {
  char *group;
  XkbRF_VarDefsRec vd;
  XkbRF_GetNamesProp(dpy, NULL, &vd);
		
  group = strtok(vd.layout, ",");
  for (int i = 0; i < grp; i++) {
    group = strtok(NULL, ",");
    if (group == NULL) {
      fprintf(stderr, "Group out of bounds: %d\n", grp);
      return;
    }
  }

  register int i;
  unsigned bit;

  for (i = XkbNumModifiers - 1, bit = 0x80; i >= 0; i--, bit >>= 1) {
    if (mods & bit) {
      switch (i) {
      case 6:
        fprintf(stdout, "󰘳 ");
        break;
      case 3:
        fprintf(stdout, "󰘵 ");
        break;
      case 2:
        fprintf(stdout, "󰘴 ");
        break;
      case 0:
        fprintf(stdout, "󰘶 ");
        break;
      }
    }
  }

  if (caps_state) {
    toUpper(group);
  }

  fprintf(stdout, "%s\n", group);
  fflush(stdout);
}

int main(int argc, char **argv) {
  Display *dpy = XOpenDisplay(NULL);

  if (dpy == NULL) {
    fprintf(stdout, "The hell is going on?\n");
    fprintf(stderr, "Cannot open the display\n");
    return 1;
  }

  wm_name = XInternAtom(dpy, "WM_NAME", 0);
  string = XInternAtom(dpy, "STRING", 0);

  XkbStateRec state;
  XkbGetState(dpy, XkbUseCoreKbd, &state);
  int lastlang = state.group;
  unsigned int lastmods = state.mods;

  unsigned int caps_state;
  XkbGetIndicatorState(dpy, XkbUseCoreKbd, &caps_state);
  unsigned int last_caps_state = (caps_state & 0x01) == 1;

  printGroup(dpy, lastlang, last_caps_state, lastmods);

  int opcode, xkbEventType, error, major, minor;
  XkbQueryExtension(dpy, &opcode, &xkbEventType, &error, &major, &minor);
  XkbSelectEventDetails(dpy, XkbUseCoreKbd, XkbStateNotify, XkbAllStateComponentsMask, XkbAllStateComponentsMask);

  XEvent event;
  while (1) {
    XNextEvent(dpy, &event);
    if (event.type == xkbEventType) {
      XkbEvent *xkbe = (XkbEvent*)&event;
      if (xkbe->any.xkb_type == XkbStateNotify) {
        XkbGetIndicatorState(dpy, XkbUseCoreKbd, &caps_state);
        caps_state = (caps_state & 0x01) == 1;
              
        if (xkbe->state.group != lastlang || last_caps_state != caps_state || xkbe->state.mods != lastmods) {
          lastlang = xkbe->state.group;
          last_caps_state = caps_state;
          lastmods = xkbe->state.mods;
          printGroup(dpy, lastlang, last_caps_state, lastmods);
        }
      }
    }
  }
	
  return 0;
}
