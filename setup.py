import os
import setuptools

try:
    import multiprocessing  # noqa
except ImportError:
    pass

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
