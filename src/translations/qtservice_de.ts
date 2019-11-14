<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>AndroidServiceControl</name>
    <message>
        <location filename="../plugins/servicebackends/android/androidservicecontrol.cpp" line="+63"/>
        <location line="+6"/>
        <source>The bind command must be called with a QAndroidServiceConnection* as first parameter and QtAndroid::BindFlags as optional second parameter</source>
        <translation>Das bind Kommando muss mit QAndroidServiceConnection* als ersten und QtAndroid::BindFlags als optionalen zweiten Parameter aufgerufen werden</translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+6"/>
        <source>The unbind command must be called with a QAndroidServiceConnection* as only parameter</source>
        <translation>Das unbind Kommando muss mit QAndroidServiceConnection* als einzigen Parameter aufgerufen werden</translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+6"/>
        <source>The startWithIntent command must be called with a QAndroidIntent as only parameter</source>
        <translation>Das startWithIntent Kommando muss mit QAndroidIntent als einzigen Parameter aufgerufen werden</translation>
    </message>
</context>
<context>
    <name>LaunchdServiceControl</name>
    <message>
        <location filename="../plugins/servicebackends/launchd/launchdservicecontrol.cpp" line="+88"/>
        <source>Failed to find launchctl executable</source>
        <translation>Konnte launchctl Anwendung nicht finden</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>launchctl crashed with error: %1</source>
        <translation>launchctl ist abgestürtzt mit Fehlermeldung: %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>launchctl did not exit in time</source>
        <translation>launchctl ist nicht rechtzeitig fertig geworden</translation>
    </message>
</context>
<context>
    <name>QtService::ServiceControl</name>
    <message>
        <location filename="../service/servicecontrol.cpp" line="+99"/>
        <source>Operation custom command for kind %1 is not implemented for backend %2</source>
        <translation>Die custom command Operation des Typs %1 ist für Backend %2 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Reading the service status is not implemented for backend %1</source>
        <translation>Das Lesen des Dienst-Status ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Reading the autostart state is not implemented for backend %1</source>
        <translation>Das Lesen des Autostart-Status ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Operation start is not implemented for backend %1</source>
        <translation>Die Start-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation stop is not implemented for backend %1</source>
        <translation>Die Stop-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Operation restart is not supported for non-blocking service controls without status information</source>
        <translation>Die Stop-Operation ist für Backends die weder blockend noch keine Status-Informationen bereitstellen nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation pause is not implemented for backend %1</source>
        <translation>Die Pause-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation resume is not implemented for backend %1</source>
        <translation>Die Resume-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation reload is not implemented for backend %1</source>
        <translation>Die Reload-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation enable autostart is not implemented for backend %1</source>
        <translation>Die Autostart aktivieren-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Operation disable autostart is not implemented for backend %1</source>
        <translation>Die Autostart deaktivieren-Operation ist für Backend %1 nicht implementiert</translation>
    </message>
</context>
<context>
    <name>StandardServiceControl</name>
    <message>
        <location filename="../plugins/servicebackends/standard/standardservicecontrol.cpp" line="+48"/>
        <source>Failed to access lockfile with error: %1</source>
        <translation>Konnte nicht auf Sperrdatei zugreifen mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Unabled to find executable for service with id &quot;%1&quot;</source>
        <translation>Konnte Anwendung für Service mit id &quot;%1&quot; nicht finden</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Failed to start service process with error: %1</source>
        <translation>Konnte Dienst-Prozess nicht starten mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Failed to get pid of running service</source>
        <translation>Konnten Prozess-ID des laufenden Prozesses nicht ermitteln</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Service did not stop yet</source>
        <translation>Service wurde noch nicht beendet</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Failed to send stop signal with error: %1</source>
        <translation>Konnte Stop-Signal nicht zum Dienst senden mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Failed to disable local console handler with error: %1</source>
        <translation>Konnte lokalen Konsolen-Handler nicht deaktivieren mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Failed to attach to service console with error: %1</source>
        <translation>Konnte nicht an Konsole des Dienst-Prozesses anhängen mit Fehler: %1</translation>
    </message>
</context>
<context>
    <name>SystemdServiceBackend</name>
    <message>
        <location filename="../plugins/servicebackends/systemd/systemdservicebackend.cpp" line="+43"/>
        <source>Run the service as system service, independend of the current user id</source>
        <translation>Führt den Dienst als Systemdienst aus, unabhängig von der aktuellen Nutzer-ID</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Run the service as user service, independend of the current user id</source>
        <translation>Führt den Dienst als Benutzerdienst aus, unabhängig von der aktuellen Nutzer-ID</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A command to execute on the primary service process. Can be either &apos;reload&apos; to let the service reload its configuration or &apos;stop&apos; to halt the service</source>
        <translation>Ein Kommando, welches der primäre Dienst-Prozess ausführen soll. Kann entweder &apos;reload&apos; sein um den Dienst seine Konfiguration neu laden zu lassen, oder &apos;stop&apos; um den Dienst anzuhalten</translation>
    </message>
</context>
<context>
    <name>SystemdServiceControl</name>
    <message>
        <location filename="../plugins/servicebackends/systemd/systemdservicecontrol.cpp" line="+117"/>
        <source>Unknown service state %1 for service %2</source>
        <translation>Unbekannter Dienst-Status %1 des Dienstes %2</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Service %1 was not found as systemd service</source>
        <translation>Dienst %1 konnte nicht als Systemd-Dienst gefunden werden</translation>
    </message>
    <message>
        <location line="+99"/>
        <source>Failed to find systemctl executable</source>
        <translation>Konnte systemctl Anwendung nicht finden</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>systemctl crashed with error: %1</source>
        <translation>systemctl ist abgestürtzt mit Fehlermeldung: %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>systemctl did not exit in time</source>
        <translation>systemctl ist nicht rechtzeitig fertig geworden</translation>
    </message>
</context>
<context>
    <name>WindowsServiceControl</name>
    <message>
        <location filename="../plugins/servicebackends/windows/windowsservicecontrol.cpp" line="+11"/>
        <source>Failed to get acces to service manager with error: %1</source>
        <translation>Konnte nicht auf Dienst-Manager zugreifen mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+32"/>
        <location line="+38"/>
        <location line="+25"/>
        <source>Failed to query service status with error: %1</source>
        <translation>Konnte Dienst-Status nicht lesen mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <location line="+6"/>
        <source>The command must be called with a single integer [128,255] as argument</source>
        <translation>Das command Kommando muss mit einem Integer [128,255] als einzigen Parameter aufgerufen werden</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Failed to send command %1 to service with error: %2</source>
        <translation>Konnte Kommando %1 nicht zum Dienst senden mit Fehler: %2</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Failed to start service with error: %1</source>
        <translation>Konnte Dienst-Prozess nicht starten mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Failed to stop service with error: %1</source>
        <translation>Konnte Dienst-Prozess nicht stoppen mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Failed to pause service with error: %1</source>
        <translation>Konnte Dienst-Prozess nicht pausieren mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Failed to resume service with error: %1</source>
        <translation>Konnte Dienst-Prozess nicht fortsetzen mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+26"/>
        <location line="+22"/>
        <source>Failed to enable autostart with error: %1</source>
        <translation>Konnte Autostart des Dienstes nicht de/aktivieren mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Failed to enable/disable service with error: %1</source>
        <translation>Konnte den Dienst nicht aktivieren/deaktivieren mit Fehler: %1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Failed to get access to service with error: %1</source>
        <translation>Konnte nicht auf Dienst zugreifen mit Fehler: %1</translation>
    </message>
</context>
</TS>
