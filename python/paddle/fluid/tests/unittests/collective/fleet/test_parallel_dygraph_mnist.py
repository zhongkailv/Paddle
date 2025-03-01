# Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import unittest

import paddle.fluid as fluid
from test_dist_base import TestDistBase
from spawn_runner_base import TestDistSpawnRunner
from parallel_dygraph_mnist import TestMnist

flag_name = os.path.splitext(__file__)[0]


class TestParallelDygraphMnist(TestDistBase):
    def _setup_config(self):
        self._sync_mode = False
        self._nccl2_mode = True
        self._dygraph = True
        self._find_unused_parameters = True

    def test_mnist(self):
        if fluid.core.is_compiled_with_cuda():
            self.check_with_place(
                os.path.abspath("../../parallel_dygraph_mnist.py"),
                delta=1e-5,
                check_error_log=True,
                log_name=flag_name,
            )


# TODO(liuyuhui): Multi-Card Baidu Kunlun XPU training exist accuracy problems
# it is difficult to find out immediately where the problem is,
# and we will work with frameworkers' help to fix it.
class TestParallelDygraphMnistXPU(TestDistBase):
    def _setup_config(self):
        self._sync_mode = False
        self._bkcl_mode = True
        self._dygraph = True
        self._enforce_place = "XPU"

    def test_mnist_xpu(self):
        if fluid.core.is_compiled_with_xpu():
            self.check_with_place(
                os.path.abspath("../../parallel_dygraph_mnist.py"),
                delta=1e-4,
                check_error_log=True,
                log_name=flag_name,
            )


class TestParallelDygraphMnistSpawn(TestDistSpawnRunner):
    def test_mnist_with_spawn(self):
        if fluid.core.is_compiled_with_cuda() and sys.version_info >= (3, 4):
            self.check_dist_result_with_spawn(test_class=TestMnist, delta=1e-5)


class TestParallelDygraphMnistAccGrad(TestDistBase):
    def _setup_config(self):
        self._sync_mode = False
        self._nccl2_mode = True
        self._dygraph = True
        self._use_fleet_api = True
        self._accumulate_gradient = True
        self._find_unused_parameters = False

    def test_mnist(self):
        if fluid.core.is_compiled_with_cuda():
            self.check_with_place(
                os.path.abspath("../../parallel_dygraph_mnist.py"),
                delta=1e-5,
                check_error_log=True,
                log_name=flag_name,
            )


class TestFleetDygraphMnistXPU(TestDistBase):
    def _setup_config(self):
        self._sync_mode = False
        self._bkcl_mode = True
        self._dygraph = True
        self._enforce_place = "XPU"
        self._use_fleet_api = True

    def test_mnist(self):
        if fluid.core.is_compiled_with_xpu():
            self.check_with_place(
                os.path.abspath("../../parallel_dygraph_mnist.py"),
                delta=1e-4,
                check_error_log=True,
                log_name=flag_name,
            )


if __name__ == "__main__":
    unittest.main()
