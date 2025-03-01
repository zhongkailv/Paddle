#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

import unittest
import numpy as np

from op_test import OpTest
import paddle
import paddle.fluid.core as core
import random
import sys

sys.path.append("./rnn")
from rnn_numpy import GRU
from convert import get_params_for_net

random.seed(2)
np.set_printoptions(threshold=np.inf)
paddle.enable_static()


class TestGRUOp(OpTest):
    def get_weight_names(self):
        weight_names = []
        for i in range(self.num_layers):
            for j in range(0, 2 * self.direction_num):
                weight_names.append("{}.weight_{}".format(i, j))
        for i in range(self.num_layers):
            for j in range(0, 2 * self.direction_num):
                weight_names.append("{}.bias_{}".format(i, j))
        return weight_names

    def setUp(self):
        self.op_type = "rnn"
        self.dtype = "float32" if core.is_compiled_with_rocm() else "float64"
        self.sequence_length = (
            None
            if core.is_compiled_with_rocm()
            else np.array([12, 11, 10, 9, 8, 7, 6, 5], dtype=np.int32)
        )
        self.num_layers = 1
        self.is_bidirec = False
        self.is_test = False
        self.mode = "GRU"
        self.dropout = 0.0
        seq_length = 12
        batch_size = 8
        input_size = 4
        self.hidden_size = 2
        self.set_attrs()

        self.direction_num = 2 if self.is_bidirec else 1
        direction = "bidirectional" if self.is_bidirec else "forward"

        input = np.random.uniform(
            low=-0.1, high=0.1, size=(seq_length, batch_size, input_size)
        ).astype(self.dtype)

        if self.sequence_length is not None:
            input[3][1:][:] = 0
            input[4][2:][:] = 0
            input[2][3:][:] = 0
            input[1][4:][:] = 0

        rnn1 = GRU(
            input_size,
            self.hidden_size,
            num_layers=self.num_layers,
            time_major=True,
            direction=direction,
            dropout=self.dropout,
            dtype=self.dtype,
        )

        flat_w = get_params_for_net(rnn1)

        output, last_hidden = rnn1(input, sequence_length=self.sequence_length)

        if core.is_compiled_with_rocm():

            def rocm_rnn_get_place():
                places = [core.CUDAPlace(0)]
                return places

            self._get_places = rocm_rnn_get_place

        init_h = np.zeros(
            (self.num_layers * self.direction_num, batch_size, self.hidden_size)
        ).astype(self.dtype)

        state_out = np.ndarray((300)).astype("uint8")

        self.inputs = {
            'Input': input,
            'WeightList': flat_w,
            'PreState': [('init_h', init_h)],
            'SequenceLength': self.sequence_length,
        }
        if self.sequence_length is None:
            self.inputs = {
                'Input': input,
                'WeightList': flat_w,
                'PreState': [('init_h', init_h)],
            }
        self.attrs = {
            'dropout_prob': self.dropout,
            'is_bidirec': self.is_bidirec,
            'input_size': input_size,
            'hidden_size': self.hidden_size,
            'num_layers': self.num_layers,
            'is_test': self.is_test,
            'mode': self.mode,
        }
        self.outputs = {
            'Out': output,
            'State': [('last_hidden', last_hidden)],
            'Reserve': np.ndarray((400)).astype("uint8"),
            'DropoutState': state_out,
        }

    def set_attrs(self):
        pass

    def test_output(self):
        self.check_output(no_check_set=['Reserve', 'DropoutState'])

    def test_grad(self):
        if not self.is_test:
            var_name_list = self.get_weight_names()
            grad_check_list = ['Input', 'init_h']
            grad_check_list.extend(var_name_list)
            self.check_grad(set(grad_check_list), ['Out', 'last_hidden'])


class TestGRUOp1(TestGRUOp):
    def set_attrs(self):
        self.sequence_length = None


class TestGRUOp2(TestGRUOp):
    def set_attrs(self):
        self.sequence_length = None
        self.is_bidirec = True


class TestGRUOp3(TestGRUOp):
    def set_attrs(self):
        self.sequence_length = None
        self.is_test = True


class TestGRUOp4(TestGRUOp):
    def set_attrs(self):
        self.sequence_length = None
        self.is_bidirec = True
        self.is_test = True


class TestGRUOpAvx(TestGRUOp):
    def set_attrs(self):
        self.dtype = "float32"
        self.hidden_size = 8


if __name__ == '__main__':
    unittest.main()
