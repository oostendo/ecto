/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ecto/ecto.hpp>
#include <ecto/registry.hpp>

using ecto::tendrils;
namespace ecto_test
{
  struct QuitAfter
  {
    static void declare_params(tendrils& params)
    {
      params.declare<unsigned> ("N", "Quit after this many calls to process()");
    }

    static void declare_io(const tendrils& params, tendrils& in, tendrils& out)
    {
      in.declare<double> ("in", "An inbox");
    }

    QuitAfter() : N(0), current(0) { }


    void configure(const tendrils& parms, const tendrils& inputs, const tendrils& outputs)
    {
      N = parms.get<unsigned>("N");
    }

    int process(const tendrils& in, const tendrils& /*out*/)
    {
      if (current >= N) 
        assert(false && "This shouldn't have been called, we signaled an error already");
      ++current;
      if (current >= N)
        return ecto::QUIT;
      return ecto::OK;
    }

    bool quit_already;
    unsigned N;
    unsigned current;
  };

}

ECTO_CELL(ecto_test, ecto_test::QuitAfter, "QuitAfter", "Quits the graph after so many ticks");

