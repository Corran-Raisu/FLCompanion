; FLCompanion - Installation script

!define SHARED			"external"
!define TARGETDIR		"Release_UNICODE"
!define MAINEXE			"FLCompanion.exe"
!define COPY_EXE_VERSION
!execute '${SHARED}\bin\PETool.exe "${TARGETDIR}\${MAINEXE}" /NSIS FLCompanion.nsh'
!include "FLCompanion.nsh"
!addplugindir "${SHARED}\bin"
!define APP_GUID		"FLCompanion-{0A8EB4BA-8147-460B-9B0C-6D5B32F3FF41}"
!define UNINSTALLKEY    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_GUID}"

; General --------------------------------

    ;Name and file
    Name "${PRODUCTNAME} ${VERSION}"
    OutFile "${TAGNAME}.exe"
    SetCompressor lzma
    
    ;Default installation folder
    InstallDir "$PROGRAMFILES\${PRODUCTNAME}"
    
    ;Get installation folder from registry if available
    InstallDirRegKey HKLM "${UNINSTALLKEY}" "InstallLocation"
    
    BrandingText " "
    XPStyle on

; UI Setup --------------------------------

;	SpaceTexts none

; Pages --------------------------------

;	!define MUI_PAGE_CUSTOMFUNCTION_LEAVE CheckInstalled
;	!define MUI_PAGE_HEADER_TEXT "Welcome to the $(^NameDA) Setup Wizard"
;	!define MUI_PAGE_HEADER_SUBTEXT "This wizard will guide you through the installation of $(^NameDA)."
;	!define MUI_LICENSEPAGE_TEXT_TOP "Versions history :"
;	!define MUI_LICENSEPAGE_TEXT_BOTTOM "It is recommended that you close all other applications before installation.$\r$\n$\r$\n$_CLICK"
;	!define MUI_LICENSEPAGE_BUTTON "$(^NextBtn)"
;	!insertmacro MUI_PAGE_LICENSE "${CUSTOEMP_DIR}\WhatsNew.txt"
;	!insertmacro MUI_PAGE_DIRECTORY
;	!insertmacro MUI_PAGE_INSTFILES
;	!insertmacro MUI_PAGE_FINISH

	PageEx license
		Caption " "
		;LicenseText "$(WhatsNew)" "$(^NextBtn)"
		PageCallbacks "" "" CheckInstalled
		LicenseData ReadMe.txt
	PageExEnd
	Page directory
	Page instfiles

; Languages --------------------------------

	LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
	LangString WhatsNew						0	"What's new in this version:"
	LangString AskUninstall					0	"A previous installation of ${PRODUCTNAME} (version $R1) was found.$\n$\nDo you want to uninstall it? (recommended)"
	LangString StillActive					0	"Unable to uninstall ${PRODUCTNAME}$\nbecause the program seems to be currently running$\nor you don't have administrator rights."
	LangString OnlyRecentOS					0	"${PRODUCTNAME} will only run on Windows 2000, XP or more recent"
  	LangString MUI_UNTEXT_ABORT_SUBTITLE 	0	"Uninstall of previous version was not completed successfully."

;	LoadLanguageFile "${NSISDIR}\Contrib\Language files\French.nlf"
;	LangString WhatsNew						0	"Liste des nouveautés de cette version :"
;	LangString AskUninstall					0	"Une précédente installation de ${PRODUCTNAME} (version $R1) a été détectée.$\n$\nVoulez-vous la désinstaller ? (recommandé)"
;	LangString StillActive					0	"Impossible de désinstaller ${PRODUCTNAME}$\ncar le programme est en cours d'exécution."
;	LangString OnlyRecentOS					0	"${PRODUCTNAME} ne fonctionne que sous Windows 2000, XP ou plus récent"
;  	LangString MUI_UNTEXT_ABORT_SUBTITLE	0	"La désinstallation de la précédente version n'a pas été terminée."

; Installer Functions --------------------------------

Var WINDOWS_VERSION

Function .onInit
	InitPluginsDir
	ClearErrors
	ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" "CurrentVersion"
	IfErrors bad_windows
	StrCpy $WINDOWS_VERSION $R0 1
	IntOp $WINDOWS_VERSION $WINDOWS_VERSION * 100
	StrCpy $R0 $R0 "" 2
	IntOp $WINDOWS_VERSION $WINDOWS_VERSION + $R0
	IntCmp $WINDOWS_VERSION 500 0 bad_windows
	
	Call CheckAdmin
	
	Return
bad_windows:
	MessageBox MB_OK|MB_ICONSTOP $(OnlyRecentOS)
	Abort
FunctionEnd

Function CheckAdmin
	File "/oname=$PLUGINSDIR\IHelper.exe" "${SHARED}\bin\IHelper.exe"
	ExecWait '"$PLUGINSDIR\IHelper.exe" /NSIS "$EXEDIR" $CMDLINE' $0
	IntCmp $0 -2 +2 ; we are admin, skip Quit & proceed to installation
		Quit
FunctionEnd

Function CheckInstalled
	ClearErrors
	ReadRegStr $R1 HKLM "${UNINSTALLKEY}" "DisplayVersion"
	IfErrors uninstallOver
	MessageBox MB_YESNO|MB_ICONQUESTION $(AskUninstall) /SD IDYES IDNO uninstallOver

	ReadRegStr $R0 HKLM "${UNINSTALLKEY}" "UninstallString"
	ReadRegStr $R1 HKLM "${UNINSTALLKEY}" "InstallLocation"
retryUninstall:
	ClearErrors
	CopyFiles $R0 $PLUGINSDIR\Uninstall.exe
	ExecWait '"$PLUGINSDIR\Uninstall.exe" /S /SHOWPROGRESS _?=$R1'
	IfErrors uninstallError
	Delete "$PLUGINSDIR\Uninstall.exe"
