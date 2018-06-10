#include "procgui.h"
#include "LSystemView.h"
#include "helper_math.h"

namespace procgui
{
    using namespace drawing;
    using namespace colors;

    int LSystemView::id_count_ = 0;
    UniqueColor LSystemView::color_gen_ {};
    
    LSystemView::LSystemView(const std::string& name,
                             std::shared_ptr<LSystem> lsys,
                             std::shared_ptr<drawing::InterpretationMap> map,
                             drawing::DrawingParameters params,
                             std::shared_ptr<VertexPainter> painter)
        : Observer<LSystem> {lsys}
        , Observer<InterpretationMap> {map}
        , Observer<VertexPainter> {painter}
        , id_{id_count_++}
        , color_id_{color_gen_.register_id(id_)}
        , name_ {name}
        , lsys_buff_ {lsys}
        , interpretation_buff_ {map}
        , params_ {params}
        , vertices_ {}
        , bounding_box_ {}
        , sub_boxes_ {}
        , is_selected_ {false}
    {
        // Invariant respected: cohesion between the LSystem/InterpretationMap
        // and the vertices. 
        Observer<LSystem>::add_callback([this](){compute_vertices();});
        Observer<InterpretationMap>::add_callback([this](){compute_vertices();});
        Observer<VertexPainter>::add_callback([this](){paint_vertices();});
        compute_vertices();
    }

    LSystemView::LSystemView(const std::string& name,
                             std::shared_ptr<LSystem> lsys,
                             std::shared_ptr<drawing::InterpretationMap> map,
                             drawing::DrawingParameters param)
        : LSystemView(name, lsys, map, param, std::make_shared<VertexPainter>())
    {
    }

    
    LSystemView::LSystemView(const sf::Vector2f& position)
        : LSystemView(
            "",
            std::make_shared<LSystem>(LSystem("F+F+F+F", {})),
            std::make_shared<drawing::InterpretationMap>(drawing::default_interpretation_map),
            drawing::DrawingParameters({position}))
    {
        // Arbitrary default LSystem.
    }

    LSystemView::LSystemView(const LSystemView& other)
        : Observer<LSystem> {other.Observer<LSystem>::get_target()}
        , Observer<InterpretationMap> {other.Observer<InterpretationMap>::get_target()}
        , Observer<VertexPainter> {other.Observer<VertexPainter>::get_target()}
        , id_ {id_count_++}
        , color_id_{color_gen_.register_id(id_)}
        , name_ {other.name_}
        , lsys_buff_ {other.lsys_buff_}
        , interpretation_buff_ {other.interpretation_buff_}
        , params_ {other.params_}
        , vertices_ {other.vertices_}
        , bounding_box_ {other.bounding_box_}
        , sub_boxes_ {other.sub_boxes_}
        , is_selected_ {other.is_selected_}
    {
        // Manually managing Observer<> callbacks.
        Observer<LSystem>::add_callback([this](){compute_vertices();});
        Observer<InterpretationMap>::add_callback([this](){compute_vertices();});
        Observer<VertexPainter>::add_callback([this](){paint_vertices();});
    }

