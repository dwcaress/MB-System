#ifndef MB_COLOR_H
#define MB_COLOR_H
/* **
This header does not contain window-system specific references, 
as it is included by both X11 and Qt applications.
E.g. Do NOT reference X11 headers or structures within this file.
*/

/// Standard mb-system drawing colors
typedef enum {
  WHITE = 0,
  BLACK,
  RED,
  GREEN,
  BLUE,
  ORANGE,
  PURPLE,
  CORAL,
  LIGHTGREY,
  
  NDrawingColors

} DrawingColor;


typedef enum {
  SOLID_LINE = 0,
  DASH_LINE
  
} LineStyle;

#endif
