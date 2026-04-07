#include "widgets/Label.hpp"
#include "RenderContext.hpp"
#include "Logger.hpp"

namespace Component {

Label::Label(const std::string& text) : text_(text) {}

Label::~Label() = default;

void Label::setText(const std::string& text) {
    if (text_ != text) {
        text_ = text;
        markDirty();
    }
}

void Label::setFontSize(float size) {
    if (fontSize_ != size) {
        fontSize_ = size;
        markDirty();
    }
}

void Label::setFontFamily(const std::string& family) {
    if (fontFamily_ != family) {
        fontFamily_ = family;
        markDirty();
    }
}

void Label::setTextColor(const Color& color) {
    if (textColor_.r != color.r || textColor_.g != color.g || 
        textColor_.b != color.b || textColor_.a != color.a) {
        textColor_ = color;
        markDirty();
    }
}

void Label::adjustSize() {
    // TODO: 测量文本实际大小
    // 这里简化处理，根据字符数估算
    float charWidth = fontSize_ * 0.6f;
    float textHeight = fontSize_ * 1.2f;
    
    setSize(text_.length() * charWidth, textHeight);
    markDirty();
}

void Label::onDraw(Canvas& canvas) {
    LOG_I << "Label onDraw";
    if (text_.empty()) {
        return;
    }
    
    auto pos = getPosition();
    canvas.drawText(pos.x, pos.y + fontSize_, text_.c_str(), 
                    fontFamily_.c_str(), fontSize_, textColor_);
}

void Label::fromJson(const json& config) {
    // 先调用基类处理公共属性
    Widget::fromJson(config);
    
    // 处理 Label 特有属性
    if (config.contains("text")) {
        setText(config["text"].get<std::string>());
    }
    if (config.contains("fontSize")) {
        setFontSize(config["fontSize"].get<float>());
    }
    if (config.contains("fontFamily")) {
        setFontFamily(config["fontFamily"].get<std::string>());
    }
    if (config.contains("color")) {
        std::string colorStr = config["color"].get<std::string>();
        setTextColor(Color::fromHex(colorStr.c_str()));
    }
}

} // namespace Component
