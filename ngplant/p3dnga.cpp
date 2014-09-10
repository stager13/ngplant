/***************************************************************************

 Copyright (C) 2014  Sergey Prokhorchuk

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

#include <stdio.h>
#include <time.h>

#include <map>

#include "zipstore/zipstore.h"

#include <ngpcore/p3diostream.h>
#include <ngput/p3dospath.h>

#include "p3dapp.h"
#include "p3dnga.h"

namespace {
class OutFileStream : public ZS::OutStream
 {
  public           :

                   OutFileStream      (const char         *FileName);
  virtual         ~OutFileStream      ();

  virtual void     Write              (const void         *Data,
                                       size_t              Size);

  virtual unsigned long      GetPos   () const;
  virtual void               Seek     (unsigned long       Pos);

  public           :

  FILE            *File;
 };

OutFileStream::OutFileStream          (const char         *FileName)
 {
  File = fopen(FileName,"wb");

  if (File == NULL)
   {
    throw ZS::Error(ZS::Error::IO_ERROR);
   }
 }

OutFileStream::~OutFileStream         ()
 {
  if (fclose(File) != 0)
   {
    throw ZS::Error(ZS::Error::IO_ERROR);
   }
 }

void
OutFileStream::Write                  (const void         *Data,
                                       size_t              Size)
 {
  if (fwrite(Data,1,Size,File) != Size)
   {
    throw ZS::Error(ZS::Error::IO_ERROR);
   }
 }

unsigned long
OutFileStream::GetPos                 () const
 {
  return ftell(File);
 }

void
OutFileStream::Seek                   (unsigned long       Pos)
 {
  if (fseek(File,Pos,SEEK_SET) != 0)
   {
    throw ZS::Error(ZS::Error::IO_ERROR);
   }
 }

class OutputStringStream : public P3DOutputStringStream
 {
  public           :

                   OutputStringStream (ZS::Writer         &Writer);

  virtual void     WriteString        (const char         *Buffer);
  virtual void     AutoLnEnable       ();
  virtual void     AutoLnDisable      ();

  private          :

  bool             AutoLn;
  ZS::Writer      &Writer;
 };

                   OutputStringStream::OutputStringStream
                                      (ZS::Writer         &_Writer)
 : Writer(_Writer)
 {
 }

void               OutputStringStream::WriteString
                                      (const char         *Buffer)
 {
  Writer.WriteData(Buffer,strlen(Buffer));

  if (AutoLn)
   {
    Writer.WriteData("\n",1);
   }
 }

void               OutputStringStream::AutoLnEnable
                                      ()
 {
  AutoLn = true;
 }

void               OutputStringStream::AutoLnDisable
                                      ()
 {
  AutoLn = false;
 }

class NGAMaterialSaver : public P3DMaterialSaver
 {
  public           :

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMaterialInstance
                                                          *Material) const;

  void             SaveTextures       (ZS::Writer         &ZipWriter) const;

  private          :

  void             ProcessTextures    (const P3DMaterialInstanceSimple
                                                          *Material,
                                       P3DMaterialDef     *MaterialDef) const;
  void             ProcessTexture     (P3DMaterialDef     *MaterialDef,
                                       const char         *TexName,
                                       const char         *FileName) const;

  void             SaveTexture        (ZS::Writer         &ZipWriter,
                                       const char         *TexName,
                                       const char         *FileName) const;

  static
  std::string      TexName2NGATexName (const char         *TexName);

  typedef std::map<std::string,std::string> TextureBindings ;

  mutable TextureBindings Textures;
 };

void               NGAMaterialSaver::Save
                                      (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMaterialInstance
                                                          *Material) const
 {
  const P3DMaterialInstanceSimple
                       *MaterialImpl        = (const P3DMaterialInstanceSimple*)Material;
  const P3DMaterialDef *OriginalMaterialDef = Material->GetMaterialDef();

  P3DMaterialDef NewMaterialDef(*OriginalMaterialDef);

  ProcessTextures(MaterialImpl,&NewMaterialDef);

  NewMaterialDef.Save(TargetStream);
 }

void               NGAMaterialSaver::SaveTextures
                                      (ZS::Writer         &ZipWriter) const
 {
  for (TextureBindings::const_iterator Iter = Textures.begin();
       Iter != Textures.end();
       ++Iter)
   {
    SaveTexture(ZipWriter,Iter->first.c_str(),Iter->second.c_str());
   }
 }


void               NGAMaterialSaver::SaveTexture
                                      (ZS::Writer         &ZipWriter,
                                       const char         *TexName,
                                       const char         *FileName) const
 {
  FILE            *SrcFile;

  SrcFile = fopen(FileName,"rb");

  if (SrcFile == NULL)
   {
    throw P3DExceptionGeneric("Unable to open texture file");
   }

  try
   {
    ZipWriter.BeginFile(TexName,time(NULL));

    char   Buffer[8 * 1024];
    size_t Size;

    do
     {
      Size = fread(Buffer,1,sizeof(Buffer),SrcFile);

      if (Size > 0)
       {
        ZipWriter.WriteData(Buffer,Size);
       }
     } while (Size == sizeof(Buffer));

    if (ferror(SrcFile))
     {
      throw P3DExceptionGeneric("Texture file read error");
     }

    fclose(SrcFile);
   }
  catch (...)
   {
    fclose(SrcFile);

    throw;
   }

 }

void               NGAMaterialSaver::ProcessTextures
                                      (const P3DMaterialInstanceSimple
                                                          *Material,
                                       P3DMaterialDef     *MaterialDef) const
 {
  for (unsigned int LayerIndex = 0; LayerIndex < P3D_MAX_TEX_LAYERS; LayerIndex++)
   {
    const char *TexName = MaterialDef->GetTexName(LayerIndex);

    if (TexName != 0)
     {
      const char *FileName = Material->GetTexFileName(LayerIndex);

      if (FileName != 0)
       {
        ProcessTexture(MaterialDef,TexName,FileName);
       }
     }
   }
 }

void               NGAMaterialSaver::ProcessTexture
                                      (P3DMaterialDef     *MaterialDef,
                                       const char         *TexName,
                                       const char         *FileName) const
 {
  std::string                     NGATexName = TexName2NGATexName(TexName);
  TextureBindings::const_iterator Entry      = Textures.find(NGATexName);
  std::string                     FileNameStr(FileName);

  if (Entry == Textures.end() || Entry->second == FileNameStr)
   {
    Textures[NGATexName] = FileNameStr;
   }
  else
   {
    throw P3DExceptionGeneric(".NGA doesn't support several textures with the same base name");
   }
 }

std::string        NGAMaterialSaver::TexName2NGATexName
                                      (const char         *TexName)
 {
  std::string      BaseName = P3DPathName::BaseName(TexName);

  return std::string(P3D_LOCAL_TEXTURES_PATH) + "/" + BaseName;
 }
}

static void        SaveModel          (ZS::Writer          &ZipWriter,
                                       NGAMaterialSaver    &MaterialSaver)
 {
  wxString NGPFileName(P3DApp::GetApp()->GetDerivedFileName(wxString("ngp",wxConvUTF8)));

  ZipWriter.BeginFile(NGPFileName.mb_str(),time(NULL));

  OutputStringStream OutputStream(ZipWriter);

  P3DApp::GetApp()->GetModel()->Save(&OutputStream,&MaterialSaver);
 }

static void        SaveTextures       (ZS::Writer          &ZipWriter,
                                       NGAMaterialSaver    &MaterialSaver)
 {
  MaterialSaver.SaveTextures(ZipWriter);
 }

static void        WriteNGPToNGA      (ZS::Writer          &ZipWriter)
 {
  NGAMaterialSaver    MaterialSaver;

  SaveModel(ZipWriter,MaterialSaver);
  SaveTextures(ZipWriter,MaterialSaver);
 }

static void       ExportToFile        (const char          *FileName)
 {
  OutFileStream   OutStream(FileName);
  ZS::Writer      ZipWriter(OutStream);

  WriteNGPToNGA(ZipWriter);

  ZipWriter.Close();
 }

static void       DisplayErrorMessage(const char          *Message)
 {
  ::wxMessageBox(wxString(Message,wxConvUTF8),wxT("Error"),wxOK | wxICON_ERROR);
 }

void              P3DNGAExport       ()
 {
  wxString        FileName;

  FileName = ::wxFileSelector(wxT("File name"),
                              wxT(""),
                              P3DApp::GetApp()->GetDerivedFileName(wxT("nga")),
                              wxT(".nga"),
                              wxT("*.nga"),
                              wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (!FileName.empty())
   {
    try
     {
      ExportToFile(FileName.mb_str());
     }
    catch (P3DException &e)
     {
      DisplayErrorMessage(e.GetMessage());
     }
    catch (ZS::Error &e)
     {
      DisplayErrorMessage(e.GetMessage());
     }
   }
 }

