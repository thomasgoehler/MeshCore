# Fork-Workflow: Upstream-Updates integrieren

Diese Datei beschreibt, wie der Fork mit neuen offiziellen MeshCore-Releases synchronisiert wird.

## Eigene Änderungen (Fork-spezifisch)

Diese Dateien existieren nur im Fork und erzeugen nie Merge-Konflikte:

- `variants/diy_rp2040_zero/` — komplettes Verzeichnis (DIY RP2040 Zero Hardware-Variant)
- `src/helpers/SerialForward.cpp` / `.h` — SerialForward-Feature
- `build.py` — Hilfsskript zum Bauen und Flashen

Diese Datei kann gelegentlich Konflikte haben:

- `examples/companion_radio/MyMesh.cpp` — ForwardAllChannels-Änderung

## Workflow: Neue offizielle Version integrieren

### Vorbereitung (einmalig, falls noch nicht gemacht)

```bash
git remote add upstream https://github.com/meshcore-dev/MeshCore.git
```

### Schritt 1 — Stand prüfen

```bash
git fetch upstream
git log --oneline upstream/main..HEAD   # Eigene Commits (noch nicht im upstream)
git log --oneline HEAD..upstream/main   # Neue upstream-Commits (noch nicht integriert)
```

### Schritt 2 — Option A: Rebase (Normalfall, empfohlen)

```bash
git rebase upstream/main
```

Git setzt die eigenen Commits einzeln auf den neuen upstream-Stand auf.
Bei Konflikten:

```bash
# Konflikt in Editor lösen, dann:
git add <konflikt-datei>
git rebase --continue

# Abbrechen falls nötig:
git rebase --abort
```

### Schritt 2 — Option B: Frischer Branch mit Patches (Fallback)

Falls der Rebase zu viele Konflikte hat:

```bash
# Eigene Commits als Patches exportieren
git format-patch upstream/main -o /tmp/my-patches/

# Neuen Branch auf Basis des neuen upstream erstellen
git checkout -b main-new upstream/main

# Patches anwenden
git am /tmp/my-patches/*.patch

# Bei Konflikt:
# git am --abort
# git apply --reject /tmp/my-patches/000X-*.patch   → erzeugt .rej-Dateien als Hilfestellung
# Datei manuell anpassen, dann:
# git add <datei> && git am --continue
```

### Schritt 3 — Auf GitHub pushen

```bash
git push origin main --force-with-lease
```

`--force-with-lease` schlägt fehl wenn jemand anderes zwischenzeitlich gepusht hat — sicherer als `--force`.

## Schnellcheck: Wie weit ist der Fork entfernt?

```bash
git fetch upstream
git log --oneline upstream/main..HEAD    # Eigene Commits
git log --oneline HEAD..upstream/main    # Neue upstream-Commits
git diff --stat upstream/main...HEAD     # Geänderte Dateien zusammengefasst
```
