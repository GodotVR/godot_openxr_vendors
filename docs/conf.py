# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import sys, os, datetime

sys.path.append(os.path.abspath('_extensions'))

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Godot OpenXR Vendors plugin'
copyright = (
    "2022-present Godot XR contributors"
)
author = "Godot XR contributors"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_rtd_theme',
    'gdscript',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'python.env']

# Add GDScript syntax highlighting.
from gdscript import GDScriptLexer
from sphinx.highlighting import lexers
lexers['gdscript'] = GDScriptLexer()

smartquotes = False

# Pygments (syntax highlighting) style to use
pygments_style = "sphinx"
highlight_language = "gdscript"

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    #"logo_only": True,
    "collapse_navigation": False,
}
html_static_path = ['_static']
#html_logo = 'img/logo.png'
html_css_files = [
    "css/custom.css",
]
