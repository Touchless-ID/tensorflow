/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_COMPILER_XLA_CLIENT_LIB_APPROX_TOPK_H_
#define TENSORFLOW_COMPILER_XLA_CLIENT_LIB_APPROX_TOPK_H_

#include "tensorflow/compiler/xla/client/xla_builder.h"
#include "tensorflow/compiler/xla/statusor.h"
#include "tensorflow/compiler/xla/xla_data.pb.h"

namespace xla {

// EXPERIMENTAL
// This method is only implemented on TPU, and must have the flag
// `xla_tpu_nested_dot_fusion` set to true.
//
// Computes approximate top-ks by aggregating top-1s in equal-sized windows.
// The number and the size of the windows are determined by the `recall_target`.
//
// operand: A sequence of multi-dimensional arrays of type T_0, ..., T_{N-1}
// init_values: N starting values for top-1 reductions
// top_k: Determines the k in top-k operation.
// reduction_dim: Determines the dimension to compute top-k.
// comparator: The comparator computation to use, which should have function
//   signatore of (T_0, T_0, T_1, T_1, ..., T_{N-1}, T_{N-1}) -> bool.
// recall_target: Valid range (0, 1]. User can trade-off quality and performance
//   with this knob.
// aggregate_to_topk: When true, sorts the set of approximate top-k elements and
//   only keep the final k elements on TPU. This option is useful when user
//   wanted to forward the approximate results to host and aggregate the results
//   on CPU for better throughput.
//
// Returns a sequence of multidimensional arrays of type T_0, ..., T_{N-1},
// which contains the approximate top-ks from the input operands. When
// `aggregate_to_topk` is set to true, the output size is just top_k. When
// `aggregate_to_topk` is set to false, the output size varied by the target
// recall. For target recall = 0.9, the output size is roughly 10 * top_k. For
// target recall = 0.99, the output size is roughly 100 * top_k.
//
// TODO(fchern): Support other hardware platforms.
XlaOp ApproxTopK(XlaBuilder* builder, absl::Span<const XlaOp> operands,
                 absl::Span<const XlaOp> init_values, int64_t top_k,
                 int64_t reduction_dim, const XlaComputation& comparator,
                 float recall_target = 0.9, bool aggregate_to_topk = true);

// Determine the output size of the reduciton dimension. This is useful for jax
// abstract eval to determine the output size.
//
// input_size: Input size of the reduction dimension.
// rank: Rank of the input operand.
// top_k: Determines the k in top-k operation.
// recall_target: Valid range (0, 1]. User can trade-off quality and performance
//   with this knob.
// aggregate_to_topk: When true, sorts the set of approximate top-k elements and
//   only keep the final k elements on TPU. This option is useful when user
//   wanted to forward the approximate results to host and aggregate the results
//   on CPU for better throughput.
//
// Returns a pair of
//   1. Reduction output size
//   2. Reduction amount in log2 form.
//
// 2. is invalid and set to -1 when the approximate output is disabled, i.e.
//   top_k = 1 or aggregate_to_topk = true.
//
// TODO(fchern): Add a pybind11 interface for ApproxTopKReductionOutputSize
StatusOr<std::pair<int64_t, int64_t>> ApproxTopKReductionOutputSize(
    int64_t input_size, int64_t rank, int64_t top_k, float recall_target,
    bool aggregate_to_topk);

}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_CLIENT_LIB_APPROX_TOPK_H_
