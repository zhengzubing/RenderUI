#include "widgets/ImageView.hpp"
#include "CairoGlRenderer.hpp"
#include "ResourceManager.hpp"
#include "Logger.hpp"

namespace Component {

ImageView::ImageView(const std::string& imagePath) : imagePath_(imagePath) {}

ImageView::~ImageView() = default;

void ImageView::setImage(const std::string& path) {
    LOG_I << "ImageView setImage: " << path;
    if (imagePath_ != path) {
        imagePath_ = path;
        
        // 加载图片获取原始大小
        auto surface = ResourceManager::instance().loadImage(path);
        if (surface) {
            imageWidth_ = cairo_image_surface_get_width(surface);
            imageHeight_ = cairo_image_surface_get_height(surface);
            
            // 根据缩放模式调整大小
            auto size = getSize();
            if (size.width == 0 && size.height == 0) {
                setSize(static_cast<float>(imageWidth_), static_cast<float>(imageHeight_));
            }
        }
        
        markDirty();
    }
}

void ImageView::setScaleMode(ScaleMode mode) {
    if (scaleMode_ != mode) {
        scaleMode_ = mode;
        markDirty();
    }
}

void ImageView::setKeepAspectRatio(bool keep) {
    keepAspectRatio_ = keep;
}

void ImageView::adjustSize() {
    if (imageWidth_ > 0 && imageHeight_ > 0) {
        setSize(static_cast<float>(imageWidth_), static_cast<float>(imageHeight_));
        markDirty();
    }
}

void ImageView::onDraw(Canvas& canvas) {
    LOG_I << "ImageView onDraw";
    if (imagePath_.empty()) {
        return;
    }
    
    auto surface = ResourceManager::instance().loadImage(imagePath_);
    if (!surface) {
        return;
    }
    
    auto size = getSize();
    
    // 根据缩放模式计算实际绘制尺寸
    float drawWidth = size.width;
    float drawHeight = size.height;
    
    if (imageWidth_ > 0 && imageHeight_ > 0) {
        float imgAspect = static_cast<float>(imageWidth_) / static_cast<float>(imageHeight_);
        float widgetAspect = size.width / size.height;
        
        switch (scaleMode_) {
            case ScaleMode::None:
                // 原始大小
                drawWidth = static_cast<float>(imageWidth_);
                drawHeight = static_cast<float>(imageHeight_);
                break;
                
            case ScaleMode::FitWidth:
                // 适应宽度，高度按比例缩放
                drawWidth = size.width;
                drawHeight = size.width / imgAspect;
                break;
                
            case ScaleMode::FitHeight:
                // 适应高度，宽度按比例缩放
                drawHeight = size.height;
                drawWidth = size.height * imgAspect;
                break;
                
            case ScaleMode::FitBoth:
                // 适应两边（保持比例，letterbox 模式）
                if (keepAspectRatio_) {
                    if (widgetAspect > imgAspect) {
                        // 控件更宽，以高度为准
                        drawHeight = size.height;
                        drawWidth = size.height * imgAspect;
                    } else {
                        // 控件更高，以宽度为准
                        drawWidth = size.width;
                        drawHeight = size.width / imgAspect;
                    }
                } else {
                    // 不保持比例，直接填充
                    drawWidth = size.width;
                    drawHeight = size.height;
                }
                break;
                
            case ScaleMode::Fill:
                // 填充整个控件（可能变形）
                drawWidth = size.width;
                drawHeight = size.height;
                break;
        }
    }
    
    // Cairo 只在 (0, 0) 位置绘制，位置由 OpenGL 变换处理
    LOG_I << "ImageView drawImage: size(" << drawWidth << ", " << drawHeight << ")";
    canvas.drawImage(0, 0, drawWidth, drawHeight, surface);
}

void ImageView::fromJson(const json& config) {
    LOG_I << "ImageView fromJson";
    // 先调用基类处理公共属性
    Widget::fromJson(config);
    
    // 处理 ImageView 特有属性
    if (config.contains("imagePath")) {
        setImage(config["imagePath"].get<std::string>());
    }
    if (config.contains("scaleMode")) {
        std::string modeStr = config["scaleMode"].get<std::string>();
        if (modeStr == "FitWidth") setScaleMode(ScaleMode::FitWidth);
        else if (modeStr == "FitHeight") setScaleMode(ScaleMode::FitHeight);
        else if (modeStr == "FitBoth") setScaleMode(ScaleMode::FitBoth);
        else if (modeStr == "Fill") setScaleMode(ScaleMode::Fill);
    }
    if (config.contains("keepAspectRatio")) {
        setKeepAspectRatio(config["keepAspectRatio"].get<bool>());
    }
}

} // namespace Component
