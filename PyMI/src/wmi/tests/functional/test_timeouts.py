# Copyright 2016 Cloudbase Solutions Srl
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

import wmi
from wmi.tests.functional import test_base


# The following tests perform various operations,
# setting timeouts that are expected to be hit.
class OperationTimeoutTestCase(test_base.BaseFunctionalTestCase):
    def _check_op_timeout(self, f, *args, **kwargs):
        # 'f' is expected to be a method that accepts
        # the 'operation_timeout' argument.
        self.assertRaises(wmi.x_wmi_timed_out,
                          f, *args,
                          operation_timeout=0.001,
                          **kwargs)

    def test_query(self):
        self._check_op_timeout(self._conn_cimv2.Win32_Process)

    def test_invoke_method(self):
        disk = self._conn_storage.Msft_Disk()[0]
        self._check_op_timeout(disk.Refresh)

    def test_associators(self):
        disk = self._conn_cimv2.Win32_DiskDrive()[0]
        self._check_op_timeout(disk.associators)

    @test_base.BaseFunctionalTestCase.pass_temp_file
    def test_modify_delete(self, temp_file_path):
        datafile = self._conn_cimv2.Cim_DataFile(Name=temp_file_path)[0]
        datafile.Description = 'fake_description'
        datafile.put(operation_timeout=1)
        datafile.Delete_(operation_timeout=1)

    def test_per_session_timeouts(self):
        conn = wmi.WMI(operation_timeout=0.1)
        self.assertRaises(wmi.x_wmi_timed_out,
                          conn.Win32_Processor)

        # We expect the query to be successful after we override the
        # session based operation timeout.
        conn.Win32_Process(operation_timeout=10)
