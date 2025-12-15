// camera_controller.cpp
// 摄像头控制类实现

#include "camera_controller.h"
#include <iostream>

namespace cinepi {

CameraController::CameraController() 
    : camera_(std::make_unique<CinePI>()), 
      is_initialized_(false), 
      is_previewing_(false), 
      is_recording_(false) {
}

CameraController::~CameraController() {
    StopRecording();
    StopPreview();
    if (is_initialized_) {
        camera_->shutdown();
    }
}

void CameraController::Initialize(const CameraParams& params) {
    if (is_initialized_) {
        return;
    }

    params_ = params;

    // 初始化摄像头
    if (!camera_->initialize()) {
        throw std::runtime_error("摄像头初始化失败");
    }
    is_initialized_ = true;

    // 设置摄像头参数
    SetResolution(params_.width, params_.height);
    SetFPS(params_.fps);
    SetBitDepth(params_.bit_depth);
    SetExposureCompensation(params_.exposure_compensation);
    SetISO(params_.iso);
    SetWhiteBalance(params_.white_balance);
}

void CameraController::StartPreview() {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    if (!is_previewing_) {
        if (camera_->startPreview()) {
            is_previewing_ = true;
            std::cout << "预览已启动" << std::endl;
        } else {
            throw std::runtime_error("预览启动失败");
        }
    }
}

void CameraController::StopPreview() {
    if (is_previewing_) {
        camera_->stopPreview();
        is_previewing_ = false;
        std::cout << "预览已停止" << std::endl;
    }
}

const uint8_t* CameraController::GetPreviewFrame() {
    if (!is_initialized_ || !is_previewing_) {
        return nullptr;
    }

    return camera_->getPreviewFrame();
}

void CameraController::StartRecording(const std::string& filename) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    if (!is_recording_) {
        // 这里可以根据需要实现录制功能
        // 目前假设CinePI类已经有相关方法
        // camera_->startRecording(filename);
        is_recording_ = true;
        std::cout << "开始录制: " << filename << std::endl;
    }
}

void CameraController::StopRecording() {
    if (is_recording_) {
        // 这里可以根据需要实现停止录制功能
        // 目前假设CinePI类已经有相关方法
        // camera_->stopRecording();
        is_recording_ = false;
        std::cout << "录制已停止" << std::endl;
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

    params_.width = width;
    params_.height = height;
    camera_->setPreviewResolution(width, height);
}

void CameraController::SetFPS(int fps) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.fps = fps;
    camera_->setFPS(fps);
}

void CameraController::SetBitDepth(int bit_depth) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.bit_depth = bit_depth;
    // 这里可以根据需要调用CinePI类的相关方法
    // camera_->setBitDepth(bit_depth);
}

void CameraController::SetExposureCompensation(float value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.exposure_compensation = value;
    camera_->adjustExposure(value);
}

void CameraController::SetISO(int value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    // 限制ISO范围
    if (value < 100) value = 100;
    if (value > 3200) value = 3200;
    
    params_.iso = value;
    camera_->adjustISO(value - params_.iso); // 注意：这里需要根据实际API调整
}

void CameraController::SetWhiteBalance(int value) {
    if (!is_initialized_) {
        throw std::runtime_error("摄像头未初始化");
    }

    params_.white_balance = value;
    // 这里可以根据需要调用CinePI类的相关方法
    // camera_->setWhiteBalance(value);
}

} // namespace cinepi