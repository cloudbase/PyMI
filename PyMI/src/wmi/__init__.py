﻿# Copyright 2015 Cloudbase Solutions Srl
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

import re
import six
import struct
import threading
import weakref

import mi


class x_wmi(Exception):
    def __init__(self, info="", com_error=None):
        self.info = info
        self.com_error = com_error

    def __str__(self):
        return "<x_wmi: %s %s>" % (
            self.info or "Unexpected COM Error",
            self.com_error or "(no underlying exception)"
        )


class x_wmi_timed_out(x_wmi):
    pass


class x_wmi_not_found(x_wmi):
    pass


class com_error(Exception):
    def __init__(self, hresult, strerror, excepinfo, argerror):
        self.hresult = hresult
        self.strerror = strerror
        self.excepinfo = excepinfo
        self.argerror = argerror


def unsigned_to_signed(unsigned):
    signed, = struct.unpack("l", struct.pack("L", unsigned))
    return signed


def mi_to_wmi_exception(func):
    def func_wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except mi.error as ex:
            d = ex.args[0]
            hresult = unsigned_to_signed(d.get("error_code", 0))
            err_msg = d.get("message") or ""
            com_ex = com_error(
                hresult, err_msg,
                (0, None, err_msg, None, None, hresult),
                None)

            if d.get('mi_result') == 6:
                raise x_wmi_not_found(err_msg, com_ex)
            elif(isinstance(ex, mi.timeouterror)):
                raise x_wmi_timed_out(err_msg, com_ex)
            else:
                raise x_wmi(err_msg, com_ex)
    return func_wrapper

_app = None


def _get_app():
    global _app
    if not _app:
        _app = mi.Application()
    return _app


class _Method(object):
    def __init__(self, conn, target, method_name):
        self._conn = conn
        self._target = target
        self._method_name = method_name

    @mi_to_wmi_exception
    def __call__(self, *args, **kwargs):
        return self._conn.invoke_method(
            self._target, self._method_name, *args, **kwargs)


class _Path(object):
    """
    Provides a SWbemObjectPath replacement.
    """
    def __init__(self, item):
        self._item = item

    def __str__(self):
        return self.Path

    @property
    def Authority(self):
        raise NotImplementedError()

    @property
    def Class(self):
        return self._item.get_class_name()

    @property
    def DisplayName(self):
        return self.Path()

    @property
    def IsClass(self):
        return isinstance(self._item, _Class)

    @property
    def IsSingleton(self):
        raise NotImplementedError()

    @property
    def Keys(self):
        raise NotImplementedError()

    @property
    def Locale(self):
        raise NotImplementedError()

    @property
    def Namespace(self):
        return self._item.get_namespace()

    @property
    def ParentNamespace(self):
        raise NotImplementedError()

    @property
    def Path(self):
        return self._item.get_path()

    @property
    def RelPath(self):
        path = self._item.get_path()
        return path[path.find(':') + 1:]

    @property
    def Security_(self):
        raise NotImplementedError()

    @property
    def Server(self):
        return self._item.get_server_name()


class _Instance(object):
    def __init__(self, conn, instance, use_conn_weak_ref=False):
        if use_conn_weak_ref:
            object.__setattr__(self, "_conn_ref", weakref.ref(conn))
        else:
            object.__setattr__(self, "_conn_ref", conn)
        object.__setattr__(self, "_instance", instance)

    @property
    def _conn(self):
        if isinstance(self._conn_ref, weakref.ref):
            return self._conn_ref()
        else:
            return self._conn_ref

    @mi_to_wmi_exception
    def __getattr__(self, name):
        try:
            # If the class is an association class, certain of its properties
            # are references which contain the paths to the associated objecs.
            # The WMI module translates automatically into WMI objects those
            # class properties that are references. To maintain the
            # compatibility with the WMI module, those class properties that
            # are references are translated into objects
            return _wrap_element(self._conn, *self._instance.get_element(name),
                                 convert_references=True)
        except mi.error:
            return _Method(self._conn, self, name)

    @mi_to_wmi_exception
    def __setattr__(self, name, value):
        _, el_type, _ = self._instance.get_element(name)
        self._instance[six.text_type(name)] = _unwrap_element(el_type, value)

    @mi_to_wmi_exception
    def associators(self, wmi_association_class=None, wmi_result_class=None):
        return self._conn.get_associators(
            self, wmi_association_class, wmi_result_class)

    @mi_to_wmi_exception
    def path(self):
        return _Path(self._instance)

    @mi_to_wmi_exception
    def path_(self):
        return self._instance.get_path()

    @mi_to_wmi_exception
    def GetText_(self, text_format):
        return self._conn.serialize_instance(self)

    @mi_to_wmi_exception
    def put(self):
        if not self._instance.get_path():
            self._conn.create_instance(self)
        else:
            self._conn.modify_instance(self)

    @mi_to_wmi_exception
    def Delete_(self):
        self._conn.delete_instance(self)

    @mi_to_wmi_exception
    def set(self, **kwargs):
        for k, v in kwargs.items():
            self.__setattr__(k, v)


