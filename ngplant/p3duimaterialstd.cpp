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

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/colordlg.h>

#include <p3dapp.h>
#include <p3dcmdhelpers.h>

#include <p3dmaterialstd.h>
#include <p3duimaterialstd.h>

enum
 {
  wxID_BASECOLOR_CTRL = wxID_HIGHEST + 1,
  wxID_TEXLAYER_BUTTON_MIN,
  wxID_TEXLAYER_BUTTON_DIFFUSE = wxID_TEXLAYER_BUTTON_MIN,
  wxID_TEXLAYER_BUTTON_NORMAL_MAP,
  wxID_TEXLAYER_BUTTON_AUX0,
  wxID_TEXLAYER_BUTTON_AUX1,
  wxID_TEXLAYER_BUTTON_MAX = wxID_TEXLAYER_BUTTON_AUX1,
  wxID_BASETEXTURE_CTRL,
  wxID_REMOVETEXTURE_CTRL,
  wxID_USCALE_CTRL,
  wxID_VMODE_CTRL,
  wxID_VSCALE_CTRL,
  wxID_DOUBLESIDED_CTRL,
  wxID_TRANSPARENT_CTRL,
  wxID_BILLBOARD_MODE_CTRL,
  wxID_ALPHA_CTRL_MODE_CTRL,
  wxID_ALPHA_FADEIN_CTRL,
  wxID_ALPHA_FADEOUT_CTRL,
 };

BEGIN_EVENT_TABLE(P3DMaterialStdPanel,wxPanel)
 EVT_BUTTON(wxID_BASECOLOR_CTRL,P3DMaterialStdPanel::OnBaseColorClicked)
 EVT_COMMAND_RANGE(wxID_TEXLAYER_BUTTON_MIN,wxID_TEXLAYER_BUTTON_MAX,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,P3DMaterialStdPanel::OnTexLayerClicked)
 EVT_BUTTON(wxID_BASETEXTURE_CTRL,P3DMaterialStdPanel::OnBaseTexClicked)
 EVT_BUTTON(wxID_REMOVETEXTURE_CTRL,P3DMaterialStdPanel::OnRemoveTexClicked)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_USCALE_CTRL,P3DMaterialStdPanel::OnUScaleChanged)
 EVT_CHOICE(wxID_VMODE_CTRL,P3DMaterialStdPanel::OnVModeChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_VSCALE_CTRL,P3DMaterialStdPanel::OnVScaleChanged)
 EVT_CHECKBOX(wxID_DOUBLESIDED_CTRL,P3DMaterialStdPanel::OnDoubleSidedChanged)
 EVT_CHECKBOX(wxID_TRANSPARENT_CTRL,P3DMaterialStdPanel::OnTransparentChanged)
 EVT_CHOICE(wxID_BILLBOARD_MODE_CTRL,P3DMaterialStdPanel::OnBillboardModeChanged)
 EVT_CHECKBOX(wxID_ALPHA_CTRL_MODE_CTRL,P3DMaterialStdPanel::OnAlphaCtrlEnabledChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_ALPHA_FADEIN_CTRL,P3DMaterialStdPanel::OnAlphaFadeInChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_ALPHA_FADEOUT_CTRL,P3DMaterialStdPanel::OnAlphaFadeOutChanged)
END_EVENT_TABLE()

static const char *TexLayerNames[] =
 {
  "Dif", "Nor", "Aux0", "Aux1"
 };

static wxColour GetUnusedTexTextColour()
 {
  return wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
 }

