# Architektur

## Zwei Wege zur Maske

1. **Codegenerierung (`protoc-gen-qtforms`)** – erzeugt statische `QWidget`-Klassen. Gut lesbar,
   anpassbar, kein Laufzeit-Overhead. Gedacht als Hauptprodukt.
2. **Reflection (`MessageForm`)** – baut die Maske zur Laufzeit aus dem `Descriptor`.
   Gut für Tools, Prototypen und Tests; gleiche Annotationen, gleiches Verhalten.

Beide lesen dieselben Annotationen aus `pbform/ui_options.proto`.
Der Test `roundtrip` stellt sicher, dass beide Wege dieselbe Message liefern.

## Lizenz-Trennung (Qt)

Das Plugin (`plugin/`) linkt nur gegen protobuf/libprotoc, nicht gegen Qt. Es erzeugt Text.
Qt kommt erst im generierten Code und in `runtime/` ins Spiel – dort gelten die Qt-Bedingungen
des Anwenders bzw. beim Ausliefern der Runtime deine eigenen (LGPL: dynamisch linken!).
Vor einem Release rechtlich prüfen lassen.

## Annotationen

`field_ui`: label, unit, min, max, group, read_only, widget, tooltip, order, db_index
`message_ui`: title, table_name, list_columns

Extension-Nummer 50001 ist für lokale Nutzung okay; für ein öffentliches Release
eine Nummer bei Google registrieren (Protobuf Global Extension Registry).

## Datenhaltung

`SqliteStorage` (Blob-Modus): Tabelle pro Message-Typ mit `id` + serialisiertem Protobuf.
Nächster Schritt: `db_index`-Felder zusätzlich als echte Spalten (Filtern/Sortieren in SQL),
danach relationaler Modus mit Migrationen.
