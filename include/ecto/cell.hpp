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
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <boost/typeof/std/utility.hpp>

#include <ecto/tendril.hpp>
#include <ecto/tendrils.hpp>
#include <ecto/strand.hpp>
#include <ecto/util.hpp>
#include <ecto/profile.hpp>
#include <ecto/is_threadsafe.hpp>

#include <map>

namespace ecto
{

  /**
   * \brief Return values for modules' process functions. These
   * are appropriate for non exceptional behavior.
   */
  enum ReturnCode
  {
    OK = 0, //!< Everything A OK.
    QUIT = 1,
  //!< Explicit quit now.
  };

  template<int RVal>
  const std::string&
  ReturnCodeToStr()
  {
    static const std::string str = "Unknown Return Value";
    return str;
  }

  template<>
  const std::string&
  ReturnCodeToStr<ecto::OK>();
  template<>
  const std::string&
  ReturnCodeToStr<ecto::QUIT>();

  const std::string&
  ReturnCodeToStr(int rval);

  /**
   * \brief ecto::cell is the non virtual interface to the basic building
   * block of ecto graphs.  This interface should never be the parent of
   * client cell, but may be used for polymorphic access to client cells.
   *
   * Clients should expose their code to this interface through
   * ecto::wrap, or ecto::create_cell<T>().
   *
   * For a client's cell to satisfy the ecto::cell idium, it must
   * look similar to the following definition.
   * @code
   struct MyEctoCell
   {
     //called first thing, the user should declare their parameters in this
     //free standing function.
     static void declare_params(tendrils& params);
     //declare inputs and outputs here. The parameters may be used to
     //determine the io
     static void declare_io(const tendrils& params, tendrils& in, tendrils& out);
     //called right after allocation of the cell, exactly once.
     void configure(tendrils& params, tendrils& inputs, tendrils& outputs);
     //called at every execution of the graph
     int process(const tendrils& in, tendrils& out);
   };
   * @endcode
   *
   * It is important to note that all functions have are optional and they all have
   * default implementations.
   */
  struct ECTO_EXPORT cell: boost::noncopyable
  {
    typedef boost::shared_ptr<cell> ptr; //!< A convenience pointer typedef

    cell();
    virtual ~cell();

    /**
     * \brief Dispatches parameter declaration code. After this code, the parameters
     * for the cell will be set to their defaults.
     */
    void declare_params();
    /**
     * \brief Dispatches input/output declaration code.  It is assumed that the parameters
     * have been declared before this is called, so that inputs and outputs may be dependent
     * on those parameters.
     */
    void declare_io();

    /**
     * \brief Given initialized parameters,inputs, and outputs, this will dispatch the client
     * configuration code.  This will allocated an instace of the clients cell, so this
     * should not be called during introspection.
     */
    void configure();

    /**
     * \brief Dispatches the process function for the client cell.  This should only
     * be called from one thread at a time.
     *
     * Also, this function may throw exceptions...
     *
     * @return A return code, ecto::OK , or 0 means all is ok. Anything non zero should be considered an
     * exit signal.
     */
    ReturnCode process();

    /**
     * \brief Return the type of the child class.
     * @return A human readable non mangled name for the client class.
     */
    std::string type() const;

    /**
     * \brief Grab the name of the instance.
     * @return The name of the instance, or the address if none was given when object was constructed
     */
    std::string name() const;

    /**
     * \brief Set the name of the instance.
     */
    void name(const std::string&);

    /**
     * \brief Set the short_doc_ of the instance.
     */
    std::string short_doc() const;

    /**
     * \brief Set the short_doc_ of the instance.
     */
    void short_doc(const std::string&);

    /**
     * \brief Generate an Restructured Text doc string for the cell. Includes documentation for all parameters,
     * inputs, outputs.
     * @param doc The highest level documentation for the cell.
     * @return A nicely formatted doc string.
     */
    std::string gen_doc(const std::string& doc = "A module...") const;

    void verify_params() const;
    void verify_inputs() const;

    ptr clone() const;