static wxColour GetUsedTexTextColour  ()
 {
  wxColour Colour = GetUnusedTexTextColour();

  return wxColour(Colour.Red(),Colour.Green(),255 - Colour.Blue());
 }

                   P3DMaterialStdPanel::P3DMaterialStdPanel
                                      (wxWindow           *Parent,
                                       P3DMaterialInstanceSimple
                                                          *Material,
                                       P3DStemModel       *StemModel)
                   : P3DUIParamPanel(Parent)
 {
  float            R,G,B;

  StemModelTube = dynamic_cast<P3DStemModelTube*>(StemModel);
  StemModelQuad = dynamic_cast<P3DStemModelQuad*>(StemModel);

  this->Material = Material;

  ActiveTexLayer = P3D_TEX_DIFFUSE;

  Material->GetColor(&R,&G,&B);

  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *ColorTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Color")),wxVERTICAL);
  wxFlexGridSizer  *ColorGridSizer = new wxFlexGridSizer(1,2,3,1);

  ColorGridSizer->AddGrowableCol(1);

  ColorGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Base color")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxButton *BaseColorButton = new wxButton(this,wxID_BASECOLOR_CTRL,wxEmptyString);

  BaseColorButton->SetBackgroundColour(wxColour((int)(R * 255.0f),(int)(G * 255.0f),(int)(B * 255.0f)));

  ColorGridSizer->Add(BaseColorButton,1,wxALL | wxALIGN_RIGHT,1);

  wxSpinSliderCtrl *spin_slider;

  ColorTopSizer->Add(ColorGridSizer,0,wxEXPAND,0);

  TopSizer->Add(ColorTopSizer,0,wxEXPAND | wxALL,1);

  /* Base texture */

  wxStaticBoxSizer *BaseTexTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Texture layers")),wxVERTICAL);

  wxGridSizer      *LayerButtonsSizer = new wxGridSizer(1,4,1,1);

  wxToggleButton   *LayerButton;

  for (unsigned int TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    LayerButton = new wxToggleButton(this,
                                     wxID_TEXLAYER_BUTTON_DIFFUSE + TexLayer,
                                     wxString(TexLayerNames[TexLayer],wxConvUTF8),
                                     wxDefaultPosition,
                                     wxDefaultSize,
                                     wxBU_EXACTFIT);

    if (Material->GetTexHandle(TexLayer) == P3DTexHandleNULL)
     {
      LayerButton->SetForegroundColour(GetUnusedTexTextColour());
     }
    else
     {
      LayerButton->SetForegroundColour(GetUsedTexTextColour());
     }

    LayerButton->SetValue(TexLayer == ActiveTexLayer);

    LayerButtonsSizer->Add(LayerButton,0,wxGROW,1);
   }

  BaseTexTopSizer->Add(LayerButtonsSizer,0,wxGROW | wxALL ,1);

  wxFlexGridSizer  *BaseTexGridSizer = new wxFlexGridSizer(1,2,1,3);

  BaseTexGridSizer->AddGrowableCol(1);

  BaseTexGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Texture")),0,wxALIGN_CENTER_VERTICAL,0);

  BaseTexButton = new wxBitmapButton(this,wxID_BASETEXTURE_CTRL,wxBitmap(),wxDefaultPosition,wxSize(72,72));

  if (Material->GetTexHandle(ActiveTexLayer) == P3DTexHandleNULL)
   {
    BaseTexButton->SetBitmapLabel(P3DApp::GetApp()->GetBitmap(P3D_BITMAP_NO_TEXTURE));
   }
  else
   {
    const wxBitmap *TexBitmap;

    TexBitmap = P3DApp::GetApp()->GetTexManager()->GetBitmap(Material->GetTexHandle(ActiveTexLayer));

    BaseTexButton->SetBitmapLabel(*TexBitmap);
   }

  wxBoxSizer *TexImageControlSizer = new wxBoxSizer(wxHORIZONTAL);

  TexImageControlSizer->Add(BaseTexButton,1,wxGROW,0);

  RemoveTexButton = new wxBitmapButton(this,wxID_REMOVETEXTURE_CTRL,P3DApp::GetApp()->GetBitmap(P3D_BITMAP_REMOVE_TEXTURE),wxDefaultPosition,wxSize(18,18));

  if (Material->GetTexHandle(ActiveTexLayer) == P3DTexHandleNULL)
   {
    RemoveTexButton->Enable(FALSE);
   }

  TexImageControlSizer->Add(RemoveTexButton,0,wxALIGN_TOP,0);

  BaseTexGridSizer->Add(TexImageControlSizer,1,wxALIGN_RIGHT,0);

  BaseTexTopSizer->Add(BaseTexGridSizer,0,wxEXPAND,0);

  TopSizer->Add(BaseTexTopSizer,0,wxEXPAND | wxALL,1);

  /* Texture coordinates */

  if (StemModelTube != 0)
   {
    wxStaticBoxSizer *TexCoordTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Texture coordinates")),wxVERTICAL);
    wxFlexGridSizer  *TexCoordGridSizer = new wxFlexGridSizer(3,2,1,3);

    TexCoordGridSizer->AddGrowableCol(1);

    TexCoordGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("U-Scale")),0,wxALIGN_CENTER_VERTICAL,0);

    spin_slider = new wxSpinSliderCtrl(this,wxID_USCALE_CTRL,wxSPINSLIDER_MODE_FLOAT,StemModelTube->GetTexCoordUScale(),0.0,100.0);
    spin_slider->SetStdStep(1.0);
    spin_slider->SetSmallStep(0.1);
    spin_slider->SetLargeMove(1.0);
    spin_slider->SetStdMove(0.1);
    spin_slider->SetSmallMove(0.01);

    TexCoordGridSizer->Add(spin_slider,1,wxALIGN_RIGHT,0);

    TexCoordGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("V-Mode")),0,wxALIGN_CENTER_VERTICAL,0);

    wxChoice *TexCoordModeCtrl = new wxChoice(this,wxID_VMODE_CTRL,wxDefaultPosition,wxDefaultSize,(int)0,(const wxString*)NULL);
    TexCoordModeCtrl->Append(wxT("Relative"));
    TexCoordModeCtrl->Append(wxT("Absolute"));

    if (StemModelTube->GetTexCoordVMode() == P3DTexCoordModeRelative)
     {
      TexCoordModeCtrl->SetSelection(0);
     }
    else
     {
      TexCoordModeCtrl->SetSelection(1);
     }

    TexCoordGridSizer->Add(TexCoordModeCtrl,1,wxALIGN_RIGHT,0);

    TexCoordGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("V-Scale")),0,wxALIGN_CENTER_VERTICAL,0);

    spin_slider = new wxSpinSliderCtrl(this,wxID_VSCALE_CTRL,wxSPINSLIDER_MODE_FLOAT,StemModelTube->GetTexCoordVScale(),0.0,100.0);
    spin_slider->SetStdStep(1.0);
    spin_slider->SetSmallStep(0.1);
    spin_slider->SetLargeMove(1.0);
    spin_slider->SetStdMove(0.1);
    spin_slider->SetSmallMove(0.01);

    TexCoordGridSizer->Add(spin_slider,1,wxALIGN_RIGHT,0);

    TexCoordTopSizer->Add(TexCoordGridSizer,0,wxEXPAND,0);

    TopSizer->Add(TexCoordTopSizer,0,wxEXPAND | wxALL,1);
   }

  wxStaticBoxSizer *ModeTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Mode")),wxVERTICAL);
  wxFlexGridSizer  *ModeGridSizer = new wxFlexGridSizer(3,2,1,3);

  ModeGridSizer->AddGrowableCol(1);

  wxCheckBox *DoubleSidedCheckBox = new wxCheckBox(this,wxID_DOUBLESIDED_CTRL,wxT(""));
  DoubleSidedCheckBox->SetValue(Material->IsDoubleSided());
  ModeGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Double-sided")),0,wxALIGN_CENTER_VERTICAL,0);
  ModeGridSizer->Add(DoubleSidedCheckBox,1,wxALIGN_RIGHT,0);

  wxCheckBox *TransparentCheckBox = new wxCheckBox(this,wxID_TRANSPARENT_CTRL,wxT(""));
  TransparentCheckBox->SetValue(Material->IsTransparent());
  ModeGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Transparent")),0,wxALIGN_CENTER_VERTICAL,0);
  ModeGridSizer->Add(TransparentCheckBox,1,wxALIGN_RIGHT,0);

  wxChoice *BillboardModeCtrl = new wxChoice(this,wxID_BILLBOARD_MODE_CTRL,wxDefaultPosition,wxDefaultSize,(int)0,(const wxString*)NULL);
  BillboardModeCtrl->Append(wxT("Disabled"));
  BillboardModeCtrl->Append(wxT("Spherical"));
  BillboardModeCtrl->Append(wxT("Cylindrical"));

  unsigned int BillboardMode;

  BillboardMode = Material->GetBillboardMode();

  if      (BillboardMode == P3D_BILLBOARD_MODE_SPHERICAL)
   {
    BillboardModeCtrl->SetSelection(1);
   }
  else if (BillboardMode == P3D_BILLBOARD_MODE_CYLINDRICAL)
   {
    BillboardModeCtrl->SetSelection(2);
   }
  else
   {
    BillboardModeCtrl->SetSelection(0);
   }

  if (StemModelQuad == 0)
   {
    BillboardModeCtrl->Enable(false);
   }

  ModeGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Billboard mode")),0,wxALIGN_CENTER_VERTICAL,0);
  ModeGridSizer->Add(BillboardModeCtrl,1,wxALIGN_RIGHT,0);

  ModeTopSizer->Add(ModeGridSizer,0,wxEXPAND,0);

  TopSizer->Add(ModeTopSizer,0,wxEXPAND | wxALL,1);

  wxStaticBoxSizer *AlphaCtrlTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Transparency control")),wxVERTICAL);
  wxFlexGridSizer  *AlphaCtrlGridSizer = new wxFlexGridSizer(3,2,1,3);

  AlphaCtrlGridSizer->AddGrowableCol(1);

  wxCheckBox *AlphaCtrlEnabledCheckBox = new wxCheckBox(this,wxID_ALPHA_CTRL_MODE_CTRL,wxT(""));
  AlphaCtrlEnabledCheckBox->SetValue(Material->IsAlphaCtrlEnabled());
  AlphaCtrlGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Control enabled")),0,wxALIGN_CENTER_VERTICAL,0);
  AlphaCtrlGridSizer->Add(AlphaCtrlEnabledCheckBox,1,wxALIGN_RIGHT,0);

  spin_slider = new wxSpinSliderCtrl(this,wxID_ALPHA_FADEIN_CTRL,wxSPINSLIDER_MODE_FLOAT,Material->GetAlphaFadeIn(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  AlphaCtrlGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Fade-in")),0,wxALIGN_CENTER_VERTICAL,0);
  AlphaCtrlGridSizer->Add(spin_slider,1,wxALIGN_RIGHT,0);

  spin_slider = new wxSpinSliderCtrl(this,wxID_ALPHA_FADEOUT_CTRL,wxSPINSLIDER_MODE_FLOAT,Material->GetAlphaFadeOut(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  AlphaCtrlGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Fade-out")),0,wxALIGN_CENTER_VERTICAL,0);
  AlphaCtrlGridSizer->Add(spin_slider,1,wxALIGN_RIGHT,0);

  AlphaCtrlTopSizer->Add(AlphaCtrlGridSizer,0,wxEXPAND,0);

  TopSizer->Add(AlphaCtrlTopSizer,0,wxEXPAND | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

namespace {

class P3DColorParamEditCommand : public P3DEditCommand
 {
  public           :

                   P3DColorParamEditCommand
                                      (P3DMaterialInstanceSimple
                                                          *Material,
                                       float               R,
                                       float               G,
                                       float               B)
   {
    this->Material      = Material;
    this->NewR          = R;
    this->NewG          = G;
    this->NewB          = B;

    Material->GetColor(&OldR,&OldG,&OldB);
   }

  virtual void     Exec               ()
   {
    Material->SetColor(NewR,NewG,NewB);

    P3DApp::GetApp()->InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    Material->SetColor(OldR,OldG,OldB);

    P3DApp::GetApp()->InvalidatePlant();
   }

  private          :

  P3DMaterialInstanceSimple           *Material;
  float                                NewR,NewG,NewB;
  float                                OldR,OldG,OldB;
 };
}


void               P3DMaterialStdPanel::OnBaseColorClicked
                                      (wxCommandEvent     &event)
 {
  wxColourData                         ColorData;

  wxButton *ColorButton = (wxButton*)FindWindow(wxID_BASECOLOR_CTRL);

  if (ColorButton == 0)
   {
    return;
   }

  ColorData.SetColour(ColorButton->GetBackgroundColour());

  wxColourDialog                       Dialog(this,&ColorData);

  if (Dialog.ShowModal() == wxID_OK)
   {
    wxColourData UserColorData = Dialog.GetColourData();

    ColorButton->SetBackgroundColour(UserColorData.GetColour());

    wxColour Color = UserColorData.GetColour();

    P3DApp::GetApp()->ExecEditCmd
     (new P3DColorParamEditCommand
           (Material,
            Color.Red()   / 255.0f,
            Color.Green() / 255.0f,
            Color.Blue()  / 255.0f));
   }
 }

typedef P3DParamEditCmdTemplate<P3DStemModelTube,float> P3DStemTubeFloatParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DStemModelTube,unsigned int> P3DStemTubeUIntParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DMaterialInstanceSimple,bool> P3DMaterialBoolParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DMaterialInstanceSimple,float> P3DMaterialFloatParamEditCmd;

void               P3DMaterialStdPanel::OnUScaleChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
        (StemModelTube,
         event.GetFloatValue(),
         StemModelTube->GetTexCoordUScale(),
         &P3DStemModelTube::SetTexCoordUScale));
 }

void               P3DMaterialStdPanel::OnVModeChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemTubeUIntParamEditCmd
        (StemModelTube,
         event.GetSelection() == 0 ? P3DTexCoordModeRelative :
                                     P3DTexCoordModeAbsolute,
         StemModelTube->GetTexCoordVMode(),
         &P3DStemModelTube::SetTexCoordVMode));
 }

void               P3DMaterialStdPanel::OnVScaleChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
        (StemModelTube,
         event.GetFloatValue(),
         StemModelTube->GetTexCoordVScale(),
         &P3DStemModelTube::SetTexCoordVScale));
 }

