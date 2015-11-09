import mi

a = mi.Application()
s = mi.Session(a)
q = mi.Query(s, u"root\\cimv2", u"select * from win32_process")
i = q.get_next_instance()

while i is not None:
    print(i.name)
    i = q.get_next_instance()

