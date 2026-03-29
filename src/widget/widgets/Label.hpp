#pragma once

#include "Widget.hpp"
#include <string>

namespace Component {

/**
 * @brief 文本标签控件
 * 
 * 用于显示静态文本或动态文本
 */
class Label : public Widget {
public:
    explicit Label(const std::string& text = "");
    ~Label() override;
    
    /**
     * @brief 设置文本内容
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取文本内容
     */
    const std::string& getText() const { return text_; }
    
    /**
     * @brief 设置字体大小
     */
    void setFontSize(float size);
    
    /**
     * @brief 获取字体大小
     */
    float getFontSize() const { return fontSize_; }
    
    /**
     * @brief 设置字体名称
     */
    void setFontFamily(const std::string& family);
    
    /**
     * @brief 获取字体名称
     */
    const std::string& getFontFamily() const { return fontFamily_; }
    
    /**
     * @brief 设置文本颜色
     */
    void setTextColor(const Color& color);
    
    /**
     * @brief 获取文本颜色
     */
    Color getTextColor() const { return textColor_; }
    
    /**
     * @brief 自动调整大小以适应文本
     */
    void adjustSize();
    
protected:
    void onDraw(Canvas& canvas) override;
    
private:
    std::string text_;
    float fontSize_ = 14.0f;
    std::string fontFamily_ = "Sans";
    Color textColor_{1.0f, 1.0f, 1.0f, 1.0f};  // 默认白色
};

} // namespace Component