    LSystemView::LSystemView(LSystemView&& other)
        : Observer<LSystem> {other.Observer<LSystem>::get_target()}
        , Observer<InterpretationMap> {other.Observer<InterpretationMap>::get_target()}
        , Observer<VertexPainter> {other.Observer<VertexPainter>::get_target()}
        , id_ {other.id_}
        , color_id_{std::move(other.color_id_)}
        , name_ {std::move(other.name_)}
        , lsys_buff_ {std::move(other.lsys_buff_)}
        , interpretation_buff_ {std::move(other.interpretation_buff_)}
        , params_ {std::move(other.params_)}
        , vertices_ {std::move(other.vertices_)}
        , bounding_box_ {std::move(other.bounding_box_)}
        , sub_boxes_ {std::move(other.sub_boxes_)}
        , is_selected_ {other.is_selected_}
    {
        // Manually managing Observer<> callbacks.
        Observer<LSystem>::add_callback([this](){compute_vertices();});
        Observer<InterpretationMap>::add_callback([this](){compute_vertices();});
        Observer<VertexPainter>::add_callback([this](){paint_vertices();});

        // Remove callbacks of the moved 'other'.
        other.Observer<LSystem>::set_target(nullptr);
        other.Observer<InterpretationMap>::set_target(nullptr);
        other.Observer<VertexPainter>::set_target(nullptr);

        // the 'other' object must not matter in the 'color_gen_' anymore.
        other.id_ = -1;
        other.color_id_ = sf::Color::Black;
        other.params_ = {};
        other.bounding_box_ = {};
        other.is_selected_ = false;
    }

    LSystemView& LSystemView::operator=(LSystemView other)
    {
        swap(other);
        return *this;
    }

    void LSystemView::swap(LSystemView& other)
    {
        using std::swap;

        // Not a pure swap but Observer<> must be manually swaped:
        //  - swap the targets.
        //  - add correct callback.
        auto tmp_lsys = Observer<LSystem>::get_target();
        Observer<LSystem>::set_target(other.Observer<LSystem>::get_target());
        other.Observer<LSystem>::set_target(tmp_lsys);
    
        Observer<LSystem>::add_callback([this](){compute_vertices();});
        other.Observer<LSystem>::add_callback([&other](){other.compute_vertices();});

        auto tmp_map = Observer<InterpretationMap>::get_target();
        Observer<InterpretationMap>::set_target(other.Observer<InterpretationMap>::get_target());
        other.Observer<InterpretationMap>::set_target(tmp_map);

        auto tmp_painter = Observer<VertexPainter>::get_target();
        Observer<VertexPainter>::set_target(other.Observer<VertexPainter>::get_target());
        other.Observer<VertexPainter>::set_target(tmp_painter);
        
        Observer<InterpretationMap>::add_callback([this](){compute_vertices();});
        other.Observer<InterpretationMap>::add_callback([&other](){other.compute_vertices();});
        other.Observer<VertexPainter>::add_callback([&other](){other.compute_vertices();});

        swap(id_, other.id_);
        swap(color_id_, other.color_id_);
        swap(name_, other.name_);
        swap(lsys_buff_, other.lsys_buff_);
        swap(interpretation_buff_, other.interpretation_buff_);
        swap(params_, other.params_);
        swap(vertices_, other.vertices_);
        swap(bounding_box_, other.bounding_box_);
        swap(sub_boxes_, other.sub_boxes_);
        swap(is_selected_, other.is_selected_);
    }

    LSystemView::~LSystemView()
    {
        // Unregister the id unless the object was moved.
        if (id_ != -1)
        {
            color_gen_.remove_id(id_);
        }
    }


    LSystemView LSystemView::clone()
    {        
        // Deep copy.
        return LSystemView(
            name_,
            std::make_shared<LSystem>(*lsys_buff_.get_target()),
            std::make_shared<drawing::InterpretationMap>(*interpretation_buff_.get_target()),
            params_
            );
    }

    LSystemView LSystemView::duplicate()
    {
        return LSystemView(
            name_,
            lsys_buff_.get_target(),
            interpretation_buff_.get_target(),
            params_,
            Observer<VertexPainter>::get_target());
    }
    

