#include "Widget.hpp"
#include "CairoGlRenderer.hpp"
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

bool Widget::isDirty() const {
    return dirty_;
}

void Widget::updateTexture(CairoGlRenderer& ctx) {
    if (!visible_) {
        return;
    }
    
    // 获取 Widget 的 Cairo 上下文
    int w = static_cast<int>(width_);
    int h = static_cast<int>(height_);
    
    if (w <= 0 || h <= 0) {
        return;
    }
    
    auto cairo = ctx.getCairoContext(id_, w, h);
    if (!cairo) {
        LOG_E << "Failed to get widget context: " << id_;
        return;
    }
    
    // 更新位置信息（无论是否需要重绘都要更新）
    ctx.updateWidgetPosition(id_, x_, y_, static_cast<float>(w), static_cast<float>(h));
    
    // 如果不需要重绘，直接返回
    if (!dirty_) {
        return;
    }
    
    LOG_D << "Widget render: " << id_ << " at (" << x_ << ", " << y_ << ") size: " << w << "x" << h;
    
    // 清除画布
    cairo_save(cairo);
    cairo_set_operator(cairo, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cairo);
    cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
    cairo_restore(cairo);
    
    // 创建 Canvas 并绘制
    Canvas canvas(cairo);
    onDraw(canvas);
    
    // 标记为已渲染
    dirty_ = false;
    
    // 标记纹理需要更新
    ctx.markWidgetDirty(id_);
}

void Widget::updateZIndex(CairoGlRenderer& ctx) {
    // 同步 Z 序到渲染器
    ctx.updateWidgetZIndex(id_, zIndex_);
}

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
    if (zIndex_ != zIndex) {
        zIndex_ = zIndex;
        markDirty();  // zIndex 变化需要重绘
        LOG_D << "Widget zIndex changed: " << id_ << " -> " << zIndex;
    }
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
