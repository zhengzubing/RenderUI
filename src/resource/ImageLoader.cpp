#include "ImageLoader.hpp"
#include "Logger.hpp"
#include <stb_image.h>
#include <cstring>

namespace Component {

ImageLoader::ImageLoader() = default;

ImageLoader::~ImageLoader() = default;

std::unique_ptr<ImageData> ImageLoader::load(const std::string& path) {
    int width = 0, height = 0, channels = 0;
    
    // 使用 stb_image 加载图片
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        LOG_ERROR("Failed to load image: %s - %s", path.c_str(), stbi_failure_reason());
        return nullptr;
    }
    
    auto imageData = std::make_unique<ImageData>();
    imageData->width = width;
    imageData->height = height;
    imageData->channels = 4;  // 强制转换为 RGBA
    imageData->dataSize = width * height * 4;
    imageData->data.reset(new uint8_t[imageData->dataSize]);
    
    // 复制数据并转换为 BGRA 格式（Cairo 需要）
    for (int y = 0; y < height; y++) {
        const unsigned char* srcRow = data + y * width * 4;
        uint8_t* dstRow = imageData->data.get() + y * width * 4;
        
        for (int x = 0; x < width; x++) {
            // RGBA -> BGRA (小端 ARGB)
            dstRow[x * 4 + 0] = srcRow[x * 4 + 2];  // B
            dstRow[x * 4 + 1] = srcRow[x * 4 + 1];  // G
            dstRow[x * 4 + 2] = srcRow[x * 4 + 0];  // R
            dstRow[x * 4 + 3] = srcRow[x * 4 + 3];  // A
        }
    }
    
    stbi_image_free(data);
    
    LOG_INFO("Image loaded: %s (%dx%d, %d channels)", path.c_str(), width, height, channels);
    return imageData;
}

cairo_surface_t* ImageLoader::loadToCairoSurface(const std::string& path) {
    auto imageData = load(path);
    if (!imageData) {
        return nullptr;
    }
    
    // 创建 Cairo 表面
    cairo_surface_t* surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, imageData->width, imageData->height);
    
    if (!surface) {
        LOG_ERROR("Failed to create Cairo surface");
        return nullptr;
    }
    
    // 复制数据到 Cairo 表面
    unsigned char* cairoData = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    for (int y = 0; y < imageData->height; y++) {
        std::memcpy(cairoData + y * stride, 
                    imageData->data.get() + y * imageData->width * 4,
                    imageData->width * 4);
    }
    
    cairo_surface_mark_dirty(surface);
    
    return surface;
}

const char* ImageLoader::getFailureReason() {
    return stbi_failure_reason();
}

} // namespace Component
