import mi


class _Method(object):
    def __init__(self, conn, instance, method_name):
        self._conn = conn
        self._instance = instance
        self._method_name = method_name

    def __call__(self, *args, **kwargs):
        return self._conn.invoke_method(self._instance, self._method_name, *args, **kwargs)


class _Instance(object):
    def __init__(self, conn, instance):
        self._conn = conn
        self._instance = instance

    def __getattr__(self, name):
        try:
            return self._instance[unicode(name)]
        except mi.error:
            return _Method(self._conn, self, name)

    def associators(self, wmi_association_class=None, wmi_result_class=None):
        return self._conn.get_associators(self, wmi_association_class, wmi_result_class)

    def path_(self):
        return self._instance

    def GetText_(self, text_format):
        return self._conn.serialize_instance(self)


class _Class(object):
    def __init__(self, conn, className, cls):
        self._conn = conn
        self._className = unicode(className)
        self._cls = cls

    def __call__(self, *argc, **argv):
        fields = ""
        for i, v in enumerate(argc):
            if i > 0:
                raise ValueError('Invalid argument')
            if not isinstance(v, list):
                raise ValueError('Invalid argument')
            for f in v:
                if fields:
                    fields += ", "
                # TODO: sanitize input
                fields += f
        if not fields:
            fields = "*"

        where = ""
        for k, v in argv.items():
            if not where:
                where = " where "
            else:
                where += ", "
            where += k + " = '%s'" % v

        wql = (u"select %(fields)s from %(className)s%(where)s" %
               {"fields": fields,
                "className": self._className,
                "where": where});

        return self._conn.query(wql)

    def new(self):
        return self._conn.new_instance_from_class(self)



class _Connection(object):
    def __init__(self, computerName=".", ns="root/cimv2",
                 protocol=mi.PROTOCOL_WMIDCOM):
        self._ns = unicode(ns)
        self._app = mi.Application()
        self._session = self._app.create_session(
            computerName=unicode(computerName), protocol=unicode(protocol))

    def __del__(self):
        self._session = None
        self._app = None

    def __getattr__(self, name):
        with self._session.get_class(
            ns=self._ns, className=unicode(name)) as q:
            cls = q.get_next_class().clone()
            return _Class(self, name, cls)

    def _get_instances(self, op):
        l = []            
        i = op.get_next_instance()
        while i is not None:
            l.append(_Instance(self, i.clone()))
            i = op.get_next_instance()
        return l

    def query(self, wql):
        with self._session.exec_query(ns=self._ns, query=unicode(wql)) as q:
            return self._get_instances(q)

    def get_associators(self, instance, wmi_association_class=None, wmi_result_class=None):
        with self._session.get_associators(
                ns=self._ns, instance=instance._instance,
                assocClass=unicode(wmi_association_class),
                resultClass=unicode(wmi_result_class)) as q:
            return self._get_instances(q)

    def _wrap_instances(self, l):
        for i, o in enumerate(l):
            if isinstance(o, mi.Instance):
                l[i] = _Instance(self, o)
            if isinstance(o, list):
                self._wrap_instances(o)
            if isinstance(o, tuple):
                tmp = list(o)
                self._wrap_instances(tmp)
                l[i] = tuple(tmp)

    def invoke_method(self, instance, method_name, *args, **kwargs):
        cls = instance._instance.get_class()
        params = self._app.create_method_params(cls, unicode(method_name))

        for i, v in enumerate(args):
            params[i] = v
        for k, v in kwargs.items():
            print k
            params[k] = v

        with self._session.invoke_method(instance._instance, unicode(method_name), params) as op:
            l = []
            r = op.get_next_instance()
            for i in xrange(0, len(r)):
                l.append(r[i])
            self._wrap_instances(l)
            return tuple(l)

    def new_instance_from_class(self, cls):
        return _Instance(self, self._app.create_instance_from_class(cls._className, cls._cls))

    def serialize_instance(self, instance):
        with self._app.create_serializer() as s:
            return s.serialize_instance(instance._instance)


def WMI(moniker="root/cimv2", privileges=None):
    return _Connection(computerName=".", ns=moniker)
