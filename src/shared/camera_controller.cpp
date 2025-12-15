// camera_controller.cpp
// 摄像头控制类实现

#include "camera_controller.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <libcamera/control_ids.h>
#include <libcamera/formats.h>

namespace cinepi {

CameraController::CameraController() 
    : camera_manager_(nullptr),
      camera_(nullptr),
      config_(nullptr),
      allocator_(nullptr),
      stream_(nullptr),
      request_(nullptr),
      current_buffer_(nullptr),
      preview_buffer_(nullptr),
      is_initialized_(false), 
      is_previewing_(false), 
      is_recording_(false) {
}

CameraController::~CameraController() {
    StopRecording();
    StopPreview();
    if (is_initialized_) {
        // 清理资源
        if (camera_) {
            camera_->release();
        }
        if (camera_manager_) {
            camera_manager_->stop();
        }
        delete[] preview_buffer_;
    }
}

void CameraController::Initialize(const CameraParams& params) {
    if (is_initialized_) {
        return;
    }

    params_ = params;

    try {
        // 初始化相机管理器
        camera_manager_ = std::make_unique<libcamera::CameraManager>();
        if (camera_manager_->start()) {
            throw std::runtime_error("相机管理器初始化失败");
        }

        // 获取相机列表
        auto cameras = camera_manager_->cameras();
        if (cameras.empty()) {
            throw std::runtime_error("未找到相机");
        }

        // 获取第一个相机
        camera_ = cameras[0];
        std::cout << "使用相机: " << camera_->id() << std::endl;

        // 激活相机
        if (camera_->acquire()) {
            throw std::runtime_error("相机获取失败");
        }

        // 生成相机配置
        config_ = camera_->generateConfiguration({
            libcamera::StreamRole::Viewfinder,
            libcamera::StreamRole::Raw
        });

        if (!config_ || config_->validate() == libcamera::CameraConfiguration::Invalid) {
            throw std::runtime_error("相机配置无效");
        }

        // 配置预览流
        libcamera::StreamConfiguration &viewfinder_config = config_->at(0);
        viewfinder_config.size = libcamera::Size(params_.width, params_.height);
        viewfinder_config.pixelFormat = libcamera::formats::RGB888;
        viewfinder_config.bufferCount = 4;

        // 配置相机
        if (camera_->configure(config_.get())) {
            throw std::runtime_error("相机配置失败");
        }

        // 获取预览流
        stream_ = viewfinder_config.stream();

        // 创建帧缓冲分配器
        allocator_ = std::make_unique<libcamera::FrameBufferAllocator>(camera_);
        if (allocator_->allocate(stream_) < 0) {
            throw std::runtime_error("帧缓冲分配失败");
        }

        // 创建预览缓冲区
        preview_buffer_ = new uint8_t[params_.width * params_.height * 3];

        is_initialized_ = true;

        // 设置摄像头参数
        SetExposureCompensation(params_.exposure_compensation);
        SetISO(params_.iso);
        SetWhiteBalance(params_.white_balance);

        std::cout << "相机初始化成功" << std::endl;

    } catch (const std::exception& e) {
        StopPreview();
        StopRecording();
        if (camera_) {
            camera_->release();
        }
        if (camera_manager_) {
            camera_manager_->stop();
        }
        delete[] preview_buffer_;
        preview_buffer_ = nullptr;
        throw e;
    }
}

void CameraController::setupControls() {
    if (!camera_ || !is_initialized_) {
        return;
    }

    libcamera::ControlList controls(camera_->controls());

    // 设置曝光补偿
    controls.set(libcamera::controls::ExposureValue, params_.exposure_compensation);

    // 设置ISO
    // ISO 100 = gain 1.0
    double gain = static_cast<double>(params_.iso) / 100.0;
    controls.set(libcamera::controls::AnalogueGain, gain);

    // 设置白平衡
    controls.set(libcamera::controls::ColourTemperature, params_.white_balance);

    // 应用控制
    camera_->setControls(controls);
}

void CameraController::StartPreview() {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    if (!is_previewing_) {
        try {
            // 清理之前的请求
            if (request_) {
                camera_->queueRequest(request_);
                request_ = nullptr;
            }

            // 创建新请求
            request_ = camera_->createRequest();
            if (!request_) {
                throw std::runtime_error("请求创建失败");
            }

            // 获取缓冲列表
            const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator_->buffers(stream_);
            if (buffers.empty()) {
                throw std::runtime_error("没有可用的缓冲");
            }

            // 设置请求缓冲
            request_->addBuffer(stream_, buffers[0].get());

            // 启动相机
            libcamera::ControlList controls = camera_->controls();
            if (camera_->start(&controls)) {
                throw std::runtime_error("相机启动失败");
            }

            // 连接请求完成信号
            request_connection_ = camera_->requestCompleted.connect(this, &CameraController::processRequest);

            // 发送请求
            if (camera_->queueRequest(request_)) {
                throw std::runtime_error("请求发送失败");
            }

            is_previewing_ = true;
            std::cout << "预览已启动" << std::endl;

        } catch (const std::exception& e) {
            StopPreview();
            throw e;
        }
    }
}

void CameraController::StopPreview() {
    if (is_previewing_) {
        try {
            if (camera_) {
                camera_->stop();
            }
            // 断开请求完成信号
            request_connection_.disconnect();
            if (request_) {
                request_ = nullptr;
            }
            is_previewing_ = false;
            std::cout << "预览已停止" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "停止预览时发生错误: " << e.what() << std::endl;
        }
    }
}

void CameraController::processRequest(libcamera::Request* request) {
    if (request->status() == libcamera::Request::RequestComplete) {
        // 获取缓冲
        libcamera::FrameBuffer* buffer = request->findBuffer(stream_);
        if (buffer) {
            // 更新当前缓冲
            current_buffer_ = buffer;

            // 复制数据到预览缓冲区（简化版，实际需要根据像素格式进行转换）
            if (buffer->planes().size() > 0) {
                const libcamera::FrameBuffer::Plane& plane = buffer->planes()[0];
                std::memcpy(preview_buffer_, plane.data(), 
                           std::min(static_cast<size_t>(plane.length), static_cast<size_t>(params_.width * params_.height * 3)));
            }
        }

        // 重新创建并发送请求以持续获取帧
        if (is_previewing_) {
            std::unique_ptr<libcamera::Request> new_request = camera_->createRequest();
            if (new_request) {
                // 获取缓冲列表
                const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator_->buffers(stream_);
                if (!buffers.empty()) {
                    // 设置请求缓冲
                    new_request->addBuffer(stream_, buffers[0].get());
                    // 发送请求
                    camera_->queueRequest(std::move(new_request));
                }
            }
        }
    }
}

const uint8_t* CameraController::GetPreviewFrame() {
    if (!is_initialized_ || !is_previewing_) {
        return nullptr;
    }

    return preview_buffer_;
}

void CameraController::StartRecording(const std::string& filename) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    if (!is_recording_) {
        try {
            // 在这里实现录制功能
            // libcamera 本身不处理录制，需要使用其他库或自行实现
            // 这里只是简单的演示
            is_recording_ = true;
            std::cout << "开始录制: " << filename << std::endl;
        } catch (const std::exception& e) {
            StopRecording();
            throw e;
        }
    }
}

