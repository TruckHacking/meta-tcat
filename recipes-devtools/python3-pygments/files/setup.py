from setuptools import setup, find_packages

setup(
    name="Pygments",
    version="2.19.2",
    description="Pygments is a syntax highlighting package written in Python.",
    author="Georg Brandl",
    author_email="georg@python.org",
    packages=find_packages(),
    entry_points={
        "console_scripts": ["pygmentize = pygments.cmdline:main"]
    },
)
