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

#ifndef __P3DAPPPREFS_H__
#define __P3DAPPPREFS_H__

#include <wx/wx.h>
#include <wx/config.h>

enum
 {
  P3D_ALWAYS = 0,
  P3D_NEVER     ,
  P3D_ASK
 };

struct P3DAppColor3b
 {
                   P3DAppColor3b      () {};

                   P3DAppColor3b      (unsigned char       aR,
                                       unsigned char       aG,
                                       unsigned char       aB)
                   : R(aR),G(aG),B(aB)
   {
   }

  unsigned char    R;
  unsigned char    G;
  unsigned char    B;
 };

struct P3DExport3DPrefs
 {
                   P3DExport3DPrefs   ();

  void             Read               (const wxConfigBase *Config);
  void             Save               (wxConfigBase       *Config) const;

  unsigned int     HiddenGroupsExportMode; /* P3D_ALWAYS,P3D_NEVER or P3D_ASK */
  unsigned int     OutVisRangeExportMode; /* P3D_ALWAYS,P3D_NEVER or P3D_ASK */

  private          :

  static
  unsigned int     LongToMode         (long                Value);
  void             SetDefaults        ();
 };

struct P3DCameraControlPrefs
 {
                   P3DCameraControlPrefs
                                      ();

  void             Read               (const wxConfigBase *Config);
  void             Save               (wxConfigBase       *Config) const;

  void             Init               ();

  /* true  - mouse motion rotates view around camera(view) space Y axis */
  /* false - mouse motion rotates view around world space Y axis        */
  bool             MouseRotYCS;

  float            MouseRotYSens;
  float            MouseRotXSens;
 };

struct P3D3DViewPrefs
 {
                   P3D3DViewPrefs     ();

  void             Read               (const wxConfigBase *Config);
  void             Save               (wxConfigBase       *Config) const;

  bool             GroundVisible;
  P3DAppColor3b    GroundColor;
  P3DAppColor3b    BackgroundColor;

  private          :

  void             SetDefaults        ();
  static bool      ColorFromStr       (P3DAppColor3b      *Color,
                                       const wxString     &Str);
  static wxString  ColorToStr         (const P3DAppColor3b*Color);
 };

struct P3DUIControlsPrefs
 {
  static void      Read               (const wxConfigBase *Config);
  static void      Save               (wxConfigBase       *Config);
 };

struct P3DRenderQuirksPrefs
 {
                   P3DRenderQuirksPrefs
                                      ();

  void             Read               (const wxConfigBase *Config);
  void             Save               (wxConfigBase       *Config) const;

  bool                                 UseColorArray;

  private          :

  void             SetDefaults        ();
 };

#endif

