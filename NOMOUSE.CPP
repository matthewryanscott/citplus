// --------------------------------------------------------------------------
// Citadel: NoMouse.CPP
//
// Can you have a mouse and no mouse at the same time?

#ifdef WINCIT
// these are stubs for what would be in mouse.asm if this were a DOS version

int hideCounter;

void initMouseHandler(void) {}
void deinitMouseHandler(void) {}
void showMouse(void) {}
void hideMouse(void) {}
void setMousePos(int, int) {}

#endif

