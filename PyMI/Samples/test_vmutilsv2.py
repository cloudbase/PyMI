import wmi
wmi._Connection

from hyperv.nova import vmutilsv2
vm_name = "test1"

u = vmutilsv2.VMUtilsV2()

if u.vm_exists(vm_name):
    u.destroy_vm(vm_name)

u.create_vm(vm_name, 256, 1, False, 1.0, 1, 'blah')
