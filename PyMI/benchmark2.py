def test_mi():
    import mi

    a = mi.Application()
    s = a.create_session(protocol=u"WMIDCOM")
    q = s.exec_query(u"root/virtualization/v2", u"select * from Msvm_VirtualSystemManagementService")
    svc = q.get_next_instance()

    c = svc.get_class()
    p = a.create_method_params(c, u"GetSummaryInformation")

    q1 = s.exec_query(u"root/virtualization/v2", u"select * from Msvm_ComputerSystem where ElementName = 'nano1'")
    vm = q1.get_next_instance()

    q2 = s.get_associators(u"root/virtualization/v2", vm, assocClass=u"Msvm_SettingsDefineState", resultClass=u"Msvm_VirtualSystemSettingData")
    vssd = q2.get_next_instance()

    p[u'SettingData'] = (vssd,) 
    p[u'requestedInformation'] = (4, 100, 103, 105)

    r = s.invoke_method(svc, u"GetSummaryInformation", p)

    print("Result: %s" % r[0])
    summary_info = r[1][0]

    print("vCPUs: %s" % summary_info["NumberOfProcessors"])
    print("EnabledState: %s" % summary_info["EnabledState"])
    print("Memory: %s" % summary_info["MemoryUsage"])
    print("UpTime: %s" % summary_info["UpTime"])

    q2 = None
    q1 = None
    q = None
    s = None
    a = None


def test_wmi():
    import wmi

    conn = wmi.WMI(moniker="root\\virtualization\\v2")
    svc = conn.Msvm_VirtualSystemManagementService()[0]
    vm = conn.query("select * from Msvm_ComputerSystem where ElementName = 'nano1'")[0]

    vssd = vm.associators(
            wmi_association_class="Msvm_SettingsDefineState",
            wmi_result_class="Msvm_VirtualSystemSettingData")[0]

    (ret_val, summary_info) = svc.GetSummaryInformation(
            [4, 100, 103, 105],
            [vssd.path_()])

    print("Result: %s" % ret_val)
    summary_info = summary_info[0]

    print("vCPUs: %s" % summary_info.NumberOfProcessors)
    print("EnabledState: %s" % summary_info.EnabledState)
    print("Memory: %s" % summary_info.MemoryUsage)
    print("UpTime: %s" % summary_info.UpTime)

if __name__ == '__main__':
    import timeit
    t1 = timeit.timeit("test_mi()", setup="from __main__ import test_mi", number=100)
    t2 = timeit.timeit("test_wmi()", setup="from __main__ import test_wmi", number=100)

    print "MI: %s" % t1
    print "WMI %s" % t2
