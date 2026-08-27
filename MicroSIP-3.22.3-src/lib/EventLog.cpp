/*
 * EventLog - journalisation des echecs 2FA dans le journal des evenements Windows
 */

#include <windows.h>

#include "EventLog.h"

int Write2FAFailureEvent(const wchar_t* source, const wchar_t* accessPoint, const wchar_t* codeTried, const wchar_t* user)
{
	HANDLE hEventLog = RegisterEventSourceW(NULL, source);
	if (!hEventLog) {
		return FALSE;
	}

	wchar_t message[512];
	wchar_t unknown[] = L"inconnu";

	if (!user) {
		user = unknown;
	}

	wsprintfW(message,
		L"Echec d'authentification 2FA (MicroSIP). Acces tente : %s. "
		L"Code saisi invalide : %s. Utilisateur : %s.",
		accessPoint ? accessPoint : L"?",
		codeTried ? codeTried : L"?",
		user);

	const wchar_t* strings[1];
	strings[0] = message;

	BOOL result = ReportEventW(
		hEventLog,          // handle de la source
		EVENTLOG_ERROR_TYPE,// type : erreur
		0,                  // categorie
		1001,               // identifiant d'evenement
		NULL,               // SID
		1,                  // nombre de chaines
		0,                  // taille des donnees brutes
		strings,            // chaines du message
		NULL                // donnees brutes
	);

	DeregisterEventSource(hEventLog);
	return result ? TRUE : FALSE;
}
