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

#include <stdlib.h>
#include <p3dcmdqueue.h>

                   P3DCmdRingBuffer::P3DCmdRingBuffer
                                      (unsigned int        Size)
 {
  Buffer = new P3DEditCommand*[Size];

  BufferSize = Size;
  UsedSize   = 0;
  Next       = 0;
 }

                   P3DCmdRingBuffer::~P3DCmdRingBuffer
                                      ()
 {
  Cut(0);

  delete[] Buffer;
 }

unsigned int       P3DCmdRingBuffer::Size
                                      () const
 {
  return(UsedSize);
 }

void               P3DCmdRingBuffer::Append
                                      (P3DEditCommand     *Cmd)
 {
  if (UsedSize == BufferSize)
   {
    delete Buffer[Next];

    UsedSize--;
   }

  Buffer[Next] = Cmd;

  UsedSize++;

  Next++;

  if (Next == BufferSize)
   {
    Next = 0;
   }
 }

void               P3DCmdRingBuffer::Cut
                                      (unsigned int        NewSize)
 {
  while (UsedSize > NewSize && Next > 0)
   {
    Next--;
    UsedSize--;

    delete Buffer[Next];
   }

  if (UsedSize > NewSize)
   {
    Next = BufferSize;

    while (UsedSize > NewSize)
     {
      Next--;
      UsedSize--;

      delete Buffer[Next];
     }
   }
 }

P3DEditCommand    *P3DCmdRingBuffer::Get
                                      (unsigned int        Index)
 {
  if (Index >= UsedSize)
   {
    return(NULL);
   }

  unsigned int     RIndex;

  RIndex = UsedSize - Index;

  if (RIndex <= Next)
   {
    return(Buffer[Next - RIndex]);
   }
  else
   {
    return(Buffer[BufferSize - (RIndex - Next)]);
   }
 }

                   P3DEditCommandQueue::P3DEditCommandQueue
                                      (unsigned int        MaxDepth)
                   : CmdBuffer(MaxDepth)
 {
  Next = 0;
 }

                   P3DEditCommandQueue::~P3DEditCommandQueue
                                      ()
 {
 }

void               P3DEditCommandQueue::PushAndExec
                                      (P3DEditCommand     *Cmd)
 {
  if (Next < CmdBuffer.Size())
   {
    Cut(Next);
   }

  Cmd->Exec();

  CmdBuffer.Append(Cmd);

  Next = CmdBuffer.Size();
 }

void               P3DEditCommandQueue::Undo
                                      ()
 {
  if (Next > 0)
   {
    Next--;

    CmdBuffer.Get(Next)->Undo();
   }
 }

void               P3DEditCommandQueue::Redo
                                      ()
 {
  if (Next < CmdBuffer.Size())
   {
    CmdBuffer.Get(Next)->Exec();

    Next++;
   }
 }

void               P3DEditCommandQueue::Clear
                                      ()
 {
  Cut(0);
 }

void               P3DEditCommandQueue::Cut
                                      (unsigned int        NewSize)
 {
  if (NewSize < CmdBuffer.Size())
   {
    CmdBuffer.Cut(NewSize);

    if (Next > NewSize)
     {
      Next = NewSize;
     }
   }
 }

bool               P3DEditCommandQueue::UndoQueueEmpty
                                      () const
 {
  return(Next == 0);
 }

bool               P3DEditCommandQueue::RedoQueueEmpty
                                      () const
 {
  return(Next == CmdBuffer.Size());
 }

