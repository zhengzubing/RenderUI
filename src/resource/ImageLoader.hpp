#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <cairo/cairo.h>

namespace Component {

/**
 * @brief 图片数据
 */
struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 0;
    size_t dataSize = 0;
    std::unique_ptr<uint8_t[]> data;
};

/**
 * @brief 图片加载器
 * 
 * 使用 stb_image 加载各种格式的图片
 */
class ImageLoader {
public:
    ImageLoader();
    ~ImageLoader();
    
    /**
     * @brief 加载图片文件
     * @param path 图片路径
     * @return 图片数据，失败返回 nullptr
     */
    std::unique_ptr<ImageData> load(const std::string& path);
    
    /**
     * @brief 加载图片并创建 Cairo 表面
     * @param path 图片路径
     * @return Cairo 表面，失败返回 nullptr
     */
    cairo_surface_t* loadToCairoSurface(const std::string& path);
    
    /**
     * @brief 获取加载失败原因
     */
    static const char* getFailureReason();
};

} // namespace Component
