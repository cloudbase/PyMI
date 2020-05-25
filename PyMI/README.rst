PyMI - Windows Management Infrastructure API for Python
=======================================================

This project provides a Python native module wrapper over the Windows
Management Infrastructure (MI) API [#miapi]_.

Works with Python 2.7 and 3.x on any Windows version which supports the MI API,
both x86 and x64.

It includes also a drop-in replacement for the Python WMI [#pywmi]_ module,
proving much faster execution times and no dependency on pywin32.

Installation
------------

Pip is the preferred way to install PyMI. Pre-compiled binary wheels are
available on Pypi [#pymipypi]_:

.. code-block:: powershell

    pip install PyMI

Usage
-----

This project can be used either with a lower level interface that reflects the
underlying MI API structure or with the higher level (and slightly slower)
WMI module replacement.

MI module basic usage
^^^^^^^^^^^^^^^^^^^^^

Here's a simple example which enumerates all processes and kills any instance of
"KillerRabbitOfCaerbannog.exe".

.. code-block:: python

    import mi

    with mi.Application() as a:
        with a.create_session(protocol=mi.PROTOCOL_WMIDCOM) as s:
            proc_name = u'notepad.exe'
            with s.exec_query(
                    u"root\\cimv2", u"select * from Win32_Process") as q:
                i = q.get_next_instance()
                while i is not None:
                    if i[u'name'].lower() == u"KillerRabbitOfCaerbannog.exe":
                        cls = i.get_class()
                        # Prepare parameters
                        params = a.create_method_params(cls, u"Terminate")
                        # Exit code
                        params['reason'] = 10
                        # Invoke method
                        with s.invoke_method(i, u"Terminate", params) as op:
                            op.get_next_instance()
                    i = q.get_next_instance()

WMI module basic usage
^^^^^^^^^^^^^^^^^^^^^^

And here's the same example written using the *WMI* module replacement,
which provides a simpler and higher level interface over the *mi* API:

.. code-block:: python

    import wmi

    conn = wmi.WMI()
    for p in conn.Win32_Process():
        if p.Name == u"KillerRabbitOfCaerbannog.exe":
            p.Terminate(reason=10)


Build
-----

Use the following to build Python 3 wheels. Those will be copied to the build
dir.

.. code-block:: powershell

    python setup.py bdist_wheel

The best way to build PyMI for Python 2.7 or 3.4 and below is to use the
Visual Studio solution (described below). This will statically link the
vc140 runtime, which is required by PyMI.

Custom VS env vars
^^^^^^^^^^^^^^^^^^

distutils will automatically locate your Visual Studio and Windows SDK
installation. If you'd like to call vcvarsall.bat yourself and use a specific
version, use the following:

.. code-block:: powershell

    function SetVCVars($vcvarsdir, $platform="amd64")
    {
        pushd $vcvarsdir
        try
        {
            cmd /c "vcvarsall.bat $platform & set" |
            foreach {
              if ($_ -match "=") {
                $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
              }
            }
        }
        finally
        {
            popd
        }
    }

    # Replace this folder with the one in which the vcvarsall.bat script is
    # located (the exact location depends on the Visual Studio version).
    # SetVCVars "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build"
    SetVCVars "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"

    $env:DISTUTILS_USE_SDK=1
    $env:MSSdk=1

    python setup.py bdist_wheel

Make sure to use the Visual Studio toolset that matches the Python version
that you're targetting: https://wiki.python.org/moin/WindowsCompilers.

By default, we're statically linking the VC runtime. To enable dynamic
linking, set ``$env:PYMI_VCRUNTIME_DYNAMIC_LINKING="y"``.

Debug builds
^^^^^^^^^^^^

The easiest way to do a debug build is to set the following in setup.cfg:

.. code-block::

    [build]
    debug = 1

This will be honored regardless of the build type (e.g. stdist, wheel, etc).

To enable distutils debug logging, you may set the following:

.. code-block:: powershell

    $env:DISTUTILS_DEBUG = 1

Before doing a debug build, you may wish to clean the build dir.

Using the Visual Studio Solution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Open the provided *PyMI.sln* solution in Visual Studio 2015 [#VS2015]_, choose
your target Python version / platform and build. Wheel packages are
automatically generated in the *dist* folder for release builds.

Note: the target Python version must be present. The Python path can be
customized by setting the corresponding PythonDir* user macro,
e.g. *PythonDir_34_x64*. The *wheel* and *GitPython* packages are required during the build process:

.. code-block:: powershell

    pip install wheel
    pip install GitPython

As an alternative, you can use the MSBuild CLI tool:

.. code-block:: powershell

    $env:MSBuildEmitSolution="TRUE"
    MSBuild.exe .\PyMI.sln /p:Configuration="Release (Python 3.7)"


References
----------

.. [#miapi] MI API https://msdn.microsoft.com/en-us/library/hh404805(v=vs.85).aspx
.. [#pywmi] Python WMI module https://pypi.python.org/pypi/WMI
.. [#pymipypi] PyMI on Pypi https://pypi.python.org/pypi/PyMI
.. [#vs2015] Visual Studio 2015 download https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