void               P3DMaterialStdPanel::OnTexLayerClicked
                                      (wxCommandEvent     &event)
 {
  int                                  Id;

  Id = event.GetId();

  if ((Id < wxID_TEXLAYER_BUTTON_MIN) || (Id > wxID_TEXLAYER_BUTTON_MAX))
   {
    return;
   }

  if (event.GetInt())
   {
    ActiveTexLayer = Id - wxID_TEXLAYER_BUTTON_MIN;

    for (unsigned int TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
     {
      wxToggleButton *Button = (wxToggleButton*)FindWindow(wxID_TEXLAYER_BUTTON_MIN + TexLayer);

      if (Button != 0)
       {
        Button->SetValue(TexLayer == ActiveTexLayer);
       }
     }

    if (Material->GetTexHandle(ActiveTexLayer) == P3DTexHandleNULL)
     {
      BaseTexButton->SetBitmapLabel(P3DApp::GetApp()->GetBitmap(P3D_BITMAP_NO_TEXTURE));
      RemoveTexButton->Enable(FALSE);
     }
    else
     {
      const wxBitmap *TexBitmap;

      TexBitmap = P3DApp::GetApp()->GetTexManager()->GetBitmap(Material->GetTexHandle(ActiveTexLayer));

      BaseTexButton->SetBitmapLabel(*TexBitmap);

      RemoveTexButton->Enable(TRUE);
     }
   }
  else
   {
    wxToggleButton *Button = (wxToggleButton*)FindWindow(Id);

    if (Button != 0)
     {
      Button->SetValue(true);
     }
   }
 }

namespace {

class ChangeTextureCommand : public P3DEditCommand
 {
  public           :

                   ChangeTextureCommand
                                      (P3DMaterialInstanceSimple
                                                          *Material,
                                       unsigned int        Layer,
                                       P3DTexHandle        TexHandle)
   {
    this->Material      = Material;
    this->Layer         = Layer;
    this->TexHandle     = TexHandle;
   }

  virtual         ~ChangeTextureCommand
                                      ()
   {
    if (TexHandle != P3DTexHandleNULL)
     {
      P3DApp::GetApp()->GetTexManager()->FreeTexture(TexHandle);
     }
   }

  virtual void     Exec               ()
   {
    ChangeTexture();
   }

  virtual void     Undo               ()
   {
    ChangeTexture();
   }

  private          :

  void             ChangeTexture      ()
   {
    P3DTexHandle   CurrTexHandle;

    CurrTexHandle = Material->GetTexHandle(Layer);

    if (CurrTexHandle != P3DTexHandleNULL)
     {
      P3DApp::GetApp()->GetTexManager()->IncRefCount(CurrTexHandle);
     }

    Material->SetTexHandle(Layer,TexHandle);

    TexHandle = CurrTexHandle;

    P3DApp::GetApp()->InvalidatePlant();
   }

  P3DMaterialInstanceSimple           *Material;
  unsigned int                         Layer;
  P3DTexHandle                         TexHandle;
 };
}

void               P3DMaterialStdPanel::OnBaseTexClicked
                                      (wxCommandEvent     &event)
 {
  wxString                             FileName;
  P3DTexHandle                         TexHandle;
  unsigned int                         ImgFormatCount;
  unsigned int                         ImgFormatIndex;
  wxString                             FileWildcard;

  ImgFormatCount = P3DApp::GetApp()->GetTexManager()->GetFmtHandler()->FormatCount();

  wxString  FormatExt;
  wxString  AllImgFormatExt;
  wxString  WildPrefix(wxT("*."));

  for (ImgFormatIndex = 0; ImgFormatIndex < ImgFormatCount; ImgFormatIndex++)
   {
    FormatExt = wxString(P3DApp::GetApp()->GetTexManager()->GetFmtHandler()->FormatExt(ImgFormatIndex),wxConvUTF8);

    FileWildcard += wxString::Format(wxT("%s files (*.%s)|*.%s|"),
                                     FormatExt.Upper().c_str(),
                                     FormatExt.c_str(),
                                     FormatExt.c_str());

    if (ImgFormatIndex > 0)
     {
      AllImgFormatExt += wxT(";");
     }

    AllImgFormatExt += WildPrefix + FormatExt;
   }

  FileWildcard  = wxString(wxT("All images (")) + AllImgFormatExt +
                   wxT(")|") + AllImgFormatExt + wxT("|") + FileWildcard;

  FileWildcard += wxT("All files(*.*)|*.*");

  FileName = ::wxFileSelector(wxT("Select texture file"),
                              wxEmptyString,
                              wxEmptyString,
                              wxEmptyString,
                              FileWildcard,
                              wxOPEN | wxFILE_MUST_EXIST);

  if (!FileName.empty())
   {
    wxString                  ErrorMessage;

    TexHandle = P3DApp::GetApp()->GetTexManager()->LoadFromFile(FileName.mb_str(),ErrorMessage);

    if (TexHandle == P3DTexHandleNULL)
     {
      ::wxMessageBox(ErrorMessage,wxT("Error"),wxOK | wxICON_ERROR,this);

      return;
     }

    BaseTexButton->SetBitmapLabel(*(P3DApp::GetApp()->GetTexManager()->GetBitmap(TexHandle)));

    wxToggleButton *Button = (wxToggleButton*)FindWindow(wxID_TEXLAYER_BUTTON_MIN + ActiveTexLayer);

    if (Button != 0)
     {
      Button->SetForegroundColour(GetUsedTexTextColour());
     }

    RemoveTexButton->Enable(TRUE);

    P3DApp::GetApp()->ExecEditCmd
     (new ChangeTextureCommand(Material,ActiveTexLayer,TexHandle));
   }
 }

void               P3DMaterialStdPanel::OnRemoveTexClicked
                                      (wxCommandEvent     &event)
 {
  RemoveTexButton->Enable(FALSE);

  BaseTexButton->SetBitmapLabel(P3DApp::GetApp()->GetBitmap(P3D_BITMAP_NO_TEXTURE));

  wxToggleButton *Button = (wxToggleButton*)FindWindow(wxID_TEXLAYER_BUTTON_MIN + ActiveTexLayer);

  if (Button != 0)
   {
    Button->SetForegroundColour(GetUnusedTexTextColour());
   }

  P3DApp::GetApp()->ExecEditCmd
   (new ChangeTextureCommand(Material,ActiveTexLayer,P3DTexHandleNULL));
 }

void               P3DMaterialStdPanel::OnDoubleSidedChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DMaterialBoolParamEditCmd
         (Material,
          event.IsChecked(),
          Material->IsDoubleSided(),
          &P3DMaterialInstanceSimple::SetDoubleSided));
 }

