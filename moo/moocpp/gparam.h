/*
 *   moocpp/gparam.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef __cplusplus
#error "This is a C++-only header"
#endif

#include <mooglib/moo-glib.h>
#include <moocpp/strutils.h>
#include <memory>
#include <utility>
#include <vector>

namespace moo {

// Bit-compatible with GValue
class Value : public GValue
{
public:
    Value();
    ~Value();

    Value(Value&&);
    Value& operator=(Value&&);

    Value(const Value&) = delete;
    Value& operator=(const Value&) = delete;

    void init(GType type);
};

// Bit-compatible with GParameter
class Parameter : public GParameter
{
public:
    explicit Parameter(const char* name = nullptr);
    ~Parameter();

    Parameter(Parameter&&);
    Parameter& operator=(Parameter&&);

    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
};

using ValueArray = std::vector<Value>;
using ParameterArray = std::vector<Parameter>;

} // namespace moo
