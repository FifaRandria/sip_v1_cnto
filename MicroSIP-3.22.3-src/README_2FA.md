# MicroSIP - Implantation de la double authentification (2FA)

## Contexte

MicroSIP est utilise par des agents de call center. Les responsables veulent
restreindre l'acces a certaines fonctions sensibles :

- **Parametres** (menu -> Settings, raccourci Ctrl+P)
- **Ajouter un compte** (Ctrl+M)
- **Editer un compte** / **Editer le compte local** (Ctrl+L)
- **Auto-ouverture du compte** quand aucun compte n'est configure

Chaque tentative d'acces declenche une boite de dialogue demandant un **code
2FA a 6 chiffres** (TOTP, RFC 6238), genere par une application
d'authentification (Google Authenticator, Microsoft Authenticator, Authy...).

Aucun fichier de configuration n'est utilise : le secret est **ecrit en dur
dans le code source** et compile dans `microsip.exe`.

---

## Fichiers crees

| Fichier | Role |
|---|---|
| `lib/TOTP.h` | Interface du module TOTP : `generate()` et `verify()`. |
| `lib/TOTP.cpp` | Implementation autonome TOTP (RFC 6238) : Base32, HMAC-SHA1, generation et verification du code. Aucune dependance externe (SHA-1/HMAC integres, pas de lien OpenSSL). |

## Fichiers modifies

| Fichier | Changement |
|---|---|
| `mainDlg.cpp` | Nouvelle fonction `CheckAdminAuth()`, secret TOTP en dur, verification sur les 5 points d'acces. |
| `mainDlg.h` | Declaration de `bool CheckAdminAuth();` (ligne 173). |
| `PasswordDlg.h` / `PasswordDlg.cpp` | La boite de dialogue demande le code 2FA a 6 chiffres (membre `CString code`, champ non masque). |
| `res/dialog.rc2` | Ressource `IDD_PASSWORD` : titre "Two-Factor Authentication", libelle "Enter 2FA code:", champ sans masquage. |
| `microsip.vcxproj` | Enregistrement de `lib/TOTP.cpp` (ligne 268) et `lib/TOTP.h` (ligne 325). |
| `microsip.vcxproj.filters` | Enregistrement de `lib/TOTP.cpp` (ligne 114) et `lib/TOTP.h` (ligne 287) dans les filtres du projet. |

---

## Fonction cle : `CmainDlg::CheckAdminAuth()`

**Declaration** : `mainDlg.h:173`
**Definition** : `mainDlg.cpp:2220`

```cpp
bool CmainDlg::CheckAdminAuth()
{
    const char* TOTPSecret = "BIENVEILLANCECONNECTEO";
    PasswordDlg dlg(this);
    if (dlg.DoModal() == IDOK) {
        std::string code = (LPCSTR)CStringA(dlg.code);
        if (TOTP::verify(TOTPSecret, code)) {
            return true;
        }
        AfxMessageBox(Translate(_T("Invalid 2FA code. Access denied.")), MB_ICONWARNING);
    }
    return false;
}
```

Deroulement :

1. Lit le secret Base32 stocke en dur dans la constante `TOTPSecret`.
2. Ouvre la boite `PasswordDlg` (dialogue modal).
3. Verifie le code avec `TOTP::verify(secret, code)` :
   - compteur = temps UNIX / 30 secondes ;
   - comparaison sur le pas courant **+/- 1 pas** (tolerance d'horloge) ;
   - code valide -> `true` (acces autorise) ;
   - code invalide -> message "Invalid 2FA code. Access denied." et `false`.
4. Si l'utilisateur annule la boite, retourne `false`.

> **IMPORTANT** : avant livraison, remplacer la valeur de `TOTPSecret`
> (ligne 2222 de `mainDlg.cpp`) par un secret Base32 aleatoire (32 caracteres
> conseilles). Le meme secret doit etre enregistre dans l'application
> d'authentification des responsables (TOTP, 30 s, 6 chiffres).

---

## Points d'acces proteges

Chacune de ces fonctions appelle en premier `if (!CheckAdminAuth()) return;`
et sont donc bloquees sans code 2FA valide.

| Fonction | Ligne | Action protegee |
|---|---|---|
| `CmainDlg::OnMenuAccountAdd()` | `mainDlg.cpp:2234` | "Ajouter un compte..." |
| `CmainDlg::OnMenuAccountEdit(UINT nID)` | `mainDlg.cpp:2250` | "Editer un compte" (tous les comptes) |
| `CmainDlg::OnMenuAccountLocalEdit()` | `mainDlg.cpp:2285` | "Editer le compte local" |
| `CmainDlg::OnMenuSettings()` | `mainDlg.cpp:2307` | "Parametres" |
| `CmainDlg::OnAccount(WPARAM, LPARAM)` | `mainDlg.cpp:2747` | Auto-ouverture du compte (aucun compte configure) |

Remarques :

- Les raccourcis clavier **Ctrl+P / Ctrl+M / Ctrl+L** passent par ces memes
  fonctions (vues dans la table des commandes, ex. `mainDlg.cpp:1691` pour
  `ID_ACCOUNT_EDIT_LOCAL` et `mainDlg.cpp:1695` pour `ID_SETTINGS`), ils sont
  donc automatiquement proteges.
- Le menu contextuel de l'icone de la barre des taches utilise le meme code.
- Le code est demande a **chaque acces** (aucune memorisation de session).

---

## Module TOTP : `lib/TOTP.cpp` / `lib/TOTP.h`

API publique (namespace `TOTP`) :

| Fonction | Role |
|---|---|
| `generate(secretBase32, timeStep, digits)` | Genere le code TOTP courant a partir du secret Base32. |
| `verify(secretBase32, code, timeStep, digits, window)` | Verifie un code saisi ; tolere un ecart de +/- `window` pas de temps (defaut 1). |

Fonctions internes de l'implementation :

- `decodeBase32()` : decodage du secret en octets (ignore espaces, `-`, `=`).
- `sha1Init()` / `sha1Transform()` / `sha1Update()` / `sha1Final()` /
  `sha1Hash()` : implementation SHA-1 integree.
- `hmacSha1()` : HMAC-SHA1 (vecteurs de test RFC 4226 / RFC 6238 valides).
- `totpFromCounter()` : troncature dynamique + code a N chiffres (6 par defaut).

Utilisation dans `mainDlg.cpp` (include ligne 36) :

```cpp
#include "TOTP.h"
```

---

## Compilation et livraison

```bat
MSBuild.exe microsip.vcxproj /p:Configuration=Release /p:Platform=Win32
```

Binaire genere : `Release\microsip.exe`

Note : si le lien echoue avec "LNK1104: impossible d'ouvrir microsip.exe",
fermer le programme en cours d'execution (ou l'antivirus le verrouille) et
relancer.

---

## Test rapide

1. Ouvrir "Parametres" -> la boite de code 2FA s'affiche.
2. Saisir un code incorrect -> message "Access denied", rien ne s'ouvre.
3. Saisir le code valide de l'application d'authentification -> la fenetre
   Parametres s'ouvre.
4. Fermer puis re-cliquer : un nouveau code est demande (pas de memorisation).
5. Verifier "Ajouter un compte...", "Editer un compte" et Ctrl+P / Ctrl+M de
   la meme maniere.
6. Decaler l'horloge de la machine d'une vingtaine de secondes : le code du
   pas precedent reste accepte (fenetre de tolerance +/- 1 pas).
