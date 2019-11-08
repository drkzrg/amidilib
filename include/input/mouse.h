#ifndef MOUSE_H_
#define MOUSE_H_
 
  void disableMouse(void);
  void enableMouse(void);
  void  mouseInit(void);
  void  mouseDeinit(void);
  void	mouseSpriteRestore(void);
  void  mouseSpriteDraw(void *logScrn,uint16 mx,uint16 my);
  void  mouseSwapBuffers(void);

#endif