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
available on Pypi [#pymipypi]_: ::

    pip install PyMI

Usage
-----

This project can be used either with a lower level interface that reflects the
underlying MI API structure or with the higher level (and slightly slower)
WMI module replacement.

MI module basic usage
^^^^^^^^^^^^^^^^^^^^^

Here's a simple example which enumerates all processes and kills any instance of
"KillerRabbitOfCaerbannog.exe". ::

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
which provides a simpler and higher level interface over the *mi* API: ::

    import wmi

    conn = wmi.WMI()
    for p in conn.Win32_Process():
        if p.Name == u"KillerRabbitOfCaerbannog.exe":
            p.Terminate(reason=10)


Build
-----

Open the provided *PyMI.sln* solution in Visual Studio 2015 [#VS2015]_, choose
your target Python version / platform and build. Wheel packages are
automatically generated in the *dist* folder for release builds.

Note: the target Python version must be present. The Python path can be
customized by setting the corresponding PythonDir* user macro,
e.g. *PythonDir_34_x64*.

References
----------

.. [#miapi] MI API https://msdn.microsoft.com/en-us/library/hh404805(v=vs.85).aspx
.. [#pywmi] Python WMI module https://pypi.python.org/pypi/WMI
.. [#pymipypi] PyMI on Pypi https://pypi.python.org/pypi/PyMI
.. [#vs2015] Visual Studio 2015 download https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx
