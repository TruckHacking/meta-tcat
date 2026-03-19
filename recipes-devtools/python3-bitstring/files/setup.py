from setuptools import setup, find_packages

setup(
    name="bitstring",
    version="4.4.0",
    description="A Python module for the creation, manipulation and analysis of binary data.",
    long_description="bitstring is a pure Python module designed to help make the creation and analysis of binary data as simple and natural as possible.",
    author="Scott Griffiths",
    url="https://github.com/scott-griffiths/bitstring",
    license="MIT",
    packages=find_packages(exclude=["test*"]),
    install_requires=[
    ],
)
