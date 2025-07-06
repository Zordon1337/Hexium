#pragma once
#include "../hooks.hpp"

void __cdecl H::PrintNotifDetour(QString* message, int durationMs) {

	// call og func first otherwise in game we will see funny chinese text lmao
	H::pPrintNotifOG(message, durationMs);

	/*if (message && M::QString_toUtf8 && M::QByteArray_constData) {
		QByteArray* utf8 = M::QString_toUtf8(message);
		const char* text = M::QByteArray_constData(utf8);
		//if (text)
			//printf("[Notification] %s\n", text);
	}*/
}