#pragma once

#include "Component/Widget.hpp"
#include <string>

namespace Component {

/**
 * @brief 图片显示控件
 * 
 * 支持加载和显示图片，自动缩放
 */
class ImageView : public Widget {
public:
    explicit ImageView(const std::string& imagePath = "");
    ~ImageView() override;
    
    /**
     * @brief 设置图片路径
     */
    void setImage(const std::string& path);
    
    /**
     * @brief 获取图片路径
     */
    const std::string& getImagePath() const { return imagePath_; }
    
    /**
     * @brief 设置缩放模式
     */
    enum class ScaleMode {
        None,       // 原始大小
        FitWidth,   // 适应宽度
        FitHeight,  // 适应高度
        FitBoth,    // 适应两边（保持比例）
        Fill        // 填充（可能变形）
    };
    
    void setScaleMode(ScaleMode mode);
    
    /**
     * @brief 获取缩放模式
     */
    ScaleMode getScaleMode() const { return scaleMode_; }
    
    /**
     * @brief 是否保持宽高比
     */
    void setKeepAspectRatio(bool keep);
    
    /**
     * @brief 自动调整大小到图片原始大小
     */
    void adjustSize();
    
protected:
    void onDraw(Canvas& canvas) override;
    
private:
    std::string imagePath_;
    ScaleMode scaleMode_ = ScaleMode::FitBoth;
    bool keepAspectRatio_ = true;
    
    int imageWidth_ = 0;
    int imageHeight_ = 0;
};

} // namespace Component