;	MessageBox MB_OK|MB_ICONINFORMATION $(MUI_UNTEXT_FINISH_SUBTITLE) /SD IDOK
	Return
uninstallError:
	MessageBox MB_RETRYCANCEL|MB_ICONSTOP $(MUI_UNTEXT_ABORT_SUBTITLE) /SD IDOK IDRETRY retryUninstall
	Abort
uninstallOver:
FunctionEnd

; Installer Sections --------------------------------

Section "!$(^Name)"
SectionIn RO
	IfSilent 0 checkInstalled
	Call CheckInstalled
checkInstalled:

	SetShellVarContext all
	nsisDDE::Execute "${APP_GUID}" "[Quit]"
	Pop $0
	nsisDDE::Execute "/TOPIC=${INTERNALNAME}" "Check4Update-{6B6BDEFF-569D-449E-9707-FED242753DF2}" "[Quit]"
	Pop $1
	IntCmp "$0$1" 0 notRunning
	DetailPrint "Exiting running instances..."
	Sleep 2000
notRunning:

	; Game installation is really done in Section "-Main Program" below
SectionEnd

Section "-Main Program"
	SetOutPath "$INSTDIR"
	WriteUninstaller "Uninstall.exe"
	File "${SHARED}\bin\Check4Update.exe"
	File "${TARGETDIR}\${MAINEXE}"
	File "ReadMe.txt"
	; remove following lines after a few updates
	WriteRegStr	  HKCU "Software\Wizou\FLCompanion\Settings" "BaseCombo" ""
	WriteRegStr	  HKCU "Software\Wizou\FLCompanion\Settings" "SystemCombo" ""
SectionEnd

Section "-Create Shortcuts"
	CreateShortCut  "$SMPROGRAMS\${PRODUCTNAME}.lnk"   "$INSTDIR\${MAINEXE}"
SectionEnd

Section "-Register program information"
	;Setup Add/Remove Program information
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "DisplayName" "${PRODUCTNAME} ${VERSION}"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "DisplayIcon" "$INSTDIR\${MAINEXE}"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "Publisher" "${COMPANYNAME}"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "InstallSource" "$EXEDIR"
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "Readme" "$INSTDIR\ReadMe.txt"
!ifdef URLINFOABOUT
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "URLInfoAbout" "${URLINFOABOUT}"
!endif
!ifdef URLUPDATEINFO
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "URLUpdateInfo" "${URLUPDATEINFO}"
!endif
	WriteRegStr	  HKLM "${UNINSTALLKEY}" "DisplayVersion" "${VERSION}"
	WriteRegDWORD HKLM "${UNINSTALLKEY}" "VersionMajor" ${VERSION_MAJOR}
	WriteRegDWORD HKLM "${UNINSTALLKEY}" "VersionMinor" ${VERSION_MINOR}
	WriteRegDWORD HKLM "${UNINSTALLKEY}" "NoModify" 1
	WriteRegDWORD HKLM "${UNINSTALLKEY}" "NoRepair" 1
SectionEnd

Section "-Finish"
	SetDetailsPrint textonly
	DetailPrint "Completed - Click 'Close' to run the program"
	SetDetailsPrint listonly
SectionEnd

Function .onInstSuccess
	; start program in user-mode
	SetOutPath "$INSTDIR"
	nsisDDE::Execute "IHelper-{3968D359-C488-4414-944F-C7B2AB06BA69}" 'UserExec "$INSTDIR\${MAINEXE}"'
	Pop $0
	IntCmp $0 0 0 0 +2
		Exec "$INSTDIR\${MAINEXE}"
FunctionEnd

; Uninstaller Functions --------------------------------

Function un.onInit
	InitPluginsDir
	File "/oname=$PLUGINSDIR\IHelper.exe" "${SHARED}\bin\IHelper.exe"
	ExecWait '"$PLUGINSDIR\IHelper.exe" /NSIS "$EXEDIR" $CMDLINE _?=$INSTDIR' $0
	IntCmp $0 -2 +2 ; we are admin, skip Quit & proceed to installation
		Quit
FunctionEnd

; Uninstaller Section --------------------------------

Section "Uninstall"
	SetShellVarContext all
	nsisDDE::Execute "${APP_GUID}" "[Quit]"
	Pop $0
	nsisDDE::Execute "/TOPIC=${INTERNALNAME}" "Check4Update-{6B6BDEFF-569D-449E-9707-FED242753DF2}" "[Quit]"
	Pop $1
	IntCmp "$0$1" 0 notRunning
	DetailPrint "Exiting running instances..."
	Sleep 2000
notRunning:

	;Remove program files
	ClearErrors
	Delete "$INSTDIR\${MAINEXE}"
	IfErrors 0 removeOK
	MessageBox MB_OK|MB_ICONSTOP $(StillActive)
	SetAutoClose true
	Quit
removeOk:
	Delete "$INSTDIR\ReadMe.txt"
	Delete "$INSTDIR\Check4Update.exe"
	Delete "$INSTDIR\Uninstall.exe"
	;Remove program directory (only if directory is empty)
	RMDir "$INSTDIR"

	;Remove shortcuts
	Delete "$SMPROGRAMS\${PRODUCTNAME}.lnk"

	;Remove registration
	ReadRegStr $R0 HKLM "${UNINSTALLKEY}" "InstallLocation"
	StrCmp $R0 "$INSTDIR" 0 notMyPath
		;Delete Add/Remove Program information 
		DeleteRegKey HKLM "${UNINSTALLKEY}"
	notMyPath:
	
SectionEnd

