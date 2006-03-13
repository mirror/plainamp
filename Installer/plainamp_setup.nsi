; Script generated by the HM NIS Edit Script Wizard.
; http://hmne.sourceforge.net/

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Plainamp"
!define PRODUCT_VERSION "0.2.1"
!define PRODUCT_PUBLISHER "Hartwork Project"
!define PRODUCT_WEBSITE "http://www.hartwork.org"
!define PRODUCT_WEBSITE_LINKFILE "Website.url"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Plainamp.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_UNINST_EXE "UninstPA.exe"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor lzma



; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING

; MUI Settings / Icons
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install-nsis.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall-nsis.ico"

; MUI Settings / Header
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r-nsis.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r-nsis.bmp"

; MUI Settings / Wizard
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-nsis.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall-nsis.bmp"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Plainamp"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
; !insertmacro MUI_PAGE_FINISH
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------


Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "plainamp_setup.exe"
InstallDir "$PROGRAMFILES\Plainamp"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUninstDetails show


Var /GLOBAL bDelete



Function OpenConfirmLookup
  ; ----------------------------------------------------
  ; $0 - path without trailing slash (e.g. "C:\test")
  ; $1 - filename (e.g. "test.dll")
  ; $2 - Should-be-copied flag
  ; ----------------------------------------------------
  IfFileExists "$0\$1" 0 +2
  MessageBox MB_ICONINFORMATION|MB_YESNO|MB_DEFBUTTON1 'File "$1" already exists.  $\n$\nOverwrite?' IDYES +1 IDNO +3
  Push 1
  Goto +2
  Push 0
  Pop $2
FunctionEnd

Function CloseConfirmLookup
  ; ----------------------------------------------------
  ; $0 - path without trailing slash (e.g. "C:\test")
  ; $1 - filename (e.g. "test.dll")
  ; $2 - Should-be-copied flag
  ; ----------------------------------------------------
  IntCmp 0 $2 +3
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NSIS:Delete___$1" 1
  Goto +7
  DetailPrint "Skipped: $1"
  ClearErrors
  ReadRegDWORD $bDelete ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NSIS:Delete___$1"
  IfErrors +2 0
  IntCmp 1 $bDelete +2 0
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NSIS:Delete___$1" 0
FunctionEnd



Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  File "..\Binary\Plainamp.exe"
  File "Instdir\Plainamp.exe.manifest"

  ; Create ini and activate <out_wave_gpl.dll>
  WriteINIStr "$INSTDIR\Plainamp.ini" "Plainamp" "OutputPluginActive___out_wave_gpl.dll" "1"
  DetailPrint "Extract: Plainamp.ini... 100%"

  File "..\Binary\fftw3.dll"
  File "..\Binary\zlib1.dll"
  File "..\Changelog.txt"


  ; CreateDirectory "$INSTDIR\Plugins"
  SetOutPath "$INSTDIR\Plugins"  
  SetOverwrite on
  
  Push $0
  Push $1
  Push $2

; -------------------------------------
  StrCpy $0 "$INSTDIR\Plugins"
; -------------------------------------
  StrCpy $1 "in_mad.dll"
  Call OpenConfirmLookup
  IntCmp 0 $2 +2
  File "Instdir\Plugins\in_mad.dll"
  Call CloseConfirmLookup

  StrCpy $1 "out_wave_gpl.dll"
  Call OpenConfirmLookup
  IntCmp 0 $2 +2
  File "Instdir\Plugins\out_wave_gpl.dll"
  Call CloseConfirmLookup

  Pop $2
  Pop $1
  Pop $0

  File "Instdir\Plugins\vis_plainbar.dll"


; Shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Plainamp.lnk" "$INSTDIR\Plainamp.exe"
  CreateShortCut "$DESKTOP\Plainamp.lnk" "$INSTDIR\Plainamp.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\${PRODUCT_WEBSITE_LINKFILE}" "InternetShortcut" "URL" "${PRODUCT_WEBSITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Website.lnk" "$INSTDIR\${PRODUCT_WEBSITE_LINKFILE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\${PRODUCT_UNINST_EXE}"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\${PRODUCT_UNINST_EXE}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Plainamp.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\${PRODUCT_UNINST_EXE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Plainamp.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEBSITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


;Function un.onUninstSuccess
;  HideWindow
;  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
;FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd



Function un.DeleteRetry
  ; ----------------------------------------------------
  ; $0 - path without trailing slash (e.g. "C:\test")
  ; $1 - filename (e.g. "test.dll")
  ; ----------------------------------------------------
  ClearErrors
  Delete "$0\$1"
  IfErrors 0 +2
  MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL|MB_DEFBUTTON1 'Error deleting file "$1".  $\n$\nTry again?' IDRETRY -3
FunctionEnd

Function un.DeleteRetryLookup
  ; ----------------------------------------------------
  ; $0 - path without trailing slash (e.g. "C:\test")
  ; $1 - filename (e.g. "test.dll")
  ; ----------------------------------------------------
  ClearErrors
  ReadRegDWORD $bDelete ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NSIS:Delete___$1"
  IfErrors +4 0
  IntCmp 0 $bDelete 0 +3
  DetailPrint "Skipped: $1"
  Goto +5
  ClearErrors
  Delete "$0\$1"
  IfErrors 0 +2
  MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL|MB_DEFBUTTON1 'Error deleting file "$1".  $\n$\nTry again?' IDRETRY -3
FunctionEnd



Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP

  Push $0
  Push $1
  
; -------------------------------------
  StrCpy $0 "$INSTDIR"
; -------------------------------------
  StrCpy $1 "Plainamp.exe"
  Call un.DeleteRetry
  StrCpy $1 "Plainamp.exe.manifest"
  Call un.DeleteRetry
  StrCpy $1 "Plainamp.ini"
  Call un.DeleteRetry
  StrCpy $1 "Plainamp.m3u"
  Call un.DeleteRetry
  StrCpy $1 "fftw3.dll"
  Call un.DeleteRetry
  StrCpy $1 "zlib1.dll"
  Call un.DeleteRetry
  StrCpy $1 "Changelog.txt"
  Call un.DeleteRetry

  StrCpy $1 "${PRODUCT_WEBSITE_LINKFILE}"
  Call un.DeleteRetry
  StrCpy $1 ${PRODUCT_UNINST_EXE}
  Call un.DeleteRetry

; -------------------------------------
  StrCpy $0 "$INSTDIR\Plugins"
; -------------------------------------
  StrCpy $1 "in_mad.dll"
  Call un.DeleteRetryLookup
  StrCpy $1 "out_wave_gpl.dll"
  Call un.DeleteRetryLookup
  StrCpy $1 "vis_plainbar.dll"
  Call un.DeleteRetry

; -------------------------------------
  StrCpy $0 "$SMPROGRAMS\$ICONS_GROUP"
; -------------------------------------
  StrCpy $1 "Uninstall.lnk"
  Call un.DeleteRetry
  StrCpy $1 "Website.lnk"
  Call un.DeleteRetry
  StrCpy $1 "Plainamp.lnk"
  Call un.DeleteRetry

; -------------------------------------
  StrCpy $0 "$DESKTOP"
; -------------------------------------
  StrCpy $1 "Plainamp.lnk"
  Call un.DeleteRetry

  Pop $1
  Pop $0

  RMDir "$SMPROGRAMS\$ICONS_GROUP"
  RMDir "$INSTDIR\Plugins"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  ; SetAutoClose true
SectionEnd
