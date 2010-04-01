/***************************************************************************

 Copyright (C) 2008  Sergey Prokhorchuk

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

#ifndef __P3DCMDQUEUE_H__
#define __P3DCMDQUEUE_H__

class P3DEditCommand
 {
  public           :

  virtual         ~P3DEditCommand     () {};

  virtual void     Exec               () = 0;
  virtual void     Undo               () = 0;
 };

class P3DCmdRingBuffer
 {
  public           :

                   P3DCmdRingBuffer   (unsigned int        Size);
                  ~P3DCmdRingBuffer   ();

  unsigned int     Size               () const;

  void             Append             (P3DEditCommand     *Cmd);
  void             Cut                (unsigned int        NewSize);
  P3DEditCommand  *Get                (unsigned int        Index);

  private          :

  P3DEditCommand                     **Buffer;
  unsigned int                         BufferSize;
  unsigned int                         UsedSize;
  unsigned int                         Next;
 };

#define P3D_UNDO_MAX_DEPTH (1000)

class P3DEditCommandQueue
 {
  public           :

                   P3DEditCommandQueue(unsigned int        MaxDepth = P3D_UNDO_MAX_DEPTH);
                  ~P3DEditCommandQueue();

  void             PushAndExec        (P3DEditCommand     *Cmd);
  void             Undo               ();
  void             Redo               ();

  void             Clear              ();

  bool             UndoQueueEmpty     () const;
  bool             RedoQueueEmpty     () const;

  private          :

  void             Cut                (unsigned int        NewSize);

  P3DCmdRingBuffer                     CmdBuffer;
  unsigned int                         Next;
 };

#endif

