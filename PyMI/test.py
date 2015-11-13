import mi

with mi.Application() as a:
    with a.create_session(protocol=mi.PROTOCOL_WMIDCOM) as s:
        c = s.get_class(u"root\\cimv2", u"win32_process")

        proc_name = u'notepad.exe'
        with s.exec_query(u"root\\cimv2", u"select * from win32_process where name = '%s'" % proc_name) as q:
            i = q.get_next_instance()
            while i is not None:
                print('cc')
                #print(dir(i))
                print(i.get_class_name())

                if i[u'name'].lower() == proc_name.lower():
                    owner = s.invoke_method(i, u"GetOwner")
                    print(owner.get_class_name())
                    for j in xrange(0, len(owner)):
                        print(owner[j])

                    params = a.create_instance(u"__parameters")
                    c = i.get_class()
                    params = a.create_method_params(c, u"Terminate")

                    params['reason'] = 2
                    print(len(params))
                    s.invoke_method(i, u"Terminate", params)

                print(i.get_class_name())
                print(len(i))
                print(i.name)
                print(i[u'name'])
                print(i['name'])
                for j in xrange(0, len(i)):
                    print(i[j])

                i = q.get_next_instance()
