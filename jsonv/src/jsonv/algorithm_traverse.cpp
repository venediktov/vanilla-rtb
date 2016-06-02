/** \file
 *  
 *  Copyright (c) 2014 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/algorithm.hpp>
#include <jsonv/path.hpp>
#include <jsonv/value.hpp>

namespace jsonv
{

void traverse(const value&                                           tree,
              const std::function<void (const path&, const value&)>& func,
              const path&                                            base_path,
              bool                                                   leafs_only
             )
{
    if (!leafs_only || tree.empty() || (tree.kind() != kind::array && tree.kind() != kind::object))
        func(base_path, tree);
    
    if (tree.kind() == kind::object)
    {
        for (const auto& field : tree.as_object())
        {
            traverse(field.second,
                     func,
                     base_path + field.first,
                     leafs_only
                    );
        }
    }
    else if (tree.kind() == kind::array)
    {
        for (value::size_type idx = 0; idx < tree.size(); ++idx)
            traverse(tree[idx],
                     func,
                     base_path + idx,
                     leafs_only
                    );
    }
}

void traverse(const value&                                           tree,
              const std::function<void (const path&, const value&)>& func,
              bool                                                   leafs_only
             )
{
    traverse(tree, func, path(), leafs_only);
}

}