    tendrils parameters; //!< Parameters
    tendrils inputs; //!< Inputs, inboxes, always have a valid value ( may be NULL )
    tendrils outputs; //!< Outputs, outboxes, always have a valid value ( may be NULL )
    boost::optional<strand> strand_;
    profile::stats_type stats;

  protected:

    virtual void init() = 0;
    virtual void dispatch_declare_params(tendrils& t) = 0;

    virtual void dispatch_declare_io(const tendrils& params, tendrils& inputs,
                                     tendrils& outputs) = 0;

    virtual void dispatch_configure(const tendrils& params, const tendrils& inputs,
                                    const tendrils& outputs) = 0;

    virtual ReturnCode dispatch_process(const tendrils& inputs, const tendrils& outputs) = 0;

    virtual std::string dispatch_name() const = 0;
    virtual ptr dispatch_make() const
    {
      return ptr();
    }

    virtual std::string dispatch_short_doc() const
    {
      return "";
    }

    virtual void dispatch_short_doc(const std::string&)
    {
    }
  private:
    std::string instance_name_;
  };

  
  /**
   * \brief Helper class for determining if client modules have function
   * implementations or not.
   * @internal
   */
  template<class T>
  struct has_f
  {
    typedef char yes;
    typedef char (&no)[2];
    
    // SFINAE eliminates this when the type of arg is invalid
    template<class U>
    static yes test_declare_params(BOOST_TYPEOF_TPL(&U::declare_params));
    // overload resolution prefers anything at all over "..."
    template<class U>
    static no test_declare_params(...);

    template<class U>
    static yes test_declare_io(BOOST_TYPEOF_TPL(&U::declare_io));
    template<class U>
    static no test_declare_io(...);

    template<class U>
    static yes test_configure(BOOST_TYPEOF_TPL(&U::configure));
    template<class U>
    static no test_configure(...);

    template<class U>
    static yes test_process(BOOST_TYPEOF_TPL(&U::process));
    template<class U>
    static no test_process(...);

    enum
    {
      declare_params = sizeof(test_declare_params<T> (0)) == sizeof(yes)
    };
    enum
    {
      declare_io = sizeof(test_declare_io<T> (0)) == sizeof(yes)
    };
    enum
    {
      configure = sizeof(test_configure<T> (0)) == sizeof(yes)
    };
    enum
    {
      process = sizeof(test_process<T> (0)) == sizeof(yes)
    };

  };

  /**
   * \brief cell_<T> is for registering an arbitrary class
   * with the the cell NVI. This adds a barrier between client code and the cell.
   */
  template<class Impl>
  struct cell_: cell
  {
    typedef boost::shared_ptr<cell_<Impl> > ptr;

    typedef typename detail::python_mutex<Impl>::type gil_mtx_t;

    ~cell_()
    {
    }
  protected:
    template<int I>
    struct int_
    {
    };
    typedef int_<0> not_implemented;
    typedef int_<1> implemented;

    static void declare_params(not_implemented, tendrils& params)
    {
    }

    static void declare_params(implemented, tendrils& params)
    {
      Impl::declare_params(params);
    }

    void dispatch_declare_params(tendrils& params)
    {
      //this is a none static function. for virtuality.
      declare_params(int_<has_f<Impl>::declare_params> (), params);
    }

    static void declare_io(not_implemented, const tendrils& params,
                           tendrils& inputs, tendrils& outputs)
    {
    }
    static void declare_io(implemented, const tendrils& params,
                           tendrils& inputs, tendrils& outputs)
    {
      Impl::declare_io(params, inputs, outputs);
    }

    void dispatch_declare_io(const tendrils& params, tendrils& inputs,
                             tendrils& outputs)
    {
      declare_io(int_<has_f<Impl>::declare_io> (), params, inputs, outputs);
    }

    void configure(not_implemented, const tendrils&, const tendrils& , const tendrils&)
    {
    }

