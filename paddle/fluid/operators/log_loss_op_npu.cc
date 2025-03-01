/* Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <cmath>

#include "paddle/fluid/framework/op_registry.h"
#include "paddle/fluid/platform/device/npu/npu_op_runner.h"

namespace paddle {
namespace operators {

using Tensor = phi::DenseTensor;

template <typename T>
void LogLossAdds(const platform::Place& place,
                 const aclrtStream& stream,
                 const phi::DenseTensor* x,
                 float scale,
                 phi::DenseTensor* y) {
  //  Calculate y = x + scale
  y->mutable_data<T>(x->dims(), place);
  const auto& runner = NpuOpRunner("Adds", {*x}, {*y}, {{"value", scale}});
  runner.Run(stream);
}

template <typename T>
void LogLossMuls(const platform::Place& place,
                 const aclrtStream& stream,
                 const phi::DenseTensor* x,
                 float scale,
                 phi::DenseTensor* y) {
  //  Calculate y = x + scale
  y->mutable_data<T>(x->dims(), place);
  const auto& runner = NpuOpRunner("Muls", {*x}, {*y}, {{"value", scale}});
  runner.Run(stream);
}

template <typename T>
void LogLossBCE(const platform::Place& place,
                const aclrtStream& stream,
                const phi::DenseTensor* x,
                const phi::DenseTensor* y,
                phi::DenseTensor* z) {
  z->mutable_data<T>(x->dims(), place);
  const auto& runner =
      NpuOpRunner("BinaryCrossEntropy",
                  {*x, *y},
                  {*z},
                  {{"reduction", static_cast<std::string>("none")}});
  runner.Run(stream);
}

template <typename T>
void LogLossBCEGrad(const platform::Place& place,
                    const aclrtStream& stream,
                    const phi::DenseTensor* x,
                    const phi::DenseTensor* y,
                    const phi::DenseTensor* dout,
                    phi::DenseTensor* dx) {
  dx->mutable_data<T>(x->dims(), place);
  const auto& runner =
      NpuOpRunner("BinaryCrossEntropyGrad",
                  {*x, *y, *dout},
                  {*dx},
                  {{"reduction", static_cast<std::string>("none")}});
  runner.Run(stream);
}

template <typename T, typename AttrType = T>
class LogLossNPUKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& ctx) const override {
    auto* y = ctx.Output<phi::DenseTensor>("Loss");
    auto* pred = ctx.Input<phi::DenseTensor>("Predicted");
    auto* label = ctx.Input<phi::DenseTensor>("Labels");
    auto epsilon = static_cast<T>(ctx.Attr<AttrType>("epsilon"));

    auto place = ctx.GetPlace();
    auto stream =
        ctx.template device_context<paddle::platform::NPUDeviceContext>()
            .stream();

    float factor = 1 / (1 + 2 * epsilon);
    float coef = std::log(factor);
    LogLossAdds<T>(place, stream, pred, epsilon, y);
    LogLossMuls<T>(place, stream, y, factor, y);
    LogLossBCE<T>(place, stream, y, label, y);
    LogLossAdds<T>(place, stream, y, coef, y);
  }
};

template <typename T, typename AttrType = T>
class LogLossGradNPUKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& ctx) const override {
    auto* pred = ctx.Input<phi::DenseTensor>("Predicted");
    auto* label = ctx.Input<phi::DenseTensor>("Labels");
    auto* dloss = ctx.Input<phi::DenseTensor>(framework::GradVarName("Loss"));
    auto* dpred =
        ctx.Output<phi::DenseTensor>(framework::GradVarName("Predicted"));
    auto epsilon = static_cast<T>(ctx.Attr<AttrType>("epsilon"));

    auto place = ctx.GetPlace();
    auto stream =
        ctx.template device_context<paddle::platform::NPUDeviceContext>()
            .stream();

    if (dpred) {
      LogLossBCEGrad<T>(place, stream, pred, label, dloss, dpred);
      LogLossMuls<T>(place, stream, dpred, 1 / (1 + 2 * epsilon), dpred);
    }
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;

REGISTER_OP_NPU_KERNEL(log_loss, ops::LogLossNPUKernel<float>);

REGISTER_OP_NPU_KERNEL(log_loss_grad, ops::LogLossGradNPUKernel<float>);
