//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-716457
//
// All rights reserved.
//
// This file is part of Ascent.
//
// For details, see: http://ascent.readthedocs.io/.
//
// Please also read ascent/LICENSE
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


//-----------------------------------------------------------------------------
///
/// file: ascent_runtime_trigger_filters.cpp
///
//-----------------------------------------------------------------------------

#include "ascent_runtime_query_filters.hpp"

//-----------------------------------------------------------------------------
// thirdparty includes
//-----------------------------------------------------------------------------

// conduit includes
#include <conduit.hpp>
#include <conduit_blueprint.hpp>

//-----------------------------------------------------------------------------
// ascent includes
//-----------------------------------------------------------------------------
#include <ascent_expression_eval.hpp>
#include <ascent_logging.hpp>
#include <ascent_data_object.hpp>
#include <ascent_runtime_param_check.hpp>

#include <flow_graph.hpp>
#include <flow_workspace.hpp>

using namespace conduit;
using namespace std;

using namespace flow;

//-----------------------------------------------------------------------------
// -- begin ascent:: --
//-----------------------------------------------------------------------------
namespace ascent
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime --
//-----------------------------------------------------------------------------
namespace runtime
{

//-----------------------------------------------------------------------------
// -- begin ascent::runtime::filters --
//-----------------------------------------------------------------------------
namespace filters
{


//-----------------------------------------------------------------------------
BasicQuery::BasicQuery()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
BasicQuery::~BasicQuery()
{
// empty
}

//-----------------------------------------------------------------------------
void
BasicQuery::declare_interface(Node &i)
{
    i["type_name"]   = "basic_query";
    i["port_names"].append() = "in";
    // this is a dummy port that we use to enforce
    // a order of execution
    i["port_names"].append() = "dummy";
    // adding an output port to chain queries together
    // so they execute in order of declaration
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
BasicQuery::verify_params(const conduit::Node &params,
                            conduit::Node &info)
{
    info.reset();
    bool res = check_string("expression",params, info, true);
    res &= check_string("name",params, info, true);

    std::vector<std::string> valid_paths;
    valid_paths.push_back("expression");
    valid_paths.push_back("name");

    return res;
}


//-----------------------------------------------------------------------------
void
BasicQuery::execute()
{
    if(!input(0).check_type<DataObject>())
    {
        ASCENT_ERROR("Query input must be a data object");
    }

    DataObject *data_object = input<DataObject>(0);
    if(!data_object->is_valid())
    {
      set_output<DataObject>(data_object);
      return;
    }

    std::string expression = params()["expression"].as_string();
    std::string name = params()["name"].as_string();
    conduit::Node actions;

    Node v_info;

    // The mere act of a query stores the results
    runtime::expressions::ExpressionEval eval(*data_object);
    conduit::Node res = eval.evaluate(expression, name);

    // we never actually use the output port
    // since we only use it to chain ordering
    conduit::Node *dummy =  new conduit::Node();
    set_output<conduit::Node>(dummy);
}

//-----------------------------------------------------------------------------
FilterQuery::FilterQuery()
:Filter()
{
// empty
}

//-----------------------------------------------------------------------------
FilterQuery::~FilterQuery()
{
// empty
}

//-----------------------------------------------------------------------------
void
FilterQuery::declare_interface(Node &i)
{
    i["type_name"]   = "expression";
    i["port_names"].append() = "in";
    i["output_port"] = "true";
}

//-----------------------------------------------------------------------------
bool
FilterQuery::verify_params(const conduit::Node &params,
                            conduit::Node &info)
{
    info.reset();
    bool res = check_string("expression",params, info, true);
    res &= check_string("name",params, info, true);

    std::vector<std::string> valid_paths;
    valid_paths.push_back("expression");
    valid_paths.push_back("name");

    return res;
}


//-----------------------------------------------------------------------------
void
FilterQuery::execute()
{
    if(!input(0).check_type<DataObject>())
    {
        ASCENT_ERROR("Query input must be a data object");
    }

    DataObject *data_object = input<DataObject>(0);

    std::string expression = params()["expression"].as_string();
    std::string name = params()["name"].as_string();
    conduit::Node actions;

    Node v_info;

    // The mere act of a query stores the results
    //runtime::expressions::ExpressionEval eval(n_input.get());
    runtime::expressions::ExpressionEval eval(*data_object);
    conduit::Node res = eval.evaluate(expression, name);

    // if the end result is a derived field the for sure we want to make
    // its available.
    bool derived = false;
    if(res.has_path("type"))
    {
      if(res["type"].as_string() == "field")
      {
        derived = true;
      }
    }

    // Since queries might add new fields, the blueprint needs to become the source
    if(derived && (data_object->source() != DataObject::Source::LOW_BP))
    {
      // for now always copy the bp if its not the original data source
      // There is one main reasons for this:
      //   the data will likely be passed to the vtkh ghost stripper, which could create
      //   a new data sets with memory owned by vtkm. Since conduit can't take ownership of
      //   that memory, this data could could go out of scope and that would be bad. To ensure
      //   that it does not go out of scope
      //   TODO: We could be smarter than this. For example, we could provide a way to map a
      //   new field, if created, back on to the original source (e.g., vtkm)
      conduit::Node *new_source = new conduit::Node(*eval.data_object().as_low_order_bp());
      DataObject *new_do = new DataObject(new_source);
      set_output<DataObject>(new_do);
    }
    else
    {
      set_output<DataObject>(data_object);
    }

}


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime::filters --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent::runtime --
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end ascent:: --
//-----------------------------------------------------------------------------





