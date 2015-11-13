import mi

with mi.Application() as a:
    with a.create_session(protocol=mi.PROTOCOL_WMIDCOM) as s:
        with s.get_class(u"root\\cimv2", u"win32_process") as q2:
            c = q2.get_next_class()

        proc_name = u'notepad.exe'
        with s.exec_query(u"root\\cimv2", u"select * from win32_process where name = '%s'" % proc_name) as q:
            i = q.get_next_instance()
            while i is not None:
                print('cc')
                #print(dir(i))
                print(i.get_class_name())

                if i[u'name'].lower() == proc_name.lower():
                    with s.invoke_method(i, u"GetOwner") as q1:
                        owner = q1.get_next_instance()
                        print(owner.get_class_name())
                        for j in xrange(0, len(owner)):
                            print(owner[j])

                    params = a.create_instance(u"__parameters")
                    c = i.get_class()
                    params = a.create_method_params(c, u"Terminate")

                    params['reason'] = 2
                    print(len(params))
                    with s.invoke_method(i, u"Terminate", params) as q3:
                        q3.get_next_instance()

                print(i.get_class_name())
                print(len(i))
                print(i.name)
                print(i[u'name'])
                print(i['name'])
                for j in xrange(0, len(i)):
                    print(i[j])

                i = q.get_next_instance()