void               P3DMaterialStdPanel::OnTransparentChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DMaterialBoolParamEditCmd
         (Material,
          event.IsChecked(),
          Material->IsTransparent(),
          &P3DMaterialInstanceSimple::SetTransparent));
 }

namespace {

class ChangeBillboardModeCommand : public P3DEditCommand
 {
  public           :

                   ChangeBillboardModeCommand
                                      (P3DMaterialInstanceSimple
                                                          *Material,
                                       P3DStemModelQuad   *StemModelQuad,
                                       unsigned int        Mode)
   {
    this->Material      = Material;
    this->StemModelQuad = StemModelQuad;
    this->Mode          = Mode;
   }

  virtual void     Exec               ()
   {
    ChangeMode();
   }

  virtual void     Undo               ()
   {
    ChangeMode();
   }

  private          :

  void             ChangeMode         ()
   {
    unsigned int   OldMode;

    OldMode = Material->GetBillboardMode();

    if (StemModelQuad != 0)
     {
      StemModelQuad->SetBillboardMode(Mode);
     }

    Material->SetBillboardMode(Mode);

    Mode = OldMode;

    P3DApp::GetApp()->InvalidatePlant();
   }

  P3DMaterialInstanceSimple           *Material;
  P3DStemModelQuad                    *StemModelQuad;
  unsigned int                         Mode;
 };
}


