# Roadmap

## M1 – Grundgerüst (erledigt, ungetestet auf Zielplattform)
- [x] Annotationen, Plugin, Reflection-Form, SQLite-Blob, Beispiel, Tests, CI-Entwurf

## M2 – Typabdeckung
- [ ] `repeated` (Tabellen-Widget mit Add/Remove) – Plugin und Reflection
- [ ] `oneof` (Auswahl + QStackedWidget)
- [ ] `map`, `bytes`, verschachtelte Typen (`Outer.Inner`), 64-Bit-Ints exakt
- [ ] Validierung (Min/Max beim Laden, Pflichtfelder)

## M3 – Datenbank
- [ ] `db_index` als echte Spalten, Filter/Sortierung
- [ ] Listenansicht mit `list_columns`
- [ ] Postgres-Backend hinter `IStorage`

## M4 – Produkt
- [ ] QML-Ausgabe (zweites Backend im Generator)
- [ ] Installierbares Paket (`install(EXPORT)`, find_package(pbform))
- [ ] Doku-Website, Demo-Video, Lizenzmodell (Open-Core?)
- [ ] 5–10 Gespräche mit Zielgruppe (Maschinenbau/Embedded)
