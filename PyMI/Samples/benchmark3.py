import imp
import os

from distutils import sysconfig


path = os.path.dirname(os.path.abspath(__file__))
mi_wmi_path = os.path.join(os.path.dirname(path), "wmi.py")

path = sysconfig.get_python_lib()
old_wmi_path = os.path.join(path, "wmi.py")

VM_NAME = "wmi_benchmark_vm1"


def run_test():
    from hyperv.nova import constants
    from hyperv.nova import vmutilsv2

    u = vmutilsv2.VMUtilsV2()

    if u.vm_exists(VM_NAME):
        u.destroy_vm(VM_NAME)

    u.create_vm(VM_NAME, 256, 1, False, 1.0, 1, 'blah')

    u.get_vm_summary_info(VM_NAME)
    u.list_instances()
    u.get_vm_id(VM_NAME)
    u.set_vm_state(VM_NAME, constants.HYPERV_VM_STATE_ENABLED)
    u.set_vm_state(VM_NAME, constants.HYPERV_VM_STATE_DISABLED)
    u.destroy_vm(VM_NAME)


def test_mi():
    wmi = imp.load_source('wmi', mi_wmi_path)
    # Simple way to check we're loading the right module
    wmi._Connection
    run_test()


def test_wmi():
    wmi = imp.load_source('wmi', old_wmi_path)
    # Simple way to check we're loading the right module
    wmi.handle_com_error
    run_test()


if __name__ == '__main__':
    import timeit
    print("Running with old WMI module...")
    t_old = timeit.timeit(
        "test_wmi()", setup="from __main__ import test_wmi", number=50)
    print("Running with new WMI module...")
    t_new = timeit.timeit(
        "test_mi()", setup="from __main__ import test_mi", number=50)

    print("Old WMI module: %s seconds" % t_old)
    print("New WMI module: %s seconds" % t_new)

    print("Performance improvement: {percent:.2%}".format(
        percent=(1 - t_new / t_old)))
