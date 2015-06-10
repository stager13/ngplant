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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <new>

#include "zipstore.h"

namespace ZS {

#define ZS_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define ZS_ZIP_VER_MAJOR (1)
#define ZS_ZIP_VER_MINOR (0)

#define ZS_CENTRAL_FILE_HEADER_SIGNATURE (0x02014b50)
#define ZS_LOCAL_FILE_HEADER_SIGNATURE   (0x04034b50)
#define ZS_END_OF_CENTRAL_DIR_SIGNATURE  (0x06054b50)

#define ZS_END_OF_CENTRAL_DIR_RECORD_SIZE (22)
#define ZS_ZIP_FILE_COMMENT_MAX_SIZE     (0x10000)

#define ZS_CENTRAL_DIR_HEADER_FIXED_SIZE (46)

#define ZS_VERSION_MADE_BY_OS_DOS      (0)
#define ZS_VERSION_MADE_BY_SPEC        (ZS_ZIP_VER_MAJOR * 10 + ZS_ZIP_VER_MINOR)
#define ZS_VERSION_MADE_BY             ((ZS_VERSION_MADE_BY_OS_DOS << 8) + ZS_VERSION_MADE_BY_SPEC)
#define ZS_VERSION_NEEDED_TO_EXTRACT   (ZS_VERSION_MADE_BY_SPEC)

#define ZS_VERSION_NEEDED_TO_EXTRACT     (ZS_VERSION_MADE_BY_SPEC)

#define ZS_GPFLAG_NOT_ENCRYPTED        (0x0000)
#define ZS_GPFLAG_ENCRYPTED            (0x0001)

#define ZS_COMPRESSION_METHOD_STORED   (0x0000)

#define ZS_LOCAL_FILE_HEADER_CRC32_OFFSET (14)

#define ZS_CRC32_UNDEFINED             (0)
#define ZS_COMPRESSED_SIZE_UNDEFINED   (0)
#define ZS_UNCOMPRESSED_SIZE_UNDEFINED (0)

#define ZS_EMPTY_EXTRA_FIELD_LENGTH    (0)
#define ZS_EMPTY_FILE_COMMENT_LENGTH   (0)
#define ZS_DISK_NUMBER                 (0)
#define ZS_FILE_DISK_NUMBER_START      (0)
#define ZS_INTERNAL_FILE_ATTRS         (0)
#define ZS_EXTERNAL_FILE_ATTRS         (0)

#define ZS_DOS_BASE_YEAR               (1980)
#define ZS_TIMET_BASE_YEAR             (1900)

#if defined(_MSC_VER)
 #define strdup _strdup
#endif

static unsigned int
MakeDosDate                           (unsigned int        year,
                                       unsigned int        month,
                                       unsigned int        day)
 {
  return (((year - ZS_DOS_BASE_YEAR) & 0x7F) << 9) |
         (((month + 1) & 0x0F) << 5)               |
         (day & 0x1F);
 }

static unsigned int
MakeDosTime                           (unsigned int        hour,
                                       unsigned int        minute,
                                       unsigned int        second)
 {
  return ((hour & 0x1F) << 11) |
         ((minute & 0x3F) << 5)               |
         ((second / 2) & 0x1F);
 }

static void
TimeT2DosDateTime                     (unsigned int       *dosDate,
                                       unsigned int       *dosTime,
                                       time_t              dateTime)
 {
  struct tm *dateTimeParts = localtime(&dateTime);

  if (dateTimeParts == NULL)
   {
    *dosDate = 0;
    *dosTime = 0;
   }
  else
   {
    *dosDate = MakeDosDate(dateTimeParts->tm_year + 1900,
                           dateTimeParts->tm_mon,
                           dateTimeParts->tm_mday);
    *dosTime = MakeDosTime(dateTimeParts->tm_hour,
                           dateTimeParts->tm_min,
                           dateTimeParts->tm_sec);
   }
 }

static void
DosDateTime2TimeT                     (time_t             *time,
                                       unsigned int        dosDate,
                                       unsigned int        dosTime)
 {
  struct tm dateTimeParts;

  dateTimeParts.tm_sec  = (dosTime & 0x1F) * 2;
  dateTimeParts.tm_min  = (dosTime >> 5) & 0x3F;
  dateTimeParts.tm_hour = dosTime >> 11;

  dateTimeParts.tm_mday = dosDate & 0x1F;
  dateTimeParts.tm_mon  = ((dosDate >> 5) & 0x0F) - 1;
  dateTimeParts.tm_year = (dosDate >> 9) + ZS_DOS_BASE_YEAR - ZS_TIMET_BASE_YEAR;
  dateTimeParts.tm_isdst = -1;

  *time = mktime(&dateTimeParts);

  if (*time == -1)
   {
    *time = 0;
   }
 }

static const char *ErrorMessages[] =
 {
  "I/O error",
  "broken zip file (end of central directory record not found)",
  "unsupported feature (encryption)",
  "unsupported feature (compression)",
  "broken zip file (bad central directory file header signature)",
  "broken zip file (bad local file header header signature)",
  "trying to seek beyond the end of archive",
  "trying to seek beyond the end of file",
  "invalid datetime"
 };

static const char  UnknownErrorMessage[] = "undefined error";

Error::Error                          (int                 code)
 {
  this->code = code;
 }

int
Error::GetCode                        () const
 {
  return code;
 }

const char*
Error::GetMessage                     () const
 {
  return GetMessageByCode(code);
 }

const char*
Error::GetMessageByCode               (int                 code)
 {
  if (code >= 0 && (unsigned int)code < ZS_ARRAY_SIZE(ErrorMessages))
   {
    return ErrorMessages[code];
   }
  else
   {
    return UnknownErrorMessage;
   }
 }

Reader::DirEntry::DirEntry            (unsigned int   crc32,
                                       size_t         size,
                                       const char    *name,
                                       time_t         dateTime,
                                       size_t         localHeaderOffset)
 {
  this->crc32             = crc32;
  this->size              = size;
  this->name              = name;
  this->dateTime          = dateTime;
  this->localHeaderOffset = localHeaderOffset;

  next = 0;
 }

Reader::DirEntry::~DirEntry           ()
 {
  free(const_cast<char*>(name));
 }

Reader::File::File                    (InStream                &_in,
                                       const Reader::DirEntry  &entry)
 : in(_in),name(entry.name),dateTime(entry.dateTime),offset(0),size(entry.size)
 {
 }

const char*
Reader::File::GetName                 () const
 {
  return name;
 }

time_t
Reader::File::GetDateTime             () const
 {
  return dateTime;
 }

size_t
Reader::File::GetSize                 () const
 {
  return size;
 }

void
Reader::File::Read                    (void          *dest,
                                       size_t         size)
 {
  offset += size;

  if (offset > this->size)
   {
    throw Error(Error::NO_MORE_DATA);
   }

  in.Read(dest,size);
 }

Reader::Reader                        (InStream      &_in)
 : in(_in)
 {
  ECDRecord   ecdRecord;

  firstDirEntry = 0;

  ReadECDRecord(&ecdRecord);

  try
   {
    LoadDirEntries(ecdRecord);
   }
  catch (...)
   {
    FreeDirEntries();

    throw;
   }

  SeekFirst();
 }

Reader::~Reader                       ()
 {
  FreeDirEntries();
 }

bool
Reader::IsEOF                         () const
 {
  return currDirEntry == 0;
 }

void
Reader::SeekFirst                     ()
 {
  currDirEntry = firstDirEntry;

  if (currDirEntry != 0)
   {
    SeekToFile(currDirEntry);
   }
 }

void
Reader::SeekNext                      ()
 {
  if (IsEOF())
   {
    throw Error(Error::NO_MORE_FILES);
   }

  currDirEntry = currDirEntry->next;

  if (currDirEntry != 0)
   {
    SeekToFile(currDirEntry);
   }
 }

Reader::File
Reader::GetFile                       () const
 {
  if (IsEOF())
   {
    throw Error(Error::NO_MORE_FILES);
   }

  return File(in,*currDirEntry);
 }

void
Reader::SeekToFile                    (const DirEntry          *entry)
 {
  in.Seek(entry->localHeaderOffset);

  unsigned int signature = ReadLongWord();

  if (signature != ZS_LOCAL_FILE_HEADER_SIGNATURE)
   {
    throw Error(Error::BAD_LOCAL_FILE_HEADER_SIGNATURE);
   }

  SkipBytes(22);

  unsigned int nameLen  = ReadWord();
  unsigned int extraLen = ReadWord();

  SkipBytes(nameLen + extraLen);
 }

unsigned long
Reader::ReadECDRecord                 (ECDRecord          *ecd)
 {
  const unsigned long BLOCK_SIZE = 128;

  unsigned char buf[BLOCK_SIZE * 2];

  unsigned long zipFileSize = in.GetSize();
  unsigned long offsetInFile;
  unsigned long end;

  unsigned long sizeToRead;

  sizeToRead   = zipFileSize >= BLOCK_SIZE ? BLOCK_SIZE : zipFileSize;
  offsetInFile = zipFileSize - sizeToRead;
  end          = zipFileSize - offsetInFile;

  in.Seek(in.GetSize());

  while (sizeToRead > 0 &&
         (zipFileSize - offsetInFile) <= (ZS_ZIP_FILE_COMMENT_MAX_SIZE + ZS_END_OF_CENTRAL_DIR_RECORD_SIZE))
   {
    in.Seek(in.GetPos() - sizeToRead);
    in.Read(buf,sizeToRead);

    for (unsigned long back = 1; back <= end; back++)
     {
      unsigned long offsetInBuf = end - back;

      if (buf[offsetInBuf] == (ZS_END_OF_CENTRAL_DIR_SIGNATURE & 0xFF))
       {
        if (IsValidECDRecord(offsetInFile + offsetInBuf,zipFileSize,&buf[offsetInBuf],back))
         {
          ecd->CentralDirEntries = MemReadWord(&buf[offsetInBuf + 10]);
          ecd->CentralDirSize    = MemReadLongWord(&buf[offsetInBuf + 12]);
          ecd->CentralDirOffset  = MemReadLongWord(&buf[offsetInBuf + 16]);

          return offsetInFile - offsetInBuf;
         }
       }
     }

    sizeToRead = offsetInFile >= BLOCK_SIZE ? BLOCK_SIZE : offsetInFile;

    if (sizeToRead > 0)
     {
      offsetInFile -= sizeToRead;
      end           = sizeToRead + BLOCK_SIZE;

      memmove(&buf[sizeToRead],&buf,BLOCK_SIZE);
     }
   }

  throw Error(Error::END_OF_CENTRAL_DIR_RECORD_NOT_FOUND);

  return 0;
 }

bool
Reader::IsValidECDRecord              (unsigned long            fileOffset,
                                       unsigned long            fileSize,
                                       const unsigned char     *buf,
                                       unsigned long            size) const
 {
  if (size < ZS_END_OF_CENTRAL_DIR_RECORD_SIZE)
   {
    return false;
   }

  if (MemReadLongWord(buf) != ZS_END_OF_CENTRAL_DIR_SIGNATURE)
   {
    return false;
   }

  unsigned int fileCommentLength = MemReadWord(&buf[ZS_END_OF_CENTRAL_DIR_RECORD_SIZE - 2]);

  if (fileSize != fileOffset + ZS_END_OF_CENTRAL_DIR_RECORD_SIZE + fileCommentLength)
   {
    return false;
   }

  return true;
 }

void
Reader::LoadDirEntries                (ECDRecord               &ecd)
 {
  in.Seek(ecd.CentralDirOffset);

  DirEntry *currEntry = 0;
  DirEntry *prevEntry = 0;

  for (unsigned int entryIndex = 0;
       entryIndex < ecd.CentralDirEntries;
       entryIndex++)
   {
    currEntry = LoadDirEntry();

    if (prevEntry != 0)
     {
      prevEntry->next = currEntry;
     }
    else
     {
      firstDirEntry = currEntry;
     }

    prevEntry = currEntry;
   }
 }

Reader::DirEntry*
Reader::LoadDirEntry                  ()
 {
  unsigned char    headerData[ZS_CENTRAL_DIR_HEADER_FIXED_SIZE];

  in.Read(headerData,sizeof(headerData));

  unsigned int signature = MemReadLongWord(headerData);
  unsigned int flags     = MemReadWord(&headerData[8]);
  unsigned int method    = MemReadWord(&headerData[10]);
  unsigned int dosTime   = MemReadWord(&headerData[12]);
  unsigned int dosDate   = MemReadWord(&headerData[14]);
  unsigned int crc32     = MemReadLongWord(&headerData[16]);
  unsigned long size     = MemReadLongWord(&headerData[20]);
  unsigned long nameLen  = MemReadWord(&headerData[28]);
  unsigned long extraFieldLen  = MemReadWord(&headerData[30]);
  unsigned long fileCommentLen = MemReadWord(&headerData[32]);
  unsigned long localHeaderOffset = MemReadLongWord(&headerData[42]);

  if (signature != ZS_CENTRAL_FILE_HEADER_SIGNATURE)
   {
    throw Error(Error::BAD_CENTRAL_DIRECTORY_SIGNATURE);
   }

  if (flags & ZS_GPFLAG_ENCRYPTED)
   {
    throw Error(Error::ENCRYPTION_NOT_SUPPORTED);
   }

  if (method != ZS_COMPRESSION_METHOD_STORED)
   {
    throw Error(Error::COMPRESSION_NOT_SUPPORTED);
   }

  char *name = (char*)malloc(nameLen + 1);

  if (name == NULL)
   {
    throw std::bad_alloc();
   }

  try
   {
    in.Read(name,nameLen);

    in.Seek(in.GetPos() + extraFieldLen + fileCommentLen);

    name[nameLen] = '\x00';

    time_t dateTime;

    DosDateTime2TimeT(&dateTime,dosDate,dosTime);

    return new DirEntry(crc32,size,name,dateTime,localHeaderOffset);
   }
  catch (...)
   {
    free(name);

    throw;

    return 0;
   }
 }

void
Reader::FreeDirEntries                ()
 {
  DirEntry *curr;
  DirEntry *next;

  for (curr = firstDirEntry; curr != 0; curr = next)
   {
    next = curr->next;

    delete curr;
   }
 }

unsigned int
Reader::ReadByte                      ()
 {
  unsigned char byte;

  in.Read(&byte,1);

  return byte;
 }

unsigned int
Reader::ReadWord                      ()
 {
  unsigned int lo = ReadByte();
  unsigned int hi = ReadByte();

  return (hi << 8) | lo;
 }

unsigned int
Reader::ReadLongWord                  ()
 {
  unsigned int lo = ReadWord();
  unsigned int hi = ReadWord();

  return (hi << 16) | lo;
 }

void
Reader::SkipBytes                     (size_t                   size)
 {
  in.Seek(in.GetPos() + size);
 }

unsigned int
Reader::MemReadWord                   (const unsigned char     *mem)
 {
  return mem[0] | ((unsigned int)(mem[1]) << 8);
 }

unsigned int
Reader::MemReadLongWord               (const unsigned char     *mem)
 {
  return MemReadWord(mem) | (MemReadWord(&mem[2]) << 16);
 }

Writer::Entry::Entry                  (const char         *name,
                                       unsigned int        dosDate,
                                       unsigned int        dosTime,
                                       size_t              localHeaderOffset)
 {
  this->name = strdup(name);

  if (this->name == 0)
   {
    throw std::bad_alloc();
   }

  this->dosDate = dosDate;
  this->dosTime = dosTime;

  this->localHeaderOffset = localHeaderOffset;

  size  = 0;
  crc32 = 0;
  next  = 0;
 }

Writer::Entry::~Entry                 ()
 {
  free(const_cast<char*>(name));
 }

Writer::Writer                        (OutStream          &_out)
 : out(_out),centralDirectoryOffset(0),first(0),last(0)
 {
 }

Writer::~Writer                       ()
 {
  FreeEntries();
 }

void
Writer::BeginFile                     (const char         *name,
                                       time_t              dateTime)
 {
  EndFile();

  Entry *entry = AddNewEntry(name,dateTime,out.GetPos());

  crc32.Reset();

  WriteLocalFileHeader(name,entry->dosDate,entry->dosTime);
 }

void
Writer::EndFile                       (void)
 {
  if (last == 0) return;

  last->crc32 = crc32.GetCRC32();

  UpdateSizeAndCRCInLocalFileHeader(last);
 }

void
Writer::WriteData                     (const void         *data,
                                       size_t              size)
 {
  WriteBytes(data,size);

  crc32.Update(data,size);

  last->size += size;
 }

void
Writer::Close                         ()
 {
  EndFile();
  WriteCentralDirectory();
  WriteEndOfCentralDirectoryRecord();
 }

Writer::Entry*
Writer::AddNewEntry                   (const char    *name,
                                       time_t         dateTime,
                                       size_t         offset)
 {
  unsigned int dosDate;
  unsigned int dosTime;

  TimeT2DosDateTime(&dosDate,&dosTime,dateTime);

  Entry *entry = new Entry(name,dosDate,dosTime,offset);

  if (last != 0)
   {
    last->next = entry;
    last       = entry;
   }
  else
   {
    first = last = entry;
   }

  return entry;
 }

unsigned int
Writer::GetNumberOfEntries                 () const
 {
  unsigned int n = 0;

  for (Entry *curr = first; curr != 0; curr = curr->next)
   {
    ++n;
   }

  return n;
 }

void
Writer::WriteLocalFileHeader          (const char    *name,
                                       unsigned int   dosDate,
                                       unsigned int   dosTime)
 {
  size_t       nameLength  = strlen(name);

  WriteLongWord(ZS_LOCAL_FILE_HEADER_SIGNATURE);
  WriteWord(ZS_VERSION_NEEDED_TO_EXTRACT);
  WriteWord(ZS_GPFLAG_NOT_ENCRYPTED);
  WriteWord(ZS_COMPRESSION_METHOD_STORED);
  WriteWord(dosTime);
  WriteWord(dosDate);
  WriteLongWord(ZS_CRC32_UNDEFINED);
  WriteLongWord(ZS_COMPRESSED_SIZE_UNDEFINED);
  WriteLongWord(ZS_UNCOMPRESSED_SIZE_UNDEFINED);
  WriteWord(nameLength);
  WriteWord(ZS_EMPTY_EXTRA_FIELD_LENGTH);

  WriteBytes(name,nameLength);
 }

void
Writer::UpdateSizeAndCRCInLocalFileHeader  (Entry         *entry)
 {
  long currentPos = out.GetPos();

  out.Seek(entry->localHeaderOffset + ZS_LOCAL_FILE_HEADER_CRC32_OFFSET);

  WriteLongWord(entry->crc32);
  WriteLongWord(entry->size);
  WriteLongWord(entry->size);

  out.Seek(currentPos);
 }

void
Writer::WriteCentralDirectory              ()
 {
  centralDirectoryOffset = out.GetPos();

  for (Entry *curr = first; curr != 0; curr = curr->next)
   {
    WriteCentralDirectoryHeader(curr);
   }

  centralDirectorySize = out.GetPos() - centralDirectoryOffset;
 }

void
Writer::WriteCentralDirectoryHeader        (Entry         *entry)
 {
  size_t       nameLength  = strlen(entry->name);

  WriteLongWord(ZS_CENTRAL_FILE_HEADER_SIGNATURE);
  WriteWord(ZS_VERSION_MADE_BY);
  WriteWord(ZS_VERSION_NEEDED_TO_EXTRACT);
  WriteWord(ZS_GPFLAG_NOT_ENCRYPTED);
  WriteWord(ZS_COMPRESSION_METHOD_STORED);
  WriteWord(entry->dosTime);
  WriteWord(entry->dosDate);
  WriteLongWord(entry->crc32);
  WriteLongWord(entry->size);
  WriteLongWord(entry->size);
  WriteWord(nameLength);
  WriteWord(ZS_EMPTY_EXTRA_FIELD_LENGTH);
  WriteWord(ZS_EMPTY_FILE_COMMENT_LENGTH);
  WriteWord(ZS_FILE_DISK_NUMBER_START);
  WriteWord(ZS_INTERNAL_FILE_ATTRS);
  WriteLongWord(ZS_EXTERNAL_FILE_ATTRS);
  WriteLongWord(entry->localHeaderOffset);

  WriteBytes(entry->name,nameLength);
 }

void
Writer::WriteEndOfCentralDirectoryRecord   ()
 {
  unsigned int numberOfEntries = GetNumberOfEntries();

  WriteLongWord(ZS_END_OF_CENTRAL_DIR_SIGNATURE);
  WriteWord(ZS_DISK_NUMBER);
  WriteWord(ZS_DISK_NUMBER);
  WriteWord(numberOfEntries);
  WriteWord(numberOfEntries);
  WriteLongWord(centralDirectorySize);
  WriteLongWord(centralDirectoryOffset);
  WriteWord(ZS_EMPTY_FILE_COMMENT_LENGTH);
 }

void
Writer::WriteByte                          (unsigned int   v)
 {
  unsigned char b = (unsigned char)v;

  WriteBytes(&b,1);
 }

void
Writer::WriteWord                          (unsigned int   v)
 {
  WriteByte(v & 0xFF);
  WriteByte(v >> 8);
 }

void
Writer::WriteLongWord                      (unsigned int   v)
 {
  WriteWord(v & 0xFFFF);
  WriteWord(v >> 16);
 }

void
Writer::WriteBytes                    (const void         *bytes,
                                       size_t              size)
 {
  out.Write(bytes,size);
 }

void
Writer::FreeEntries                   ()
 {
  Entry *curr;
  Entry *next;

  for (curr = first; curr != 0; curr = next)
   {
    next = curr->next;

    delete curr;
   }
 }

unsigned int CRC32::table[256] =
 {
  0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
  0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
  0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
  0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
  0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
  0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
  0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
  0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
  0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
  0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
  0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
  0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
  0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
  0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
  0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
  0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
  0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
  0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
  0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
  0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
  0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
  0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
  0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
  0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
  0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
  0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
  0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
  0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
  0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
  0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
  0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
  0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
  0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
  0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
  0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
  0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
  0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
  0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
  0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
  0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
  0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
  0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
  0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
  0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
  0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
  0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
  0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
  0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
  0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
  0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
  0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
  0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
  0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
  0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
  0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
  0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
  0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
  0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
  0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
  0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
  0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
  0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
  0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
  0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
 };

CRC32::CRC32                          ()
 {
  Reset();
 }

void
CRC32::Reset                          ()
 {
  crc = 0xFFFFFFFFU;
 }

void
CRC32::Update                         (const void         *data,
                                       size_t              size)
 {
  const unsigned char *p = (const unsigned char*)data;

  for (size_t i = 0; i < size; i++)
   {
    crc = (crc >> 8) ^ table[(crc & 0xFF) ^ (*p++)];
   }
 }

unsigned int
CRC32::GetCRC32                       () const
 {
  return ~crc;
 }

/*
void
CRC32::DumpTable                      ()
 {
  for (unsigned int byte = 0; byte <= 0xFF; byte++)
   {
    unsigned int remainder = byte;

    for (unsigned int bit = 0; bit < 8; bit++)
     {
      if (remainder & 0x01)
       {
        remainder = POLYNOMIAL ^ (remainder >> 1);
       }
      else
       {
        remainder >>= 1;
       }
     }

    if (byte % 4 == 0)
     {
      printf("\n");
     }

    printf("0x%08X, ",remainder);
   }
 }
*/

}