class _Class(object):
    def __init__(self, conn, class_name, cls):
        self._conn = conn
        self.class_name = six.text_type(class_name)
        self._cls = cls

    @mi_to_wmi_exception
    def __call__(self, *argc, **argv):
        fields = ""
        for i, v in enumerate(argc):
            if i > 0:
                raise ValueError('Invalid argument')
            if not isinstance(v, list):
                raise ValueError('Invalid argument')
            # TODO: sanitize input
            fields = ", ".join(v)
        if not fields:
            fields = "*"

        # TODO: sanitize input
        filter = " and ".join(
            "%(k)s = '%(v)s'" % {'k': k, 'v': v} for k, v in argv.items())
        if filter:
            where = " where %s" % filter
        else:
            where = ""

        wql = (u"select %(fields)s from %(class_name)s%(where)s" %
               {"fields": fields,
                "class_name": self.class_name,
                "where": where})
        return self._conn.query(wql)

    @mi_to_wmi_exception
    def __getattr__(self, name):
        try:
            return _wrap_element(self._conn, *self._cls.get_element(name))
        except mi.error:
            return _Method(self._conn, self, name)

    @mi_to_wmi_exception
    def new(self):
        return self._conn.new_instance_from_class(self)

    @mi_to_wmi_exception
    def watch_for(self, raw_wql=None, notification_type="operation",
                  wmi_class=None, delay_secs=1, fields=[], **where_clause):
        return _EventWatcher(self._conn, six.text_type(raw_wql))

    @mi_to_wmi_exception
    def path(self):
        return _Path(self._cls)


class _EventWatcher(object):
    def __init__(self, conn, wql):
        self._conn_ref = weakref.ref(conn)
        self._events_queue = []
        self._error = None
        self._event = threading.Event()
        self._operation = conn.subscribe(
            wql, self._indication_result, self.close)

    def _process_events(self):
        if self._error:
            err = self._error
            self._error = None
            raise x_wmi(info=err[1])
        if self._events_queue:
            return self._events_queue.pop(0)

    def __call__(self, timeout_ms=-1):
        while True:
            try:
                event = self._process_events()
                if event:
                    return event

                timeout = None
                if timeout_ms >= 0:
                    timeout = timeout_ms / 1000.0
                if not self._event.wait(timeout):
                    raise x_wmi_timed_out()
                self._event.clear()
            finally:
                if (not self._operation or
                        not self._operation.has_more_results()):
                    self.close()
                    raise x_wmi("No more events")

    def _indication_result(self, instance, bookmark, machine_id, more_results,
                           result_code, error_string, error_details):
        if self._conn_ref:
            conn = self._conn_ref()
            if conn:
                if instance:
                    event = _Instance(conn,
                                      instance[u"TargetInstance"].clone(),
                                      use_conn_weak_ref=True)
                    try:
                        previous_inst = _Instance(
                            conn, instance[u'PreviousInstance'].clone(),
                            use_conn_weak_ref=True)
                        object.__setattr__(event, 'previous', previous_inst)
                    except (mi.error, AttributeError):
                        # The 'PreviousInstance' attribute may be missing, for
                        # example in case of a creation event or simply
                        # because this field was not requested.
                        pass

                    self._events_queue.append(event)
                if error_details:
                    self._error = (
                        result_code, error_string,
                        _Instance(
                            conn, error_details.clone(),
                            use_conn_weak_ref=True))
                self._event.set()

    def close(self):
        if self._operation:
            self._operation.cancel()
            self._operation.close()
        self._event.set()
        self._operation = None
        self._timeout_ms = None
        self._conn_ref = None
        self._events_queue = []

    def __del__(self):
        self.close()


