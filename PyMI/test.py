import mi

a = mi.Application()
s = mi.Session(a, protocol=u"WMIDCOM")
q = s.exec_query(u"root\\cimv2", u"select * from win32_process where name = 'notepad.exe'")
i = q.get_next_instance()
while i is not None:
    #print(dir(i))
    print(i.get_class_name())

    if i.name.lower() == u'notepad.exe':
        params = a.new_instance(u"__parameters")
        params['reason'] = 2
        print(len(params))
        s.invoke_method(i, u"Terminate", params)

    owner = s.invoke_method(i, u"GetOwner")
    print(owner.get_class_name())
    for j in xrange(0, len(owner)):
        print(owner[j])

    #print(i.get_class_name())
    #print(len(i))
    #print(i.name)
    #print(i[u'name'])
    #print(i['name'])
    #for j in xrange(0, len(i)):
    #    print(i[j])

    i = q.get_next_instance()

q = None