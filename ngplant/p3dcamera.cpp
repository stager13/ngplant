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

#include <ngpcore/p3dmath.h>

#include <p3dcamera.h>

                   P3DCameraTrackBall::P3DCameraTrackBall
                                      () :
                   center(0.0f,0.0f,0.0f),
                   dir(0.0f,0.0f,0.0f,1.0f)
 {
  distance = -30.0;
 }

void               P3DCameraTrackBall::SetCenter
                                      (float               x,
                                       float               y,
                                       float               z)
 {
  center.X() = x;
  center.Y() = y;
  center.Z() = z;
 }

void               P3DCameraTrackBall::SetDirection
                                      (float               x,
                                       float               y,
                                       float               z,
                                       float               w)
 {
  dir.q[0] = x;
  dir.q[1] = y;
  dir.q[2] = z;
  dir.q[3] = w;
 }

void               P3DCameraTrackBall::SetDistance
                                      (float               distance)
 {
  this->distance = -distance;
 }

void               P3DCameraTrackBall::GetTransformToCS
                                      (float              *m) const
 {
  P3DMatrix4x4f                        tz_m;
  P3DMatrix4x4f                        rot_m;
  P3DMatrix4x4f                        temp_m;

  P3DMatrix4x4f::MakeTranslation(tz_m.m,0.0,0.0,distance);

  dir.ToMatrix(rot_m.m);

  P3DMatrix4x4f::MultMatrix(temp_m.m,tz_m.m,rot_m.m);

  P3DMatrix4x4f::Translate(m,temp_m.m,center.v[0],center.v[1],center.v[2]);
 }

void               P3DCameraTrackBall::RotateCS
                                      (float               a,
                                       float               x,
                                       float               y,
                                       float               z)
 {
  P3DQuaternionf                       t_q(dir);
  P3DQuaternionf                       r_q;

  r_q.FromAxisAndAngle(x,y,z,a);

  P3DQuaternionf::CrossProduct(dir.q,r_q.q,t_q.q);

  dir.Normalize();
 }

void               P3DCameraTrackBall::RotateWS
                                      (float               a,
                                       float               x,
                                       float               y,
                                       float               z)
 {
  P3DVector3f                          v(x,y,z);

  P3DQuaternionf::RotateVector(v.v,dir.q);

  RotateCS(a,v.X(),v.Y(),v.Z());
 }

void               P3DCameraTrackBall::CenterMoveCS
                                      (float               x,
                                       float               y,
                                       float               z)
 {
  P3DVector3f                          v(x,y,z);

  P3DQuaternionf::RotateVectorInv(v.v,dir.q);

  center += v;
 }

