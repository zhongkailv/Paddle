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

#include "paddle/fluid/framework/op_registry.h"
#include "paddle/fluid/platform/device/npu/npu_op_runner.h"
#include "paddle/fluid/platform/float16.h"

namespace paddle {
namespace operators {

using Tensor = phi::DenseTensor;

template <typename DeviceContext, typename T>
class MeanNPUKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& ctx) const override {
    auto* x = ctx.Input<phi::DenseTensor>("X");
    auto* out = ctx.Output<phi::DenseTensor>("Out");

    std::vector<int> axes;

    framework::NPUAttributeMap attr_input = {{"keep_dims", false},
                                             {"axes", axes}};

    out->mutable_data<T>(ctx.GetPlace());

    const auto& runner = NpuOpRunner("ReduceMeanD", {*x}, {*out}, attr_input);

    auto stream =
        ctx.template device_context<paddle::platform::NPUDeviceContext>()
            .stream();
    runner.Run(stream);
  }
};

template <typename DeviceContext, typename T>
class MeanGradNPUKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& context) const override {
    auto stream =
        context.template device_context<paddle::platform::NPUDeviceContext>()
            .stream();

    auto grad = context.Input<phi::DenseTensor>(framework::GradVarName("Out"));

    PADDLE_ENFORCE_EQ(grad->numel(),
                      1,
                      platform::errors::InvalidArgument(
                          "Mean Gradient Input Tensor len should be 1. But "
                          "received Out@Grad's elements num is %d.",
                          grad->numel()));

    auto IG = context.Output<phi::DenseTensor>(framework::GradVarName("X"));
    IG->mutable_data<T>(context.GetPlace());

    // ones
    Tensor ones(grad->dtype());
    ones.mutable_data<T>(IG->dims(), context.GetPlace());
    const auto& runner_ones = NpuOpRunner("OnesLike", {*IG}, {ones}, {});
    runner_ones.Run(stream);

    // means
    Tensor mean_tensor(grad->dtype());
    mean_tensor.Resize({1});
    mean_tensor.mutable_data<T>(context.GetPlace());
    FillNpuTensorWithConstant<T>(
        &mean_tensor, static_cast<T>(1.0 / static_cast<float>(IG->numel())));

    // means mul ones
    Tensor mean_ma(grad->dtype());
    mean_ma.Resize(IG->dims());
    mean_ma.mutable_data<T>(context.GetPlace());
    const auto& runner_mul_1 =
        NpuOpRunner("Mul", {mean_tensor, ones}, {mean_ma}, {});
    runner_mul_1.Run(stream);

    // and mul grad
    const auto& runner_mul_2 = NpuOpRunner("Mul", {mean_ma, *grad}, {*IG}, {});
    runner_mul_2.Run(stream);
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
namespace plat = paddle::platform;
REGISTER_OP_NPU_KERNEL(
    mean,
    ops::MeanNPUKernel<paddle::platform::NPUDeviceContext, float>,
    ops::MeanNPUKernel<paddle::platform::NPUDeviceContext, plat::float16>)

REGISTER_OP_NPU_KERNEL(
    mean_grad,
    ops::MeanGradNPUKernel<paddle::platform::NPUDeviceContext, float>,
    ops::MeanGradNPUKernel<paddle::platform::NPUDeviceContext, plat::float16>)
