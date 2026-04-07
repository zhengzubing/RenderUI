#include "Widget.hpp"
#include "RenderContext.hpp"
#include "EventLoop.hpp"
#include "Logger.hpp"
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

float Widget::getX() const {
    return x_;
}

void Widget::setX(float x) {
    x_ = x;
    markDirty();
}

float Widget::getY() const {
    return y_;
}

void Widget::setY(float y) {
    y_ = y;
    markDirty();
}

Size Widget::getSize() const {
    return Size(width_, height_);
}

float Widget::getWidth() const {
    return width_;
}

void Widget::setWidth(float width) {
    width_ = width;
    markDirty();
}

float Widget::getHeight() const {
    return height_;
}

void Widget::setHeight(float height) {
    height_ = height;
    markDirty();
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
    LOG_I << "Widget render" << getId();
    
    auto cairo = ctx.getCairoContext();
    if (cairo) {
        Canvas canvas(cairo);
        LOG_I << "Widget onDraw" << getId();
        onDraw(canvas);
    }
    dirty_ = false;
}

// void Widget::addChild(std::shared_ptr<Widget> child) {
//     if (child) {
//         children_.push_back(child);
//         child->parent_ = shared_from_this(); // 需要在外部设置 parent
//     }
// }

// void Widget::removeChild(const std::string& id) {
//     std::erase_if(children_, [&id](const auto& child) {
//         return child->getId() == id;
//     });
// }

std::shared_ptr<Widget> Widget::getParent() const {
    return parent_.lock();
}

const std::string& Widget::getId() const {
    return id_;
}

void Widget::setId(const std::string& id) {
    id_ = id;
}

int Widget::getZIndex() const {
    return zIndex_;
}

void Widget::setZIndex(int zIndex) {
    zIndex_ = zIndex;
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

void Widget::fromJson(const json& config) {
    // 默认实现：处理公共属性
    if (config.contains("position")) {
        const auto& pos = config["position"];
        float x = pos.value("x", 0.0f);
        float y = pos.value("y", 0.0f);
        setPosition(x, y);
    }
    
    if (config.contains("size")) {
        const auto& size = config["size"];
        float w = size.value("width", 0.0f);
        float h = size.value("height", 0.0f);
        setSize(w, h);
    }
    
    if (config.contains("x")) {
        float x = config.value("x", 0.0f);
        setX(x);
    }

    if (config.contains("y")) {
        float y = config.value("y", 0.0f);
        setY(y);
    }

    if (config.contains("width")) {
        float w = config.value("width", 0.0f);
        setWidth(w);
    }

    if (config.contains("height")) {
        float h = config.value("height", 0.0f);
        setHeight(h);
    }

    if (config.contains("id")) {
        setId(config["id"].get<std::string>());
    }
    
    if (config.contains("zIndex")) {
        setZIndex(config["zIndex"].get<int>());
    }
    
    if (config.contains("visible")) {
        setVisible(config["visible"].get<bool>());
    }
}

} // namespace Component