void CameraController::StopRecording() {
    if (is_recording_) {
        try {
            is_recording_ = false;
            std::cout << "录制已停止" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "停止录制时发生错误: " << e.what() << std::endl;
        }
    }
}

void CameraController::AdjustExposure(float delta) {
    SetExposureCompensation(params_.exposure_compensation + delta);
    std::cout << "曝光补偿: " << params_.exposure_compensation << std::endl;
}

void CameraController::AdjustISO(int delta) {
    SetISO(params_.iso + delta);
    std::cout << "ISO: " << params_.iso << std::endl;
}

void CameraController::CycleWhiteBalance() {
    params_.white_balance += 500;
    if (params_.white_balance > 6500) {
        params_.white_balance = 3000;
    }
    SetWhiteBalance(params_.white_balance);
    std::cout << "白平衡: " << params_.white_balance << "K" << std::endl;
}

void CameraController::SetResolution(int width, int height) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    // 重新初始化相机以更改分辨率
    StopRecording();
    StopPreview();
    
    params_.width = width;
    params_.height = height;
    
    // 重新初始化
    Initialize(params_);
    
    // 重新开始预览
    StartPreview();
}

void CameraController::SetFPS(int fps) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.fps = fps;
    std::cout << "FPS设置为: " << fps << std::endl;
    
    // libcamera 的帧率控制比较复杂，通常在流配置中设置
    // 这里只是更新参数，实际效果可能需要重新配置相机
}

void CameraController::SetBitDepth(int bit_depth) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.bit_depth = bit_depth;
    std::cout << "位深度设置为: " << bit_depth << "位" << std::endl;
    
    // 位深度通常在流配置中设置，这里只是更新参数
}

void CameraController::SetExposureCompensation(float value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.exposure_compensation = value;
    setupControls();
}

void CameraController::SetISO(int value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    // 限制ISO范围
    if (value < 100) value = 100;
    if (value > 3200) value = 3200;
    
    params_.iso = value;
    setupControls();
}

void CameraController::SetWhiteBalance(int value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.white_balance = value;
    setupControls();
}

} // namespace cinepi