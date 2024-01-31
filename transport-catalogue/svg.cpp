#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ----------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";
    bool first = true;
    for(auto p : points_){
        if(first){
            first = false;
        } else {
            out << ' ';
        }
        out << p.x << ',' << p.y;
    }
    out <<"\"";
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text --------------
// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    std::string font_weight = (font_weight_.empty()) ? ""s : font_weight_;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\"" << position_.x << "\" y=\""sv << position_.y << "\" "sv
        << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" "sv
        << "font-size=\""sv << font_size_ << "\""sv;
    if(!font_family_.empty()){
        out << " font-family=\"" << font_family_ << "\"";
    }
    if(!font_weight_.empty()){
        out << " font-weight=\""sv << font_weight_ << "\"";
    }
    out << '>';
    out << RenderText() << "</text>";
}

    std::string Text::RenderText() const{
        std::string result;
        for(auto c : data_){
            if(c == '\"'){
                result += "&quot;";
            } else if(c == '\''){
                result += "&apos;";
            }else if(c == '<'){
                result += "&lt;";
            }else if(c == '>'){
                result += "&gt;";
            }else if(c == '&'){
                result += "&amp;";
            }else{
                result += c;
            }
        }
        return result;
    }

// ---------- Document -----------
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj));
}
void Document::Render(std::ostream& out) const{
    RenderContext context(out, 0, indent_step_);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    auto obj_context = context.Indented();
    for(size_t i = 0; i < objects_.size(); ++i){
        objects_[i]->Render(obj_context);
    }
    out << "</svg>"sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap cap){
    if(cap == StrokeLineCap::BUTT){
        out << "butt"sv;
    } else if(cap == StrokeLineCap::ROUND){
        out << "round"sv;
    }else if(cap == StrokeLineCap::SQUARE){
        out << "square"sv;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin cap){
    if(cap == StrokeLineJoin::ARCS){
        out << "arcs"sv;
    } else if(cap == StrokeLineJoin::BEVEL){
        out << "bevel"sv;
    }else if(cap == StrokeLineJoin::MITER){
        out << "miter"sv;
    }else if(cap == StrokeLineJoin::MITER_CLIP){
        out << "miter-clip"sv;
    }else if(cap == StrokeLineJoin::ROUND){
        out << "round"sv;
    }
    return out;
}
}  // namespace svg