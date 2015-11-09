import mi
import wmi

def test_mi():
    a = mi.Application()
    s = mi.Session(a, protocol=u"WMIDCOM")
    q = mi.Query(s, u"root\\virtualization\\v2", u"select * from msvm_computersystem")
    i = q.get_next_instance()

    while i is not None:
        s = i.ElementName
        i = q.get_next_instance()

def test_wmi():
    conn = wmi.WMI(moniker="root\\virtualization\\v2")
    for i in conn.msvm_computersystem():
        s = i.ElementName

if __name__ == '__main__':
    import timeit
    print(timeit.timeit("test_mi()", setup="from __main__ import test_mi", number=20))
    print(timeit.timeit("test_wmi()", setup="from __main__ import test_wmi", number=20))
