#include "Component/Widget.hpp"
#include "Component/RenderContext.hpp"
#include <algorithm>

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

Vec2 Widget::getPosition() const {
    return Vec2(impl_->x, impl_->y);
}

Size Widget::getSize() const {
    return Size(impl_->width, impl_->height);
}

bool Widget::isVisible() const {
    return impl_->visible;
}

bool Widget::needsRender() const {
    return impl_->dirty;
}

void Widget::render(RenderContext& ctx) {
    if (!impl_->visible || !impl_->dirty) {
        return;
    }
    
    auto cairo = ctx.getCairoContext();
    if (cairo) {
        Canvas canvas(cairo);
        onDraw(canvas);
    }
    impl_->dirty = false;
}

void Widget::addChild(std::shared_ptr<Widget> child) {
    if (child) {
        children_.push_back(child);
        // child->parent_ = shared_from_this(); // 需要在外部设置 parent
    }
}

void Widget::removeChild(const std::string& id) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
            [&id](const std::shared_ptr<Widget>& child) {
                return child->getId() == id;
            }),
        children_.end()
    );
}

std::shared_ptr<Widget> Widget::getParent() const {
    return parent_.lock();
}

bool Widget::handleTouchEvent(const TouchEvent& event) {
    // 默认不处理事件
    return false;
}

bool Widget::handleKeyEvent(const KeyEvent& event) {
    // 默认不处理事件
    return false;
}

void Widget::onDraw(Canvas& canvas) {
    // 子类实现具体绘制逻辑
}

void Widget::markDirty() {
    impl_->dirty = true;
}

} // namespace Component
