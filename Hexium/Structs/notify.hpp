namespace Notif {
	void Show(const char* message, int durationMs);
}
void Notif::Show(const char* message, int durationMs) {
    if (!message || !M::Utf8_toQString || !G::memoryInitialized)
        return;

    using PrintNotifFn = void(__cdecl*)(QString*, int);
    static auto PrintPtr = M::PatternScan("Hexis.exe", "55 8B EC A1 ? ? ? ? 85 C0 ...");
    static PrintNotifFn PrintNotif = reinterpret_cast<PrintNotifFn>(PrintPtr);

    if (!PrintNotif) {
        LOG_ERROR("PrintNotification function not found!");
        return;
    }

    QString qMessage;
    M::Utf8_toQString(&qMessage, message, -1);
    PrintNotif(&qMessage, durationMs);
}