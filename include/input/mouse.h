#ifndef __MOUSE_H__
#define __MOUSE_H__
 
  void disableMouse();
  void enableMouse();
 
  void  mouseInit();
  void  mouseDeinit();
  void	mouseSpriteRestore();
  void  mouseSpriteDraw(void *logScrn,U16 mx,U16 my);
  void  mouseSwapBuffers();

#endif