import wmi
wmi._Connection

from hyperv.nova import vmutilsv2
vmutilsv2.VMUtilsV2().create_vm('test1', 256, 1, False, 1.0, 1, 'blah')
