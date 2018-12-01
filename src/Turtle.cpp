#include "Turtle.h"

namespace drawing
{
    using namespace impl;
    
    Turtle::Turtle(const DrawingParameters& params,
                   const std::vector<int>& recursions)
        : parameters { params }
        , str_recursion {recursions}
        , vertices_recursion {}
          // The other members are set in header as they all derives from
          // 'parameters' or 'str_recursion'.
    {
        if (str_recursion.size())
        {
            vertices_recursion.push_back(str_recursion.at(0));
        }
    }

    std::pair<std::vector<sf::Vertex>, std::vector<int>>
        compute_vertices(LSystem& lsys,
                         InterpretationMap& interpretation,
                         const DrawingParameters& parameters)

    {
        const auto [str, rec] = lsys.produce(parameters.get_n_iter());
        Turtle turtle (parameters, rec);
        
        for (auto c : str)
        {
            if (interpretation.has_predecessor(c))
            {
                // If an interpretation of the character 'c' is found,
                // applies it to the current turtle.
                interpretation.get_rule(c).second(turtle);
            }
            else
            {
                // Do nothing: if 'c' does not have an associated
                // order, it has no effects.
            }
        }

        Ensures(turtle.vertices.size() == turtle.vertices_recursion.size());
        return {turtle.vertices, turtle.vertices_recursion};
    }
}
