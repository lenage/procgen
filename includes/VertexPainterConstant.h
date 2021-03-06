#ifndef VERTEX_PAINTER_CONSTANT_H
#define VERTEX_PAINTER_CONSTANT_H


#include "VertexPainter.h"

namespace colors
{
    class VertexPainterConstant : public VertexPainter
    {
    public:
        VertexPainterConstant(); // Create a default generator
        explicit VertexPainterConstant(const std::shared_ptr<ColorGenerator> gen);
        // Shallow rule-of-five constructors.
        VertexPainterConstant(const VertexPainterConstant& other);
        VertexPainterConstant(VertexPainterConstant&& other);
        VertexPainterConstant& operator=(const VertexPainterConstant& other);
        VertexPainterConstant& operator=(VertexPainterConstant&& other);
        
        // Paint 'vertices' according to a constant real number.
        // 'bounding_box', 'iteration_of_vertices' and 'max_recursion' are not used.
        virtual void paint_vertices(std::vector<sf::Vertex>& vertices,
                                    const std::vector<int>& iteration_of_vertices,
                                    int max_recursion,
                                    sf::FloatRect bounding_box) override;

    private:
        // Implements the deep-copy cloning.
        virtual std::shared_ptr<VertexPainter> clone_impl() const override;
    };
}

#endif // VERTEX_PAINTER_CONSTANT_H
