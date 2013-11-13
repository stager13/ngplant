Name "ngPlant"
OutFile ngPlant-0.9.11-setup.exe
InstallDir $PROGRAMFILES\ngPlant
InstallDirRegKey HKLM "Software\ngPlant" "Install_Dir"

PageEx license
 LicenseData COPYING
 LicenseForceSelection checkbox
PageExEnd

PageEx components
ComponentText
PageExEnd

PageEx directory
DirText
PageExEnd

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Base"
 SetOutPath $INSTDIR
 SectionIn RO
 File "COPYING"
 File "COPYING.BSD"
 File "INSTALL"
 File "README"
 File "README.extern"
 File "ReleaseNotes"
 File "ngplant.exe"
 File "ngpshot.exe"

 CreateDirectory $INSTDIR\samples
 CreateDirectory $INSTDIR\textures
 CreateDirectory $INSTDIR\shaders
 CreateDirectory $INSTDIR\plugins

 SetOutPath $INSTDIR\samples
 File "samples\palm.ngp"
 File "samples\spdcactus.ngp"
 File "samples\simplefern.ngp"

 SetOutPath $INSTDIR\textures
 File "textures\greenleaf.tga"
 File "textures\fernleaf.tga"

 SetOutPath $INSTDIR\shaders
 File "shaders\ngplant_default_fs.glsl"
 File "shaders\ngplant_default_vs.glsl"
 File "shaders\ngpshot_nor_fs.glsl"
 File "shaders\ngpshot_nor_vs.glsl"

 SetOutPath $INSTDIR\plugins
 File "plugins\export_dae.lua"
 File "plugins\export_obj.lua"
 File "plugins\gmesh_import_obj.lua"
 File "plugins\gmesh_rhomb.lua"
 File "plugins\gmesh_triangle.lua"

 SetOutPath $INSTDIR

 WriteRegStr HKLM SOFTWARE\ngPlant "Install_Dir" "$INSTDIR"

 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ngPlant" "DisplayName" "ngPlant"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ngPlant" "UninstallString" '"$INSTDIR\ngPlant-0.9.11-uninst.exe"'
 WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ngPlant" "NoModify" 1
 WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ngPlant" "NoRepair" 1

 WriteUninstaller $INSTDIR\ngPlant-0.9.11-uninst.exe
SectionEnd

Section "Texture library path setup"
  ClearErrors
  CreateDirectory "$PROFILE\.ngplant"
  IfErrors done
  FileOpen $0 $PROFILE\.ngplant\.ngplant w
  IfErrors done
  FileWrite $0 "[TexLocations]"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "Location0=$INSTDIR\textures"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "[Paths]"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "Plugins=$INSTDIR\plugins"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0
  done:
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\ngPlant"
  CreateShortCut "$SMPROGRAMS\ngPlant\Uninstall.lnk" "$INSTDIR\ngPlant-0.9.11-uninst.exe"
  CreateShortCut "$SMPROGRAMS\ngPlant\ngPlant.lnk" "$INSTDIR\ngplant.exe"
SectionEnd


Section "Uninstall"
 DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ngPlant"
 DeleteRegKey HKLM SOFTWARE\ngPlant

 Delete "$SMPROGRAMS\ngPlant\*.*"
 RMDir "$SMPROGRAMS\ngPlant"

 Delete $INSTDIR\COPYING
 Delete $INSTDIR\COPYING.BSD
 Delete $INSTDIR\INSTALL
 Delete $INSTDIR\README
 Delete $INSTDIR\README.extern
 Delete $INSTDIR\ReleaseNotes
 Delete $INSTDIR\ngpshot.exe
 Delete $INSTDIR\ngplant.exe
 Delete $INSTDIR\ngPlant-0.9.11-uninst.exe
SectionEnd
