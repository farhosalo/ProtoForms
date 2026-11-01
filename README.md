# pbform

Qt6-Eingabemasken aus Protobuf-Messages – per **protoc-Plugin** (generierter Code)
oder per **Runtime-Reflection** – plus Speicherung in SQLite.

> Status: Prototyp / Grundgerüst. Lizenz: **TODO** (vor dem ersten Release festlegen).

## Aufbau

| Pfad | Inhalt |
|---|---|
| `proto/pbform/ui_options.proto` | Annotationen (`field_ui`, `message_ui`) |
| `plugin/` | `protoc-gen-qtforms` – erzeugt `<name>.qtform.{h,cpp}`; **ohne Qt-Abhängigkeit** |
| `runtime/` | `MessageForm` (Reflection-Maske) und `SqliteStorage` (Blob-Modus) |
| `cmake/PbformGenerate.cmake` | `pbform_generate_forms()` für Anwender-Projekte |
| `examples/recipes/` | Demo-App mit generierter Maske + SQLite |
| `tests/` | QtTest-Roundtrip (Reflection-Maske, generierte Maske, DB) |
| `docs/` | Architektur und Roadmap |

## Build (macOS / Homebrew)

    brew install qt protobuf cmake ninja
    cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt);$(brew --prefix protobuf)"
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/examples/recipes/recipes_demo

Nur das Plugin (ohne Qt) bauen: `-DPBFORM_BUILD_RUNTIME=OFF`.

## Benutzung im eigenen Projekt

```cmake
add_library(my_forms STATIC)
pbform_generate_forms(TARGET my_forms
    PROTOS machine.proto
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} /pfad/zu/pbform/proto)
target_link_libraries(my_forms PUBLIC my_protos Qt6::Widgets)
```

```proto
import "pbform/ui_options.proto";
message Recipe {
  double temperature = 1 [(pbform.field_ui) = { label: "Temperatur", unit: "°C", min: 0, max: 300 }];
}
```

## Bekannte Einschränkungen

- Nicht unterstützt (Platzhalter in der Maske): `repeated`/`map`, `oneof`, `bytes`, verschachtelte Typen (`Outer.Inner`).
- 64-Bit-Ints laufen über `QDoubleSpinBox` (exakt bis ca. 9e15).
- Rekursive Message-Typen werden erkannt (direkt) bzw. in der Reflection-Maske per Tiefenlimit begrenzt.
- Feldnamen, die in C++ als Schlüsselwort kollidieren (protoc hängt `_` an), sind im Generator noch nicht behandelt.
- Die Plugin-API ist gegen die `std::string`-Signatur von `CodeGenerator::Generate` geschrieben; bei neueren Protobuf-Versionen ggf. anpassen.
