cc_library(
  jit_executor_engine
  SRCS executor_engine.cc
  DEPS executor)

cc_library(
  jit_pe_engine
  SRCS pe_engine.cc
  DEPS parallel_executor)

cc_library(
  jit_interpreter_engine
  SRCS interpreter_engine.cc
  DEPS standalone_executor)

cc_library(
  jit_predictor_engine
  SRCS predictor_engine.cc
  DEPS paddle_inference_api analysis_predictor)
