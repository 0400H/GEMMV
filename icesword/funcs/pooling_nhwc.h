/* Copyright (c) 2018 NoobsDNN Authors, Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef NBDNN_ICESWORD_FUNCS_POOLING_U8_H
#define NBDNN_ICESWORD_FUNCS_POOLING_U8_H

#include "icesword/funcs/base.h"
#include "icesword/funcs/impl/impl_base.h"
#include "icesword/funcs/impl/x86/pooling_nhwc.h"
namespace noobsdnn {
namespace icesword {

template<typename TargetType,
        DataType OpDtype,
        DataType inDtype,
        DataType outDtype,
        typename LayOutType_op,
        typename LayOutType_in,
        typename LayOutType_out
>
class Pooling_u8 : public BaseFunc<
        Tensor<TargetType, inDtype, LayOutType_in>,
        Tensor<TargetType, outDtype, LayOutType_out>,
        Tensor<TargetType, OpDtype, LayOutType_op>,
        ImplBase,
        PoolingParam
> {
public:
    using BaseFunc<
            Tensor<TargetType, inDtype, LayOutType_in>,
            Tensor<TargetType, outDtype, LayOutType_out>,
            Tensor<TargetType, OpDtype, LayOutType_op>,
            ImplBase,
            PoolingParam>::BaseFunc;

    Pooling_u8() = default;

    typedef Tensor<TargetType, inDtype, LayOutType_in> InDataTensor;
    typedef Tensor<TargetType, outDtype, LayOutType_out> OutDataTensor;
    typedef Tensor<TargetType, OpDtype, LayOutType_op> OpTensor;
    typedef PoolingParam<OpTensor> Param_t;
    typedef std::vector<InDataTensor *> Input_v;
    typedef std::vector<OutDataTensor *> Output_v;
    typedef std::vector<Shape> Shape_v;

    virtual SaberStatus compute_output_shape(const Input_v& input, \
        Output_v &output, Param_t& param) override {

        Shape output_shape = (input[0]->valid_shape());

        int in_height = input[0]->height();
        int in_width = input[0]->width();

        int window_h = param.window_h;
        int window_w = param.window_w;
        int pad_h = param.pad_h;
        int pad_w = param.pad_w;
        int stride_h = param.stride_h;
        int stride_w = param.stride_w;
        int out_height;
        int out_width;
        if (param.global_pooling) {
            out_height = 1;
            out_width = 1;
            param.stride_h = in_height;
            param.stride_w = in_width;
            window_h = in_height;
            window_w = in_width;
            param.window_h = in_height;
            param.window_w = in_width;
        } else {
            if (param.cmp_out_shape_floor_as_conv) {
                out_height = static_cast<int>((static_cast<float>(
                             in_height + 2 * pad_h - window_h) / stride_h)) + 1;

                out_width = static_cast<int>((static_cast<float>(
                             in_width + 2 * pad_w - window_w) / stride_w)) + 1;
            } else {
                out_height = static_cast<int>(ceilf(static_cast<float>(
                             in_height + 2 * pad_h - window_h) / stride_h)) + 1;

                out_width = static_cast<int>(ceilf(static_cast<float>(
                             in_width + 2 * pad_w - window_w) / stride_w)) + 1;
            }
        }

        if (param.pooling_padded()) {
            if ((out_height - 1) * stride_h >= in_height + pad_h) {
                -- out_height;
            }
            if ((out_width - 1) * stride_w >= in_width + pad_w) {
                -- out_width;
            }
        }

        int height_idx = input[0]->height_index();
        int width_idx = input[0]->width_index();

        output_shape[height_idx] = out_height;
        output_shape[width_idx] = out_width;

        return output[0]->set_shape(output_shape);

    }

    virtual SaberStatus init_impl(ImplEnum implenum) override {
        switch (implenum) {
            case ICESWORD_IMPL_NHWC:
                this->_impl.push_back(new SaberPooling_nhwc <TargetType, OpDtype, inDtype, outDtype,
                    LayOutType_op, LayOutType_in, LayOutType_out>);
                return SaberSuccess;
            default:
                return SaberUnImplError;
        }
    }
private:

    virtual void pick_best_static() override {
        if (true) // some condition?
            this->_best_impl = this->_impl[0];
    }

    virtual void pick_best_specify(ImplEnum implenum) override {
        this->_best_impl = this->_impl[0];
    }

};

}
}
#endif