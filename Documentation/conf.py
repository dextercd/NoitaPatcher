# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'NoitaPatcher'
copyright = '2023, Dexter Castor Döpping'
author = 'Dexter Castor Döpping'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinxcontrib.luadomain",
    "sphinx_lua",
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['_static']

html_theme_options = {
    "description": "Unofficial extension to Noita's modding API.",
    "github_button": True,
    "github_user": "dextercd",
    "github_repo": "NoitaPatcher",
    "page_width": "1000px",
    "sidebar_width": "300px",
    "show_powered_by": False,
    "extra_nav_links": {
        "GitHub Repository": "https://github.com/dextercd/NoitaPatcher",
    },
}
