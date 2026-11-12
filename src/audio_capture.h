
#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <vector>
#include <cstddef>

#include <miniaudio.h>

class AudioCapture {
public:
    AudioCapture();
    ~AudioCapture();

    bool start();
    void stop();

    bool isRunning() const;

    // Obtiene un fragmento de audio de la cola.
    // Devuelve false si todavía no hay audio disponible.
    bool getAudioChunk(std::vector<float>& chunk);

private:
    ma_device device_;
    std::atomic<bool> running_{ false };

    // Cola de fragmentos de audio.
    std::deque<std::vector<float>> audioQueue_;
    std::mutex queueMutex_;

    // Máximo de fragmentos pendientes.
    static constexpr size_t MAX_QUEUE_CHUNKS = 50;

    static void dataCallback(
        ma_device* device,
        void* output,
        const void* input,
        ma_uint32 frameCount
    );
};