class _Connection(object):
    def __init__(self, computer_name=".", ns="root/cimv2", locale_name=None,
                 protocol=mi.PROTOCOL_WMIDCOM, cache_classes=True):
        self._ns = six.text_type(ns)
        self._app = _get_app()
        self._protocol = six.text_type(protocol)
        self._computer_name = six.text_type(computer_name)
        if locale_name:
            destination_options = self._app.create_destination_options()
            destination_options.set_ui_locale(locale_name=six.text_type(locale_name))
        else:
            destination_options = None
        self._session = self._app.create_session(
            computer_name=self._computer_name,
            protocol=self._protocol,
            destination_options=destination_options)
        self._cache_classes = cache_classes
        self._class_cache = {}
        self._method_params_cache = {}
        self._notify_on_close = []

    def _close(self):
        for callback in self._notify_on_close:
            callback()
        self._notify_on_close = []
        self._session = None
        self._app = None

    @mi_to_wmi_exception
    def __del__(self):
        self._close()

    @mi_to_wmi_exception
    def __getattr__(self, name):
        return self.get_class(six.text_type(name))

    def _get_instances(self, op):
        l = []
        i = op.get_next_instance()
        while i is not None:
            l.append(_Instance(self, i.clone()))
            i = op.get_next_instance()
        return l

    @mi_to_wmi_exception
    def query(self, wql):
        wql = wql.replace("\\", "\\\\")
        with self._session.exec_query(
                ns=self._ns, query=six.text_type(wql)) as q:
            return self._get_instances(q)

    @mi_to_wmi_exception
    def get_associators(self, instance, wmi_association_class=None,
                        wmi_result_class=None):
        if wmi_association_class is None:
            wmi_association_class = u""
        if wmi_result_class is None:
            wmi_result_class = u""

        with self._session.get_associators(
                ns=self._ns, instance=instance._instance,
                assoc_class=six.text_type(wmi_association_class),
                result_class=six.text_type(wmi_result_class)) as q:
            return self._get_instances(q)

    def _get_method_params(self, mi_class, method_name):
        params = None
        class_name = None
        if self._cache_classes:
            class_name = mi_class.get_class_name()
            params = self._method_params_cache.get((class_name, method_name))

        if params is not None:
            params = params.clone()
        else:
            params = self._app.create_method_params(
                mi_class, six.text_type(method_name))
            if self._cache_classes:
                self._method_params_cache[(class_name, method_name)] = params
                params = params.clone()
        return params

    @mi_to_wmi_exception
    def invoke_method(self, target, method_name, *args, **kwargs):
        if isinstance(target, _Instance):
            mi_target = target._instance
            mi_class = self.get_class(mi_target.get_class_name())._cls
        else:
            mi_target = target._cls
            mi_class = mi_target

        params = self._get_method_params(mi_class, method_name)
        for i, v in enumerate(args):
            _, el_type, _ = params.get_element(i)
            params[i] = _unwrap_element(el_type, v)
        for k, v in kwargs.items():
            _, el_type, _ = params.get_element(k)
            params[k] = _unwrap_element(el_type, v)

        if not params:
            params = None

        with self._session.invoke_method(
                mi_target, six.text_type(method_name), params) as op:
            l = []
            r = op.get_next_instance()
            elements = []
            for i in six.moves.range(0, len(r)):
                elements.append(r.get_element(i))

            # Sort the output params by name before returning their values.
            # The WINRM and WMIDCOM protocols behave differently in how
            # returned elements are ordered. This hack aligns with the WMIDCOM
            # behaviour to retain compatibility with the wmi.py module.
            for element in sorted(elements, key=lambda element: element[0]):
                # Workaround to avoid including the return value if the method
                # returns void, as there's no direct way to determine it.
                # This won't work if the method is expected to return a
                # boolean value!!
                if element != ('ReturnValue', mi.MI_BOOLEAN, True):
                    l.append(_wrap_element(self, *element))
            return tuple(l)

    @mi_to_wmi_exception
    def new_instance_from_class(self, cls):
        return _Instance(
            self, self._app.create_instance_from_class(
                cls.class_name, cls._cls))

    @mi_to_wmi_exception
    def serialize_instance(self, instance):
        with self._app.create_serializer() as s:
            return s.serialize_instance(instance._instance)

    @mi_to_wmi_exception
    def get_class(self, class_name):
        cls = None
        if self._cache_classes:
            cls = self._class_cache.get(class_name)

        if cls is None:
            with self._session.get_class(
                    ns=self._ns, class_name=class_name) as op:
                cls = op.get_next_class()
                if cls is not None:
                    cls = cls.clone()
                    if self._cache_classes:
                        self._class_cache[class_name] = cls

        if cls is not None:
            return _Class(self, class_name, cls)

    @mi_to_wmi_exception
    def get_instance(self, class_name, key):
        c = self.get_class(class_name)
        key_instance = self.new_instance_from_class(c)
        for k, v in key.items():
            key_instance._instance[six.text_type(k)] = v
        with self._session.get_instance(
                self._ns, key_instance._instance) as op:
            instance = op.get_next_instance()
            if instance:
                return _Instance(self, instance.clone())

    @mi_to_wmi_exception
    def create_instance(self, instance):
        self._session.create_instance(self._ns, instance._instance)

    @mi_to_wmi_exception
    def modify_instance(self, instance):
        self._session.modify_instance(self._ns, instance._instance)

    @mi_to_wmi_exception
    def delete_instance(self, instance):
        # Deleting an instance using WMIDCOM fails with
        # "Provider is not capable of the attempted operation"
        if self._protocol != mi.PROTOCOL_WINRM:
            tmp_session = self._app.create_session(
                computer_name=self._computer_name,
                protocol=mi.PROTOCOL_WINRM)
        else:
            tmp_session = self._session
        tmp_session.delete_instance(self._ns, instance._instance)

    @mi_to_wmi_exception
    def subscribe(self, query, indication_result_callback, close_callback):
        op = self._session.subscribe(
            self._ns, six.text_type(query), indication_result_callback)
        self._notify_on_close.append(close_callback)
        return op

    @mi_to_wmi_exception
    def watch_for(self, raw_wql=None, notification_type="operation",
                  wmi_class=None, delay_secs=1, fields=[], **where_clause):
        return _EventWatcher(self, six.text_type(raw_wql))


