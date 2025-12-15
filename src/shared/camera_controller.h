// camera_controller.h
// 摄像头控制类，用于处理摄像头的初始化、参数设置和帧获取

#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

// 使用libcamera API
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

namespace cinepi {

// 摄像头参数结构体
struct CameraParams {
    int width;
    int height;
    int fps;
    int bit_depth;
    float exposure_compensation;
    int iso;
    int white_balance;

    CameraParams(int w = 1280, int h = 720, int f = 30, int bd = 12, float ec = 0.0f, int i = 100, int wb = 4000)
        : width(w), height(h), fps(f), bit_depth(bd), exposure_compensation(ec), iso(i), white_balance(wb) {}
};

// 摄像头控制类
class CameraController {
public:
    CameraController();
    ~CameraController();

    // 初始化摄像头
    void Initialize(const CameraParams& params = CameraParams());

    // 启动预览
    void StartPreview();

    // 停止预览
    void StopPreview();

    // 获取预览帧数据
    const uint8_t* GetPreviewFrame();

    // 开始录制
    void StartRecording(const std::string& filename);

    // 停止录制
    void StopRecording();

    // 调整曝光补偿
    void AdjustExposure(float delta);

    // 调整ISO
    void AdjustISO(int delta);

    // 循环切换白平衡
    void CycleWhiteBalance();

    // 设置参数
    void SetResolution(int width, int height);
    void SetFPS(int fps);
    void SetBitDepth(int bit_depth);
    void SetExposureCompensation(float value);
    void SetISO(int value);
    void SetWhiteBalance(int value);

    // 获取参数
    int GetWidth() const { return params_.width; }
    int GetHeight() const { return params_.height; }
    int GetFPS() const { return params_.fps; }
    int GetBitDepth() const { return params_.bit_depth; }
    float GetExposureCompensation() const { return params_.exposure_compensation; }
    int GetISO() const { return params_.iso; }
    int GetWhiteBalance() const { return params_.white_balance; }
    bool IsPreviewing() const { return is_previewing_; }
    bool IsRecording() const { return is_recording_; }

private:
    // libcamera相关成员
    std::unique_ptr<libcamera::CameraManager> camera_manager_;
    std::shared_ptr<libcamera::Camera> camera_;
    std::unique_ptr<libcamera::CameraConfiguration> config_;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator_;
    libcamera::Stream* stream_;
    libcamera::Request* request_;
    const libcamera::FrameBuffer* current_buffer_;
    uint8_t* preview_buffer_;

    // 应用参数
    CameraParams params_;
    bool is_initialized_;
    bool is_previewing_;
    bool is_recording_;

    // 辅助方法
    void setupControls();
    void processRequest(libcamera::Request* request);
};

} // namespace cinepi

#endif // CAMERA_CONTROLLER_H