// Software License Agreement (BSD-3-Clause)
//
// Copyright 2018 The University of North Carolina at Chapel Hill
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

//! @author Jeff Ichnowski

#pragma once
#ifndef NIGH_TEST_IMPL_SAMPLER_LP_HPP
#define NIGH_TEST_IMPL_SAMPLER_LP_HPP

#include <nigh/metric/lp.hpp>
#include <nigh/metric/space.hpp>
#include <nigh/metric/space_lp_array.hpp>
#include <nigh/metric/space_lp_eigen.hpp>
#include <nigh/metric/space_lp_scalar.hpp>
#include <nigh/metric/space_lp_vector.hpp>
#include "sampler.hpp"
#include "box_sampler.hpp"

namespace nigh_test {
    using namespace unc::robotics::nigh;

    template <typename State, int p>
    struct Sampler<State, metric::LP<p>>
        : BoxSampler<State, metric::Space<State, metric::LP<p>>::kDimensions>
    {
        using Metric = metric::LP<p>;
        using Space = metric::Space<State, Metric>;

        Sampler(const Space& space)
            : BoxSampler<State, Space::kDimensions>(space.dimensions(), -5, 5)
        {
        }
    };
}

#endif