def _wrap_element(conn, name, el_type, value, convert_references=False):
    if isinstance(value, mi.Instance):
        if el_type == mi.MI_INSTANCE:
            return _Instance(conn, value.clone())
        elif el_type == mi.MI_REFERENCE:
            if convert_references:
                # Reload the object to populate all properties
                return WMI(value.get_path())
            return value.get_path()
        else:
            raise Exception(
                "Unsupported instance element type: %s" % el_type)
    if isinstance(value, (tuple, list)):
        if el_type == mi.MI_REFERENCEA:
            return tuple([i.get_path() for i in value])
        elif el_type == mi.MI_INSTANCEA:
            return tuple([_Instance(conn, i.clone()) for i in value])
        else:
            return tuple(value)
    else:
        return value


def _unwrap_element(el_type, value):
    if value is not None:
        if el_type == mi.MI_REFERENCE:
            instance = WMI(value)
            if instance is None:
                raise x_wmi_not_found("Reference not found: %s" % value)
            return instance._instance
        elif el_type == mi.MI_INSTANCE:
            return value._instance
        elif el_type == mi.MI_BOOLEAN:
            if isinstance(value, (str, six.text_type)):
                return value.lower() in ['true', 'yes', '1']
            else:
                return value
        elif el_type & mi.MI_ARRAY:
            l = []
            for item in value:
                l.append(_unwrap_element(el_type ^ mi.MI_ARRAY, item))
            return tuple(l)
        else:
            return value


def _parse_moniker(moniker):
    PROTOCOL = "winmgmts:"
    computer_name = '.'
    namespace = None
    path = None
    class_name = None
    key = None
    m = re.match("(?:" + PROTOCOL + r")?//([^/]+)/([^:]*)(?::(.*))?", moniker)
    if m:
        computer_name, namespace, path = m.groups()
        if path:
            m = re.match("([^.]+).(.*)", path)
            if m:
                key = {}
                class_name, kvs = m.groups()
                for kv in kvs.split(","):
                    m = re.match("([^=]+)=\"(.*)\"", kv)
                    name, value = m.groups()
                    # TODO: improve unescaping
                    key[name] = value.replace("//", "\\")
            else:
                class_name = path
    else:
        namespace = moniker
    return (computer_name, namespace, class_name, key)


@mi_to_wmi_exception
def WMI(moniker="root/cimv2", privileges=None, locale_name=None):
    computer_name, ns, class_name, key = _parse_moniker(
        moniker.replace("\\", "/"))
    conn = _Connection(computer_name=computer_name, ns=ns,
                       locale_name=locale_name)
    if not class_name:
        # Perform a simple operation to ensure the connection works.
        # This is needed for compatibility with the WMI module.
        conn.__provider
        return conn
    else:
        return conn.get_instance(class_name, key)