    drawing::DrawingParameters& LSystemView::ref_parameters()
    {
        return params_;
    }
    LSystemBuffer& LSystemView::ref_lsystem_buffer()
    {
        return lsys_buff_;
    }
    InterpretationMapBuffer& LSystemView::ref_interpretation_buffer()
    {
        return interpretation_buff_;
    }
    colors::VertexPainter& LSystemView::ref_vertex_painter()
    {
        return *Observer<VertexPainter>::get_target();
    }
    sf::FloatRect LSystemView::get_bounding_box() const
    {
        return get_transform().transformRect(bounding_box_);
    }
    const drawing::DrawingParameters& LSystemView::get_parameters() const
    {
        return params_;
    }
    const LSystemBuffer& LSystemView::get_lsystem_buffer() const
    {
        return lsys_buff_;
    }
    const InterpretationMapBuffer& LSystemView::get_interpretation_buffer() const
    {
        return interpretation_buff_;
    }
    const colors::VertexPainter& LSystemView::get_vertex_painter()
    {
        return *Observer<VertexPainter>::get_target();
    }
    int LSystemView::get_id() const
    {
        return id_;
    }
    sf::Color LSystemView::get_color() const
    {
        return color_id_;
    }
    sf::Transform LSystemView::get_transform() const
    {
        sf::Transform transform;
        transform.translate(params_.starting_position);
        return transform;
    }
    
    void LSystemView::compute_vertices()
    {
        // Invariant respected: cohesion between the vertices and the bounding
        // boxes. 
        
        vertices_ = drawing::compute_vertices(*Observer<LSystem>::get_target(),
                                              *Observer<InterpretationMap>::get_target(),
                                              params_);
        bounding_box_ = geometry::compute_bounding_box(vertices_);
        sub_boxes_ = geometry::compute_sub_boxes(vertices_, MAX_SUB_BOXES);
        paint_vertices();
    }

    void LSystemView::paint_vertices()
    {
        // un-transformed vertices and bounding box
        Observer<VertexPainter>::get_target()->paint_vertices(vertices_, bounding_box_);
    }

    
    void LSystemView::draw(sf::RenderTarget &target)
    {
        // Interact with the models and re-compute the vertices if there is a
        // modification. 
        if (interact_with(*this, name_, true, &is_selected_))
        {
            compute_vertices();
        }
        Observer<VertexPainter>::get_target()->interact_with();

        // Early out if there are no vertices.
        if (vertices_.size() == 0)
        {
            return;
        }

        // Draw the vertices.
        target.draw(vertices_.data(), vertices_.size(), sf::LineStrip, get_transform());

        if (is_selected_)
        {
            auto box = get_transform().transformRect(bounding_box_);
            // Draw the global bounding boxes with the unique color.
            std::array<sf::Vertex, 5> rect =
                {{ {{ box.left, box.top}, color_id_},
                   {{ box.left, box.top + box.height}, color_id_},
                   {{ box.left + box.width, box.top + box.height}, color_id_},
                   {{ box.left + box.width, box.top}, color_id_},
                   {{ box.left, box.top}, color_id_}}};
            target.draw(rect.data(), rect.size(), sf::LineStrip);
        }

        // // DEBUG
        // // Draw the sub-bounding boxes.
        // for (const auto& box : sub_boxes_)
        // {
        //     std::array<sf::Vertex, 5> rect =
        //         {{ {{ box.left, box.top}, sf::Color(255,0,0,50)},
        //            {{ box.left, box.top + box.height}, sf::Color(255,0,0,50)},
        //            {{ box.left + box.width, box.top + box.height}, sf::Color(255,0,0,50)},
        //            {{ box.left + box.width, box.top}, sf::Color(255,0,0,50)}}};
        //     target.draw(rect.data(), rect.size(), sf::Quads);
        // }
    }

    bool LSystemView::is_selected() const
    {
        return is_selected_;
    }

    bool LSystemView::is_inside(const sf::Vector2f& click) const
    {
        decltype(sub_boxes_) subs;
        for (const auto& box : sub_boxes_)
        {
            subs.push_back(get_transform().transformRect(box));
        }
        for (const auto& rect : subs)
        {
            if (rect.contains(sf::Vector2f(click)))
            {
                return true;
            }
        }
        return false;
    }
    
    void LSystemView::select()
    {
        is_selected_ = true;
    }
}