void               P3DMaterialStdPanel::OnBillboardModeChanged
                                      (wxCommandEvent     &event)
 {
  unsigned int                         BillboardMode;

  if      (event.GetSelection() == 1)
   {
    BillboardMode = P3D_BILLBOARD_MODE_SPHERICAL;
   }
  else if (event.GetSelection() == 2)
   {
    BillboardMode = P3D_BILLBOARD_MODE_CYLINDRICAL;
   }
  else
   {
    BillboardMode = P3D_BILLBOARD_MODE_NONE;
   }

  P3DApp::GetApp()->ExecEditCmd
   (new ChangeBillboardModeCommand
         (Material,StemModelQuad,BillboardMode));
 }

void               P3DMaterialStdPanel::OnAlphaCtrlEnabledChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DMaterialBoolParamEditCmd
         (Material,
          event.IsChecked(),
          Material->IsAlphaCtrlEnabled(),
          &P3DMaterialInstanceSimple::SetAlphaCtrlState));
 }

void               P3DMaterialStdPanel::OnAlphaFadeInChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DMaterialFloatParamEditCmd
         (Material,
          event.GetFloatValue(),
          Material->GetAlphaFadeIn(),
          &P3DMaterialInstanceSimple::SetAlphaFadeIn));
 }

void               P3DMaterialStdPanel::OnAlphaFadeOutChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DMaterialFloatParamEditCmd
         (Material,
          event.GetFloatValue(),
          Material->GetAlphaFadeOut(),
          &P3DMaterialInstanceSimple::SetAlphaFadeOut));
 }

