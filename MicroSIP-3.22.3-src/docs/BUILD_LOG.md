# MicroSIP Build Log

## Objectif
Compiler le projet MicroSIP 3.22.3 depuis VS Code avec le toolchain MSVC.

## Environnement
- OS: Windows
- Shell: PowerShell
- IDE: VS Code
- Date de début: 27/07/2026

---

## Étape 1 : Installation des outils de compilation

### Visual Studio Build Tools 2022
- **Statut** : En cours
- **Action** : Installation de "Desktop development with C++" (inclut MSVC, Windows SDK, MFC)
- **Fichier** : `vs_BuildTools.exe` depuis https://aka.ms/vs/17/release/vs_BuildTools.exe

---

## Étape 2 : Installation des extensions VS Code

### Extension C/C++
- **Statut** : À faire
- **Commande** : `code --install-extension ms-vscode.cpptools`

---

## Étape 3 : Installation de NuGet CLI

- **Statut** : À faire
- **Destination** : `C:\tools\nuget\nuget.exe`
- **Commande** : `Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" -OutFile "C:\tools\nuget\nuget.exe"`

---

## Étape 4 : Restauration des paquets NuGet

- **Statut** : À faire
- **Paquet requis** : Microsoft.Web.WebView2 v1.0.3351.48
- **Commande** : `nuget restore microsip.vcxproj`

---

## Étape 5 : Compilation

- **Statut** : À faire
- **Configuration** : Release | Win32
- **Toolset** : v140 (rétrocompatible via Build Tools 2022)
- **Commande** : `vcvarsall.bat x86 && msbuild microsip.vcxproj /p:Configuration=Release /p:Platform=Win32`
- **Sortie attendue** : `Release\microsip.exe`

---

## Modifications apportées
(Aucune modification au code source prévu — compilation telle quelle)
