#include "Component/Widget.hpp"

namespace Component {

struct Widget::Impl {
    std::string id;
    float x = 0, y = 0;
    float width = 0, height = 0;
    bool visible = true;
    bool dirty = false;
};

Widget::Widget() : impl_(std::make_unique<Impl>()) {}

Widget::~Widget() = default;

void Widget::setPosition(float x, float y) {
    impl_->x = x;
    impl_->y = y;
    markDirty();
}

void Widget::setSize(float width, float height) {
    impl_->width = width;
    impl_->height = height;
    markDirty();
}

void Widget::setVisible(bool visible) {
    impl_->visible = visible;
    markDirty();
}

bool Widget::needsRender() const {
    return impl_->dirty;
}

void Widget::render(RenderContext& ctx) {
    if (!impl_->visible || !impl_->dirty) {
        return;
    }
    
    onDraw(ctx);
    impl_->dirty = false;
}

void Widget::markDirty() {
    impl_->dirty = true;
}

void Widget::onDraw(Canvas& canvas) {
    // 子类实现
}

} // namespace Component
