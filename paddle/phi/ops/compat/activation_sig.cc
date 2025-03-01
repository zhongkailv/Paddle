/* Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/phi/core/compat/op_utils.h"

namespace phi {

#define DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(func_name, op_name, attrs) \
  KernelSignature func_name##GradOpArgumentMapping(               \
      const ArgumentMappingContext& ctx) {                        \
    return KernelSignature(                                       \
        op_name "_grad", {"X", "Out@GRAD"}, {attrs}, {"X@GRAD"}); \
  }

#define DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(func_name, op_name, attrs) \
  KernelSignature func_name##GradOpArgumentMapping(                 \
      const ArgumentMappingContext& ctx) {                          \
    return KernelSignature(                                         \
        op_name "_grad", {"Out", "Out@GRAD"}, {attrs}, {"X@GRAD"}); \
  }

#define DEFINE_ACT_GRAD_NODEP_OP_ARGMAP(func_name, op_name, attrs) \
  KernelSignature func_name##GradOpArgumentMapping(                \
      const ArgumentMappingContext& ctx) {                         \
    return KernelSignature(                                        \
        op_name "_grad", {"Out@GRAD"}, {attrs}, {"X@GRAD"});       \
  }

#define comma ,

DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Square, "square", );  // NOLINT

DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(HardTanh, "hard_tanh", "t_min" comma "t_max");
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(LeakyRelu, "leaky_relu", "alpha");
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(ThresholdedRelu,
                               "thresholded_relu",
                               "threshold");
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(SoftShrink, "soft_shrink", "lambda");
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Mish, "mish", "threshold");
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(TanhShrink, "tanh_shrink", );  // NOLINT
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Softsign, "softsign", );       // NOLINT
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Log, "log", );                 // NOLINT
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Celu, "celu", "alpha");        // NOLINT
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(HardSwish,
                               "hard_swish",
                               "threshold" comma "scale" comma
                               "offset");                // NOLINT
DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Swish, "swish", "beta");  // NOLINT

DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(STanh,
                               "stanh",
                               "scale_a" comma "scale_b");  // NOLINT

DEFINE_ACT_GRAD_DEPX_OP_ARGMAP(Softplus,
                               "softplus",
                               "beta" comma "threshold");  // NOLINT

DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(Relu, "relu", );               // NOLINT
DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(Sigmoid, "sigmoid", );         // NOLINT
DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(Sqrt, "sqrt", );               // NOLINT
DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(Rsqrt, "rsqrt", );             // NOLINT
DEFINE_ACT_GRAD_DEPOUT_OP_ARGMAP(Relu6, "relu6", "threshold");  // NOLINT

KernelSignature SqrtActiOpArgumentMapping(const ArgumentMappingContext& ctx) {
  if (ctx.IsDenseTensorInput("X")) {
    return KernelSignature("sqrt", {"X"}, {}, {"Out"});
  } else {
    return KernelSignature("sqrt_sr", {"X"}, {}, {"Out"});
  }
}

KernelSignature SquareActiOpArgumentMapping(const ArgumentMappingContext& ctx) {
  if (ctx.IsDenseTensorInput("X")) {
    return KernelSignature("square", {"X"}, {}, {"Out"});
  } else {
    return KernelSignature("square_sr", {"X"}, {}, {"Out"});
  }
}

KernelSignature ReluDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature("relu_double_grad", {"Out", "DDX"}, {}, {"DDOut"});
}

KernelSignature SigmoidDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "sigmoid_double_grad", {"Out", "DOut", "DDX"}, {}, {"DOutNew", "DDOut"});
}

KernelSignature SigmoidTripleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature("sigmoid_triple_grad",
                         {"Out", "DOut", "DDX", "D_DOut_New", "D_DDOut"},
                         {},
                         {"D_OutNew", "D_DOut", "D_DDx"});
}

KernelSignature LeakyReluDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "leaky_relu_double_grad", {"X", "DDX"}, {"alpha"}, {"DDOut"});
}

KernelSignature LeakyReluOpArgumentMapping(const ArgumentMappingContext& ctx) {
  return KernelSignature("leaky_relu", {"X"}, {"alpha"}, {"Out"});
}

KernelSignature EluOpArgumentMapping(const ArgumentMappingContext& ctx) {
  return KernelSignature("elu", {"X"}, {"alpha"}, {"Out"});
}

KernelSignature EluGradOpArgumentMapping(const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "elu_grad", {"X", "Out", "Out@GRAD"}, {"alpha"}, {"X@GRAD"});
}

KernelSignature EluDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "elu_double_grad", {"X", "DOut", "DDX"}, {"alpha"}, {"DX", "DDOut"});
}

KernelSignature LogDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "log_double_grad", {"X", "DOut", "DDX"}, {}, {"DX", "DDOut"});
}

KernelSignature SqrtDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "sqrt_double_grad", {"Out", "DX", "DDX"}, {}, {"DOut", "DDOut"});
}

KernelSignature RsqrtDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "rsqrt_double_grad", {"Out", "DX", "DDX"}, {}, {"DOut", "DDOut"});
}

KernelSignature CeluDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "celu_double_grad", {"X", "DOut", "DDX"}, {"alpha"}, {"DX", "DDOut"});
}

KernelSignature SquareDoubleGradOpArgumentMapping(
    const ArgumentMappingContext& ctx) {
  return KernelSignature(
      "square_double_grad", {"X", "DOut", "DDX"}, {}, {"DX", "DDOut"});
}

KernelSignature PowOpArgumentMapping(const ArgumentMappingContext& ctx) {
  if (ctx.HasInput("FactorTensor")) {
    return KernelSignature("pow", {"X"}, {"FactorTensor"}, {"Out"});
  } else {
    return KernelSignature("pow", {"X"}, {"factor"}, {"Out"});
  }
}

KernelSignature PowGradOpArgumentMapping(const ArgumentMappingContext& ctx) {
  if (ctx.HasInput("FactorTensor")) {
    return KernelSignature(
        "pow_grad", {"X", "Out@GRAD"}, {"FactorTensor"}, {"X@GRAD"});
  } else {
    return KernelSignature(
        "pow_grad", {"X", "Out@GRAD"}, {"factor"}, {"X@GRAD"});
  }
}

}  // namespace phi

PD_REGISTER_BASE_KERNEL_NAME(relu_grad_grad, relu_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(leaky_relu_grad_grad, leaky_relu_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(softshrink, soft_shrink);
PD_REGISTER_BASE_KERNEL_NAME(softshrink_grad, soft_shrink_grad);
PD_REGISTER_BASE_KERNEL_NAME(elu_grad_grad, elu_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(sigmoid_grad_grad, sigmoid_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(log_grad_grad, log_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(sqrt_grad_grad, sqrt_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(rsqrt_grad_grad, rsqrt_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(celu_grad_grad, celu_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(square_grad_grad, square_double_grad);
PD_REGISTER_BASE_KERNEL_NAME(brelu, hard_tanh);
PD_REGISTER_BASE_KERNEL_NAME(brelu_grad, hard_tanh_grad);

PD_REGISTER_ARG_MAPPING_FN(relu_grad, phi::ReluGradOpArgumentMapping);

PD_REGISTER_ARG_MAPPING_FN(square_grad, phi::SquareGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(sqrt_grad, phi::SqrtGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(sqrt_grad_grad,
                           phi::SqrtDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(rsqrt_grad, phi::RsqrtGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(rsqrt_grad_grad,
                           phi::RsqrtDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(mish_grad, phi::MishGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(stanh_grad, phi::STanhGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(softplus_grad, phi::SoftplusGradOpArgumentMapping);

PD_REGISTER_ARG_MAPPING_FN(relu_grad_grad,
                           phi::ReluDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(brelu_grad, phi::HardTanhGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(leaky_relu, phi::LeakyReluOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(leaky_relu_grad,
                           phi::LeakyReluGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(leaky_relu_grad_grad,
                           phi::LeakyReluDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(thresholded_relu_grad,
                           phi::ThresholdedReluGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(relu6_grad, phi::Relu6GradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(softshrink_grad,
                           phi::SoftShrinkGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(tanh_shrink_grad,
                           phi::TanhShrinkGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(elu, phi::EluOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(elu_grad, phi::EluGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(elu_grad_grad, phi::EluDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(softsign_grad, phi::SoftsignGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(sigmoid_grad, phi::SigmoidGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(sigmoid_grad_grad,
                           phi::SigmoidDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(sigmoid_triple_grad,
                           phi::SigmoidTripleGradOpArgumentMapping);

PD_REGISTER_ARG_MAPPING_FN(log_grad, phi::LogGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(log_grad_grad, phi::LogDoubleGradOpArgumentMapping);

PD_REGISTER_ARG_MAPPING_FN(sqrt, phi::SqrtActiOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(square, phi::SquareActiOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(hard_swish_grad,
                           phi::HardSwishGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(swish_grad, phi::SwishGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(pow_grad, phi::PowGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(pow, phi::PowOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(celu_grad, phi::CeluGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(celu_grad_grad,
                           phi::CeluDoubleGradOpArgumentMapping);
PD_REGISTER_ARG_MAPPING_FN(square_grad_grad,
                           phi::SquareDoubleGradOpArgumentMapping);
