#include "Component/widgets/ImageView.hpp"
#include "Component/RenderContext.hpp"
#include "Component/ResourceManager.hpp"

namespace Component {

ImageView::ImageView(const std::string& imagePath) : imagePath_(imagePath) {}

ImageView::~ImageView() = default;

void ImageView::setImage(const std::string& path) {
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

} // namespace Component
