"""Sphinx configuration for the Xmod operator and developer documentation."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent / "_ext"))

info = (ROOT / "project/info.db").read_text(encoding="utf-8")
project = "Xmod"
author = "ETc|#.Jay.# and Xmod contributors"
copyright = "2005-2011 Jaybird, 2025-2026 Xmod contributors"
release = ".".join(re.search(rf"^version{part}\s*=\s*(\d+)", info, re.M)[1]
                   for part in ("Major", "Minor", "Point"))
version = release

extensions = ["myst_parser", "xmod_reference"]
source_suffix = {".md": "markdown", ".rst": "restructuredtext"}
root_doc = "index"
language = "en"
exclude_patterns = ["_build", "requirements.txt"]
myst_enable_extensions = ["colon_fence"]
myst_heading_anchors = 6
nitpicky = True

html_theme = "sphinx_rtd_theme"
html_title = f"Xmod {release} documentation"
html_theme_options = {"navigation_depth": 3, "collapse_navigation": True}
html_static_path = []
html_show_sourcelink = True
