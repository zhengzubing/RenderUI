#include "Widget.hpp"
#include "RenderContext.hpp"
#include "EventLoop.hpp"
#include <algorithm>

namespace Component {

Widget::Widget() = default;

Widget::~Widget() = default;

void Widget::setPosition(float x, float y) {
    x_ = x;
    y_ = y;
    markDirty();
}

void Widget::setSize(float width, float height) {
    width_ = width;
    height_ = height;
    markDirty();
}

void Widget::setVisible(bool visible) {
    visible_ = visible;
    markDirty();
}

Vec2 Widget::getPosition() const {
    return Vec2(x_, y_);
}

Size Widget::getSize() const {
    return Size(width_, height_);
}

bool Widget::isVisible() const {
    return visible_;
}

bool Widget::needsRender() const {
    return dirty_;
}

void Widget::render(RenderContext& ctx) {
    if (!visible_ || !dirty_) {
        return;
    }
    
    auto cairo = ctx.getCairoContext();
    if (cairo) {
        Canvas canvas(cairo);
        onDraw(canvas);
    }
    dirty_ = false;
}

void Widget::addChild(std::shared_ptr<Widget> child) {
    if (child) {
        children_.push_back(child);
        child->parent_ = shared_from_this(); // 需要在外部设置 parent
    }
}

void Widget::removeChild(const std::string& id) {
    std::erase_if(children_, [&id](const auto& child) {
        return child->getId() == id;
    });
}

std::shared_ptr<Widget> Widget::getParent() const {
    return parent_.lock();
}

bool Widget::handleTouchEvent(const TouchEvent& event) {
    // 默认不处理事件
    return false;
}

void Widget::onDraw(Canvas& canvas) {
    // 子类实现具体绘制逻辑
}

void Widget::markDirty() {
    dirty_ = true;
}

} // namespace Component
