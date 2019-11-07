#ifndef MOUSE_H_
#define MOUSE_H_
 
  void disableMouse(void);
  void enableMouse(void);
  void  mouseInit(void);
  void  mouseDeinit(void);
  void	mouseSpriteRestore(void);
  void  mouseSpriteDraw(void *logScrn,U16 mx,U16 my);
  void  mouseSwapBuffers(void);

#endif