    void configure(implemented, 
                   const tendrils& params, const tendrils& inputs, const tendrils& outputs)
    {
      boost::this_thread::interruption_point();
      impl->configure(params,inputs,outputs);
      for (tendrils::const_iterator iter = inputs.begin(); iter != inputs.end(); ++iter)
        if (iter->second->is_type<boost::python::object>())
          throw std::runtime_error("you can't use a python object as an input");

      for (tendrils::const_iterator iter = outputs.begin(); iter != outputs.end(); ++iter)
        if (iter->second->is_type<boost::python::object>())
          throw std::runtime_error("you can't use a python object as an output");


    }

    void dispatch_configure(const tendrils& params, const tendrils& inputs,
                            const tendrils& outputs)
    {
      //the cell may not be allocated here, so check pointer.
      if (!impl)
      {
        try
        {
          impl.reset(new Impl);
        }
        catch (std::exception& e)
        {
          except::EctoException ee("Original Exception: " +name_of(typeid(e)));
          ee << "  What   : " + std::string(e.what());
          ee << "  Module : " + name() + "\n  in constructor of: " + name_of<Impl>();
          boost::throw_exception(ee); \
        }
        catch (...)
        {
          except::EctoException ee("Threw unknown exception type!");
          ee << "  Module : " + name() + "\n  in constructor of: " + name_of<Impl>();
          boost::throw_exception(ee);
        }
        //configure is only called once.
        configure(int_<has_f<Impl>::configure> (), params,inputs,outputs);
      }
    }

    ReturnCode process(not_implemented, const tendrils&, const tendrils&)
    {
      return OK;
    }

    ReturnCode process(implemented, const tendrils& inputs, const tendrils& outputs)
    {
      ReturnCode code;
      profile::stats_collector coll(name(), stats);
      code = ReturnCode(impl->process(inputs, outputs));
      return code;
    }

    ReturnCode dispatch_process(const tendrils& inputs, const tendrils& outputs)
    {
      dispatch_configure(parameters,this->inputs,outputs);
      boost::this_thread::interruption_point();

      // this is a little hacky...  there are some problems with
      // throwing boost exceptions across shared library boundaries.
      try {
        return process(int_<has_f<Impl>::process> (), inputs, outputs);
      } catch (const boost::thread_interrupted& e) {
        return ecto::QUIT;
      }
    }

    std::string dispatch_name() const
    {
      return CELL_TYPE_NAME;
    }
    std::string dispatch_short_doc() const
    {
      return SHORT_DOC;
    }

    void dispatch_short_doc(const std::string&)
    {
    }

    cell::ptr dispatch_make() const
    {
      cell::ptr m(new cell_<Impl> ());
      m->declare_params();
      //copy all of the parameters by value.
      tendrils::iterator it = m->parameters.begin();
      tendrils::const_iterator end = m->parameters.end(), oit =
          parameters.begin();
      while (it != end)
        {
          it->second << *oit->second;
          ++oit;
          ++it;
        }
      m->declare_io();
      return m;
    }
    void init()
    {
      if(!impl)
      {
        cell::configure();
      }
    }

    static const std::string CELL_TYPE_NAME;
  public:
    boost::shared_ptr<Impl> impl;
    static std::string SHORT_DOC;
  };

  template<typename Impl>
  std::string cell_<Impl>::SHORT_DOC;

  template<typename Impl>
  const std::string cell_<Impl>::CELL_TYPE_NAME = ecto::name_of<Impl>();

  /**
   * Creates a cell from type T that has not been configured, so therefore,
   * not allocated.  This only calls the static functions associated with parameter and
   * input/output declaration.
   *
   * @return A cell::ptr that is initialized as far as default params,inputs,outputs go.
   */
  template<typename Impl>
  typename cell_<Impl>::ptr inspect_cell()
  {
    typename cell_<Impl>::ptr p(new cell_<Impl> ());
    cell::ptr base(p);
    base->declare_params();
    base->declare_io();
    return p;
  }

  /**
   * Create a cell from an type that has all of the proper interface functions defined.
   * This will call configure in the cell.
   * @return A cell ptr.
   */
  template<typename Impl>
  typename cell_<Impl>::ptr create_cell()
  {
    typename cell_<Impl>::ptr p = inspect_cell<Impl>();
    return p;
  }

}//namespace ecto
