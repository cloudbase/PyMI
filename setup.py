from distutils import _msvccompiler
from distutils import ccompiler
import os
import setuptools
import sys

try:
    import multiprocessing  # noqa
except ImportError:
    pass


# MSVCP140.dll isn't included by Python along with the C runtime,
# for which reason we're going to statically link it for now.
# This can be disabled using the PYMI_VCRUNTIME_DYNAMIC_LINKING
# flag.
# Unfortunately distutils harcodes the "/MD" flag.
class Compiler(_msvccompiler.MSVCCompiler):
    def initialize(self, *args, **kwargs):
        super(Compiler, self).initialize(*args, **kwargs)

        dynamically_link_runtime = os.environ.get(
          "PYMI_VCRUNTIME_DYNAMIC_LINKING", "").lower() in (
          "yes", "y", "1", "true", "t")
        if not dynamically_link_runtime:
            self._statically_link_runtime()

    def _statically_link_runtime(self):
        if '/MD' in self.compile_options:
          self.compile_options.remove('/MD')
          self.compile_options.append('/MT')
        if '/MDd' in self.compile_options_debug:
          self.compile_options_debug.remove('/MDd')
          self.compile_options_debug.append('/MTd')


def new_compiler(plat=None, compiler=None, verbose=0, dry_run=0, force=0):
    return Compiler(None, dry_run, force)


if 'MSC' in sys.version:
    ccompiler.new_compiler = new_compiler


# Setuptools requires relative paths
mi_dir = 'MI'
pymi_dir = 'PyMI'

libmipp = (
    'mi++',
    {'sources': [os.path.join(mi_dir, src) for src in
                 ['MI++.cpp', 'MIExceptions.cpp', 'MIValue.cpp']],
     'macros': [('UNICODE', 1), ('_UNICODE', 1)]}
)
pymi_ext = setuptools.Extension(
    "mi",
    sources=[os.path.join(pymi_dir, src) for src in
             ['Application.cpp',
              'Callbacks.cpp',
              'Class.cpp',
              'DestinationOptions.cpp',
              'Instance.cpp',
              'MiError.cpp',
              'Operation.cpp',
              'OperationOptions.cpp',
              'PyMI.cpp',
              'Serializer.cpp',
              'Session.cpp',
              'stdafx.cpp',
              'Utils.cpp']],
    libraries=['mi++', 'mi', 'kernel32', 'user32', 'gdi32',
               'winspool', 'comdlg32', 'advapi32', 'shell32',
               'ole32', 'oleaut32', 'uuid', 'odbc32',
               'odbccp32'],
    include_dirs=[mi_dir],
    define_macros=[('UNICODE', 1), ('_UNICODE', 1)],
)

setuptools.setup(
    libraries=[libmipp],
    ext_modules=[pymi_ext],
    setup_requires=['pbr>=2.0.0'],
    pbr=True
)
