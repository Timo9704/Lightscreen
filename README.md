In diesem Dokument befinden sich alle HTTP-Aufrufe zur Konfiguration des Lightscreens via HTTP-Aufruf über einen Webbrowser.
Für IP-ADRESSE ist die IP des Lightscreens im lokalen Netzwerk anzugeben. Diese kann entweder beim Start des Controllers über die serielle Schnittstelle angezeigt oder im Router (z.B. Fritzbox) nachgeschaut werden.
Die Werte in den eckigen Klammern [] stehen für einsetzbare Werte, also z.B. "top" oder "bottom" , 0 bis 255.
Die Einstellungen werden dabei meist je LED Streifen vorgenommen.

Einstellung der Helligkeit:
http://IP-ADRESSE/stripBrightness?strip=[top|bottom]&brightness=[0-255]

Einstellung der Farbe in HS(V)-Farbraum:
http://IP-ADRESSE/setColor?strip=[top|bottom]&h=[0-255]&s=[0-255]

Einstellung der Zeit:
http://IP-ADRESSE/setFadeTime?strip=[top|bottom]&upDown=[up|down]&hour=[0-23]&minute=[0-59]

Aktivierung NightMode:
http://IP-ADRESSE/nightMode?minutes=[1-120]

Manuelles einschalten des Lightscreens:
http://IP-ADRESSE/on

Manuelles ausschalten des Lightscreens:
http://IP-ADRESSE/off

Rückkehr zum Automatik-/zeitgesteuerten Modus:
http://IP-ADRESSE/activateAutoMode

Rückgabe der aktuellen Einstellungen:
http://IP-ADRESSE/getInformation

Anzeige der Konfigurations-Webseite:
http://IP-ADRESSE/configPage

Aktualisierung der Uhrzeit über NTP:
http://IP-ADRESSE/setClock
