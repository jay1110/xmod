# Building the documentation

The published documentation uses Sphinx with MyST Markdown. Its entry point is
`docs/index.md`; Read the Docs loads `docs/conf.py` through the repository's
`.readthedocs.yaml`. The site includes the guides in `docs/`, the changelog, and
searchable command/cvar pages generated from `doc/cmd/*.xml`, `doc/cvar/*.xml`
and current server cvar registrations.

## Build locally

Python 3.11 or newer is required; Read the Docs and CI use Python 3.13.
Run these commands from the repository root on Windows:

```powershell
python -m venv build/docs-venv
build/docs-venv/Scripts/python -m pip install -r docs/requirements.txt
build/docs-venv/Scripts/python -m sphinx -n -W --keep-going -b html docs docs/_build/html
```

On Linux or macOS:

```sh
python3 -m venv build/docs-venv
build/docs-venv/bin/python -m pip install -r docs/requirements.txt
build/docs-venv/bin/python -m sphinx -n -W --keep-going -b html docs docs/_build/html
```

Open `docs/_build/html/index.html` after a successful build. Build output, the
virtual environment and `docs/_generated/` are ignored by Git. Edit the source
Markdown/XML or registered cvars, not generated reference pages.

`-W` turns warnings into failures. The GitHub documentation workflow runs the
same strict build and uploads the complete HTML site as an artifact. It also
checks the server configuration templates against their source registrations.
The optional manual legacy job retains the existing DocBook HTML/PDF workflow.

## Read the Docs

Select this repository and a branch containing `docs/conf.py`,
`docs/requirements.txt` and `.readthedocs.yaml`. Use `.readthedocs.yaml` as the
configuration file and rebuild that branch. A build of an older commit cannot
see files added on a newer branch.

No server binaries, game data, API keys or private server configuration are
needed to build the documentation. Only public configuration templates and
source registrations are read.

Configuration follows the official
[Read the Docs v2 reference](https://docs.readthedocs.com/platform/stable/config-file/v2.html)
and [Sphinx Markdown support](https://www.sphinx-doc.org/en/master/usage/markdown.html).

## Deutsch

Die Website wird mit Sphinx und MyST erzeugt. Der Einstieg ist `docs/index.md`,
die Build-Konfiguration liegt in `docs/conf.py`. Die Befehls- und Cvar-Seiten
werden automatisch aus den vorhandenen XML-Dateien und den aktuellen
Server-Cvar-Registrierungen erzeugt.

Die obigen Befehle im Projektverzeichnis ausführen und danach
`docs/_build/html/index.html` öffnen. Generierte Dateien nicht bearbeiten oder
einchecken. Für Read the Docs muss der ausgewählte Branch die neue
Konfiguration enthalten; anschließend den Dokumentationsbuild neu starten.
