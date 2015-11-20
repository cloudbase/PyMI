import mi

a = mi.Application()
s = a.create_session(protocol=u"WMIDCOM")
q = s.exec_query(u"root/virtualization/v2", u"select * from Msvm_VirtualSystemManagementService")
svc = q.get_next_instance()

c = svc.get_class()
p = a.create_method_params(c, u"GetSummaryInformation")

q1 = s.exec_query(u"root/virtualization/v2", u"select * from Msvm_VirtualSystemSettingData where ElementName = 'nano1'")
vssd = q1.get_next_instance()

p[u'SettingData'] = (vssd,) 
p[u'requestedInformation'] = (4, 100, 103, 105)

r = s.invoke_method(svc, u"GetSummaryInformation", p)

print("Result: %s" % r[0])
summary_info = r[1][0]

print("vCPUs: %s" % summary_info["NumberOfProcessors"])
print("EnabledState: %s" % summary_info["EnabledState"])
print("Memory: %s" % summary_info["MemoryUsage"])
print("UpTime: %s" % summary_info["UpTime"])

q1 = None
q = None
s = None
a = None

