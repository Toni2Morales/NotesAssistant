
#include "audio_capture.h"

#include <iostream>
#include <algorithm>
#include <utility>

AudioCapture::AudioCapture() {
    device_ = {};
}

AudioCapture::~AudioCapture() {
    stop();
}

bool AudioCapture::start() {
    ma_device_config config =
        ma_device_config_init(ma_device_type_capture);

    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.sampleRate = 16000;
    config.dataCallback = dataCallback;
    config.pUserData = this;

    ma_result result = ma_device_init(
        nullptr,
        &config,
        &device_
    );

    if (result != MA_SUCCESS) {
        std::cerr << "Error al inicializar el microfono: "
            << ma_result_description(result) << '\n';
        return false;
    }

    result = ma_device_start(&device_);

    if (result != MA_SUCCESS) {
        std::cerr << "Error al iniciar el microfono: "
            << ma_result_description(result) << '\n';

        ma_device_uninit(&device_);
        return false;
    }

    running_ = true;

    std::cout << "Microfono iniciado correctamente.\n";

    return true;
}

void AudioCapture::stop() {
    if (running_) {
        ma_device_uninit(&device_);
        running_ = false;

        std::cout << "Microfono detenido.\n";
    }
}

bool AudioCapture::isRunning() const {
    return running_;
}

bool AudioCapture::getAudioChunk(std::vector<float>& chunk) {
    std::lock_guard<std::mutex> lock(queueMutex_);

    if (audioQueue_.empty()) {
        return false;
    }

    chunk = std::move(audioQueue_.front());
    audioQueue_.pop_front();

    return true;
}

void AudioCapture::dataCallback(
    ma_device* device,
    void* output,
    const void* input,
    ma_uint32 frameCount
) {
    (void)output;

    auto* self = static_cast<AudioCapture*>(device->pUserData);

    if (input == nullptr || frameCount == 0) {
        return;
    }

    const auto* samples = static_cast<const float*>(input);

    // Copiamos las muestras para conservarlas después
    // de que termine este callback.
    std::vector<float> chunk(samples, samples + frameCount);

    {
        std::lock_guard<std::mutex> lock(self->queueMutex_);

        if (self->audioQueue_.size() >= MAX_QUEUE_CHUNKS) {
            self->audioQueue_.pop_front();
        }

        self->audioQueue_.push_back(std::move(chunk));
    }
}

