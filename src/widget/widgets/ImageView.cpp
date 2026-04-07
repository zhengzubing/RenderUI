#include "widgets/ImageView.hpp"
#include "RenderContext.hpp"
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
    
    auto pos = getPosition();
    auto size = getSize();
    
    // 绘制图片
    canvas.drawImage(pos.x, pos.y, size.width, size.height, surface);
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
