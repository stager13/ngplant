/***************************************************************************

 Copyright (c) 2014 Sergey Prokhorchuk.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the author nor the names of contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

***************************************************************************/

#ifndef __ZIPSTORE_H__
#define __ZIPSTORE_H__

#include <stdlib.h>
#include <time.h>

namespace ZS {

class Error
 {
  public           :

                   Error              (int                 code);

  int              GetCode            () const;
  const char      *GetMessage         () const;

  enum
   {
    IO_ERROR, END_OF_CENTRAL_DIR_RECORD_NOT_FOUND, ENCRYPTION_NOT_SUPPORTED,
    COMPRESSION_NOT_SUPPORTED, BAD_CENTRAL_DIRECTORY_SIGNATURE,
    BAD_LOCAL_FILE_HEADER_SIGNATURE, NO_MORE_FILES, NO_MORE_DATA,
    INVALID_DATETIME
   };

  private          :

  static
  const char      *GetMessageByCode   (int                 code);

  int              code;
 };

class InStream
 {
  public                     :

  virtual                   ~InStream () {}

  virtual void               Read     (void               *dest,
                                       size_t              size) = 0;

  virtual unsigned long      GetSize  () const = 0;
  virtual unsigned long      GetPos   () const = 0;
  virtual void               Seek     (unsigned long       pos) = 0;
 };

class OutStream
 {
  public           :

  virtual         ~OutStream          () {}

  virtual void     Write              (const void         *data,
                                       size_t              size) = 0;

  virtual unsigned long      GetPos   () const = 0;
  virtual void               Seek     (unsigned long       pos) = 0;
 };

class CRC32
 {
  public           :

                   CRC32              ();

  void             Reset              ();
  void             Update             (const void         *data,
                                       size_t              size);

  unsigned int     GetCRC32           () const;

//static void      DumpTable          ();

  private          :

  unsigned int     crc;

  enum { POLYNOMIAL = 0xedb88320U };

  static unsigned int                  table[256];
 };

class Reader
 {
  public           :

                   Reader                  (InStream      &_in);
                  ~Reader                  ();

  struct DirEntry;

  class File
   {
    public         :

                   File                    (InStream      &_in,
                                            const DirEntry&entry);

    const char    *GetName                 () const;
    size_t         GetSize                 () const;
    time_t         GetDateTime             () const;

    void           Read                    (void          *dest,
                                            size_t         size);

    private        :

    InStream      &in;
    const char    *name;
    time_t         dateTime;
    size_t         offset;
    size_t         size;
   };

  bool             IsEOF                   () const;
  void             SeekFirst               ();
  void             SeekNext                ();
  File             GetFile                 () const;

  struct DirEntry
   {
    public         :

                        DirEntry           (unsigned int   crc32,
                                            size_t         size,
                                            const char    *name,
                                            time_t         dateTime,
                                            size_t         localHeaderOffset);

                       ~DirEntry           ();

    unsigned int        crc32;
    size_t              size;
    const char         *name;
    time_t              dateTime;
    size_t              localHeaderOffset;

    struct DirEntry    *next;
   };


  private          :

  struct ECDRecord
   {
    unsigned int   CentralDirEntries;
    unsigned long  CentralDirSize;
    unsigned long  CentralDirOffset;
   };

  unsigned long    ReadECDRecord           (ECDRecord               *ecd);
  bool             IsValidECDRecord        (unsigned long            fileOffset,
                                            unsigned long            fileSize,
                                            const unsigned char     *buf,
                                            unsigned long            size) const;

  void             LoadDirEntries          (ECDRecord               &ecd);
  DirEntry        *LoadDirEntry            ();
  void             FreeDirEntries          ();

  void             SeekToFile              (const DirEntry          *entry);

  unsigned int     ReadByte                ();
  unsigned int     ReadWord                ();
  unsigned int     ReadLongWord            ();
  void             SkipBytes               (size_t                   size);

  static unsigned int   MemReadWord        (const unsigned char     *mem);
  static unsigned int   MemReadLongWord    (const unsigned char     *mem);

  InStream        &in;
  DirEntry        *firstDirEntry;
  DirEntry        *currDirEntry;
 };

class Writer
 {
  public           :

                   Writer                  (OutStream     &_out);
                  ~Writer                  ();

  void             BeginFile               (const char    *name,
                                            time_t         dateTime);
  void             WriteData               (const void    *data,
                                            size_t         size);
  void             Close                   ();

  private          :

  struct Entry
   {
                   Entry                   (const char    *name,
                                            unsigned int   dosDate,
                                            unsigned int   dosTime,
                                            size_t         localHeaderOffset);
                  ~Entry                   ();

    const char    *name;
    size_t         localHeaderOffset;
    size_t         size;
    unsigned int   dosDate;
    unsigned int   dosTime;
    unsigned int   crc32;

    struct Entry  *next;
   };

  void             EndFile                 (void);

  Entry           *AddNewEntry             (const char    *name,
                                            time_t         dateTime,
                                            size_t         offset);
  unsigned int     GetNumberOfEntries      () const;

  void             WriteLocalFileHeader    (const char    *name,
                                            unsigned int   dosDate,
                                            unsigned int   dosTime);
  void             WriteDataDescriptor     ();
  void             WriteCentralDirectory   ();
  void             WriteCentralDirectoryHeader
                                           (Entry         *entry);
  void             WriteEndOfCentralDirectoryRecord
                                           ();
  void             UpdateSizeAndCRCInLocalFileHeader
                                           (Entry         *entry);

  void             WriteByte               (unsigned int   v);
  void             WriteWord               (unsigned int   v);
  void             WriteLongWord           (unsigned int   v);
  void             WriteBytes              (const void    *bytes,
                                            size_t         size);

  void             FreeEntries             ();

  OutStream       &out;
  CRC32            crc32;

  size_t           centralDirectoryOffset;
  size_t           centralDirectorySize;
  Entry           *first;
  Entry           *last;
 };

}

#endif // __ZIPSTORE_H__

