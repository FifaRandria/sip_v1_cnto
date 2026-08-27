/*
 * EventLog - journalisation des echecs 2FA dans le journal des evenements Windows
 *
 * Utilisation de l'API Windows RegisterEventSource / ReportEvent :
 * aucun fichier local, la trace est visible dans l'Observateur d'evenements
 * (Journaux Windows -> Application), source "MicroSIP".
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Ecrit un evenement de type "echec 2FA" dans le journal des evenements Windows.
 *
 *   source       : nom de la source de l'evenement (ex. "MicroSIP")
 *   accessPoint  : libelle de l'acces tente (ex. "Parametres")
 *   codeTried    : code 2FA saisi (invalide)
 *   user         : nom de l'utilisateur courant si connu, sinon NULL
 *
 * Retourne TRUE en cas de succes, FALSE en cas d'echec de journalisation.
 */
int Write2FAFailureEvent(const wchar_t* source, const wchar_t* accessPoint, const wchar_t* codeTried, const wchar_t* user);

#ifdef __cplusplus
}
#endif
