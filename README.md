# README — Corrections de sécurité 2FA et gestion des secrets

## 1. Objet

Ce document décrit les corrections de sécurité apportées à la première version du code à la suite des vulnérabilités identifiées lors de la revue de sécurité.

Les modifications concernent principalement :

* La protection contre le brute-force du code 2FA/TOTP.
* Le masquage du code 2FA lors de sa saisie.
* La suppression des mots de passe présents en clair dans le package de déploiement.
* Le renforcement des contrôles avant livraison.

---

## 2. Corrections apportées

### Finding — Absence de protection contre le brute-force du 2FA

**Problème identifié :**

Dans la première version, la fonction `CheckAdminAuth()` ne possédait aucun mécanisme permettant de limiter les tentatives de saisie du code 2FA.

Un code TOTP à 6 chiffres possède 1 000 000 de combinaisons. Sans limitation, un attaquant disposant d'un accès au poste pouvait effectuer un grand nombre de tentatives automatiquement.

**Modifications apportées :**

La fonction `CheckAdminAuth()` a été renforcée afin d'intégrer :

* Un compteur des tentatives échouées.
* Une limitation du nombre de tentatives consécutives.
* Un verrouillage temporaire après plusieurs échecs.
* Un délai minimal entre deux tentatives.
* La réinitialisation du compteur après une authentification réussie.
* La journalisation des tentatives échouées dans l'Event Log Windows.

**Politique appliquée :**

* Nombre maximal de tentatives : **5**
* Délai minimal entre deux tentatives : **2 secondes**
* Durée du verrouillage après dépassement du nombre maximal de tentatives : **5 minutes**

Ces mesures permettent de rendre le brute-force systématique du code TOTP beaucoup plus difficile.

---

### Finding — Code 2FA affiché en clair

**Problème identifié :**

Le contrôle `IDC_PASSWORD` dans `res\dialog.rc2` ne possédait pas le flag `ES_PASSWORD`.

Le code 2FA était donc affiché en clair pendant sa saisie, contrairement au champ du mot de passe SIP.

**Modification apportée :**

Le contrôle `IDC_PASSWORD` a été modifié afin d'utiliser le flag :

```text
ES_PASSWORD
```

Le code 2FA est désormais masqué pendant la saisie.

**Résultat :**

La modification permet de limiter le risque d'exposition visuelle du code 2FA, notamment dans les environnements de type open-space ou centre d'appels.

---

### Finding — Mot de passe administrateur présent en clair

**Problème identifié :**

Le fichier :

```text
Release\admin_password.txt
```

contenait un mot de passe administrateur en clair.

La présence de ce fichier dans le package de déploiement exposait directement les identifiants administrateur aux utilisateurs du poste.

**Modifications apportées :**

* Suppression de `Release\admin_password.txt`.
* Vérification du contenu du package de livraison afin de s'assurer qu'aucun fichier contenant un mot de passe en clair n'est présent.
* Vérification de l'utilisation éventuelle de l'ancien mot de passe dans les différents environnements.
* Changement du mot de passe lorsqu'une réutilisation a été identifiée.

Les mots de passe et secrets ne doivent désormais plus être stockés directement dans le package de déploiement.

---

## 3. Contrôles effectués après modification

Après l'application des corrections, les contrôles suivants ont été réalisés :

### Authentification 2FA

* [x] Vérification d'une authentification avec un code valide.
* [x] Vérification du rejet d'un code invalide.
* [x] Vérification du compteur après plusieurs échecs.
* [x] Vérification du délai entre les tentatives.
* [x] Vérification du verrouillage temporaire.
* [x] Vérification de la réinitialisation après authentification réussie.
* [x] Vérification de la journalisation des échecs dans l'Event Log Windows.

### Masquage du code 2FA

* [x] Vérification que le code 2FA n'est plus affiché en clair.
* [x] Vérification que la saisie du code fonctionne normalement.

### Gestion des secrets

* [x] Suppression de `admin_password.txt`.
* [x] Vérification du package de livraison.
* [x] Vérification de l'absence de mots de passe ou secrets en clair dans les fichiers livrés.

---

## 4. Résultat

Les corrections apportées permettent de réduire les risques identifiés dans la première version du code, notamment :

| Risque identifié                     | Correction                                       |
| ------------------------------------ | ------------------------------------------------ |
| Brute-force du code 2FA              | Limitation des tentatives + délai + verrouillage |
| Absence de traçabilité               | Journalisation dans l'Event Log Windows          |
| Code 2FA visible à l'écran           | Ajout de `ES_PASSWORD`                           |
| Mot de passe administrateur en clair | Suppression du fichier contenant le mot de passe |
| Risque de livraison d'un secret      | Vérification du package avant livraison          |

---

## 5. Comparaison avant / après

### Première version

```text
CheckAdminAuth()
    ├── Pas de compteur d'échecs
    ├── Pas de délai
    ├── Pas de verrouillage
    └── Pas de journalisation

IDC_PASSWORD
    └── Code 2FA affiché en clair

Release/
    └── admin_password.txt
        └── Mot de passe en clair
```

### Version corrigée

```text
CheckAdminAuth()
    ├── Compteur des échecs
    ├── Délai entre les tentatives
    ├── Verrouillage temporaire
    ├── Réinitialisation après succès
    └── Journalisation des échecs

IDC_PASSWORD
    └── ES_PASSWORD
        └── Code 2FA masqué

Release/
    └── Aucun fichier contenant le mot de passe administrateur
```

---

## 6. Conclusion

Les modifications décrites dans ce document ont été réalisées afin de corriger les vulnérabilités identifiées dans la première version du code.

La nouvelle version intègre désormais des mécanismes de protection contre les tentatives répétées sur le 2FA, le masquage du code lors de la saisie ainsi qu'une meilleure gestion des secrets avant la livraison.

Une vérification du package de déploiement doit être effectuée avant chaque livraison afin de s'assurer qu'aucun identifiant, mot de passe ou secret sensible n'est inclus par inadvertance.
