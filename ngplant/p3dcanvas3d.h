/***************************************************************************

 Copyright (C) 2006  Sergey Prokhorchuk

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#ifndef __P3DCANVAS3D_H__
#define __P3DCANVAS3D_H__

#include <wx/wx.h>
#include <ngput/p3dglext.h>
#include <wx/glcanvas.h>

#include <ngpcore/p3dmathrng.h>
#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dplant.h>

#include <p3dcamera.h>

class P3DCanvas3D : public wxGLCanvas
 {
  public           :

                   P3DCanvas3D        (wxWindow           *parent);
  virtual         ~P3DCanvas3D        ();

  void             ForceRefresh       ();

  void             OnPaint            (wxPaintEvent       &event);
  void             OnEraseBackground  (wxEraseEvent       &event);
  void             OnSize             (wxSizeEvent        &event);
  void             OnMouseMotion      (wxMouseEvent       &event);
  void             OnMouseWheel       (wxMouseEvent       &event);
  void             OnKeyDown          (wxKeyEvent         &event);
  void             OnKeyUp            (wxKeyEvent         &event);

  void             OnMouseEnter       (wxMouseEvent       &event);

  bool             IsGLExtInited      () const
   {
    return(GLExtInited);
   }

  private          :

  void             Render             ();

  P3DViewport                          viewport;
  P3DCameraTrackBall                   camera;
  float                                ZoomFactor;
  long                                 m_x;
  long                                 m_y;

  bool                                 OrthoCamera;

  bool                                 HighlightSelection;

  bool                                 GLExtInited;

  DECLARE_EVENT_TABLE()
 };

#endif

