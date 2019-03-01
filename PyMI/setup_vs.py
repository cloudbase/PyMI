# This module is used by the Visual Studio solution.

import git
import os
from setuptools import setup, find_packages
from setuptools.command import build_clib


def get_git_version():
    g = git.Git(".")
    return g.describe(tags=True)


def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


setup(
    name="PyMI",
    version=get_git_version(),
    author="Alessandro Pilotti",
    author_email="info@cloudbasesolutions.com",
    description=("Windows Management Infrastructure API for Python."),
    license="Apache 2.0",
    keywords="wmi mi windows",
    url="https://github.com/cloudbase/PyMI",
    packages=find_packages("src") + find_packages(".."),
    package_dir={'mi': 'src/mi', 'wmi': '../wmi'},
    # Note: this extension is compiled in Visual Studio
    package_data={'mi': ['mi.pyd', 'mi.pdb']},
    long_description=read('README.rst'),
    install_requires=['setuptools'],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Environment :: Win32 (MS Windows)',
        'Intended Audience :: Developers',
        'Intended Audience :: System Administrators',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Operating System :: Microsoft :: Windows',
        'Topic :: System :: Systems Administration'
    ],
)
