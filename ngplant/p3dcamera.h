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

#ifndef __P3DCAMERA_H__
#define __P3DCAMERA_H__

#include <ngpcore/p3dmath.h>

class P3DViewport
 {
  public           :

                   P3DViewport        (int                 x,
                                       int                 y,
                                       unsigned int        w,
                                       unsigned int        h)
   {
    this->x = x; this->y = y; this->w = w; this->h = h; a = (float)w / h;
   }

  int              getX               () const
   {
    return(x);
   }

  int              getY               () const
   {
    return(y);
   }

  unsigned int     getWidth           () const
   {
    return(w);
   }

  unsigned int     getHeight          () const
   {
    return(h);
   }

  void             setOffset          (int                 x,
                                       int                 y)
   {
    this->x = x; this->y = y;
   }

  void             setSize            (unsigned int        w,
                                       unsigned int        h)
   {
    this->w = w; this->h = h; a = (float)w / h;
   }

  float            getAspect          (void) const /* return (width/height) */
   {
    return(a);
   }

  private          :

  int              x,y;
  unsigned int     w,h;
  float            a;
 };

class P3DCameraTrackBall
 {
  public           :

                   P3DCameraTrackBall ();

  void             GetTransformToCS   (float              *m) const;

  void             RotateCS           (float               a,
                                       float               x,
                                       float               y,
                                       float               z);

  void             RotateWS           (float               a,
                                       float               x,
                                       float               y,
                                       float               z);

  void             CenterMoveCS       (float               x,
                                       float               y,
                                       float               z);

  void             SetCenter          (float               x,
                                       float               y,
                                       float               z);

  void             SetDirection       (float               x,
                                       float               y,
                                       float               z,
                                       float               w);

  void             SetDistance        (float               distance);

  private          :

  P3DVector3f      center;
  P3DQuaternionf   dir;
  float            distance;
 };

#endif