#define model Material

void               P3DMaterialStdPanel::UpdateControls
                                      ()
 {
  wxButton *Button;
  wxChoice *Choice;

  Button = (wxButton*)FindWindow(wxID_BASECOLOR_CTRL);

  if (Button != NULL)
   {
    float     R,G,B;

    Material->GetColor(&R,&G,&B);

    Button->SetBackgroundColour(wxColour((int)(R * 255.0f),(int)(G * 255.0f),(int)(B * 255.0f)));
   }

  P3DUpdateParamSpinSliderExt(StemModelTube,wxID_USCALE_CTRL,GetTexCoordUScale);

  Choice = (wxChoice*)FindWindow(wxID_VMODE_CTRL);

  if (Choice != NULL)
   {
    Choice->SetSelection
     (StemModelTube->GetTexCoordVMode() == P3DTexCoordModeRelative ? 0 : 1);
   }

  P3DUpdateParamSpinSliderExt(StemModelTube,wxID_VSCALE_CTRL,GetTexCoordVScale);

  for (unsigned int TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    wxToggleButton *Button = (wxToggleButton*)FindWindow(wxID_TEXLAYER_BUTTON_MIN + TexLayer);

    if (Button != 0)
     {
      P3DTexHandle TexHandle;

      TexHandle = Material->GetTexHandle(TexLayer);

      if (TexHandle != P3DTexHandleNULL)
       {
        Button->SetForegroundColour(GetUsedTexTextColour());

        if (TexLayer == ActiveTexLayer)
         {
          const wxBitmap *TexBitmap;

          TexBitmap = P3DApp::GetApp()->GetTexManager()->GetBitmap(TexHandle);

          BaseTexButton->SetBitmapLabel(*TexBitmap);

          RemoveTexButton->Enable(TRUE);
         }
       }
      else
       {
        Button->SetForegroundColour(GetUnusedTexTextColour());

        if (TexLayer == ActiveTexLayer)
         {
          BaseTexButton->SetBitmapLabel
           (P3DApp::GetApp()->GetBitmap(P3D_BITMAP_NO_TEXTURE));
          RemoveTexButton->Enable(FALSE);
         }
       }
     }
   }

  P3DUpdateParamCheckBox(wxID_DOUBLESIDED_CTRL,IsDoubleSided);
  P3DUpdateParamCheckBox(wxID_TRANSPARENT_CTRL,IsTransparent);

  Choice = (wxChoice*)FindWindow(wxID_BILLBOARD_MODE_CTRL);

  if (Choice != NULL)
   {
    unsigned int BillboardMode;

    BillboardMode = Material->GetBillboardMode();

    if      (BillboardMode == P3D_BILLBOARD_MODE_SPHERICAL)
     {
      Choice->SetSelection(1);
     }
    else if (BillboardMode == P3D_BILLBOARD_MODE_CYLINDRICAL)
     {
      Choice->SetSelection(2);
     }
    else
     {
      Choice->SetSelection(0);
     }
   }

  P3DUpdateParamCheckBox(wxID_ALPHA_CTRL_MODE_CTRL,IsAlphaCtrlEnabled);

  P3DUpdateParamSpinSliderExt(Material,wxID_ALPHA_FADEIN_CTRL,GetAlphaFadeIn);
  P3DUpdateParamSpinSliderExt(Material,wxID_ALPHA_FADEOUT_CTRL,GetAlphaFadeOut);
 }

#undef model

