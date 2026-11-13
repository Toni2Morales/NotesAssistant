
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>

#include <sherpa-onnx/c-api/cxx-api.h>

#include "audio_capture.h"

int main() {
    std::cout << "-----------------------------\n";
    std::cout << "       NotesAssistant\n";
    std::cout << "-----------------------------\n";

    try {
        // 1. Configurar el detector de palabras clave.
        sherpa_onnx::cxx::KeywordSpotterConfig config;
        config.keywords_score = 1.0f;
        config.keywords_threshold = 0.20f;

        const std::string modelDir = "models/spanish-kws/";

        config.model_config.transducer.encoder =
            modelDir + "encoder-epoch-20-avg-2-chunk-16-left-64.onnx";

        config.model_config.transducer.decoder =
            modelDir + "decoder-epoch-20-avg-2-chunk-16-left-64.onnx";

        config.model_config.transducer.joiner =
            modelDir + "joiner-epoch-20-avg-2-chunk-16-left-64.onnx";

        config.model_config.tokens = modelDir + "tokens.txt";
        config.model_config.provider = "cpu";
        config.model_config.num_threads = 2;

        config.keywords_file = modelDir + "keywords.txt";

        auto spotter =
            sherpa_onnx::cxx::KeywordSpotter::Create(config);

        if (spotter.Get() == nullptr) {
            std::cerr << "No se pudo crear el detector.\n";
            return 1;
        }

        std::cout << "Detector de palabras clave preparado.\n";

        // 2. Crear el flujo de audio del detector.
        auto stream = spotter.CreateStream();

        // 3. Iniciar el micrófono.
        AudioCapture audio;

        if (!audio.start()) {
            std::cerr << "No se pudo iniciar el microfono.\n";
            return 1;
        }

        std::cout << "\nEscuchando...\n";
        std::cout << "Di la frase de activacion.\n";
        std::cout << "Pulsa ENTER para detener.\n\n";

        std::atomic<bool> running{ true };

        // 4. Hilo que procesa el audio y busca la palabra clave.
        std::thread detectionThread([&]() {
            std::vector<float> chunk;

            while (running) {
                if (audio.getAudioChunk(chunk)) {
                    // Enviar las muestras al detector.
                    stream.AcceptWaveform(
                        16000,
                        chunk.data(),
                        static_cast<int32_t>(chunk.size())
                    );

                    // Procesar todo el audio que esté listo.
                    while (spotter.IsReady(&stream)) {
                        spotter.Decode(&stream);

                        auto result = spotter.GetResult(&stream);

                        if (!result.keyword.empty()) {
                            std::cout
                                << "\n\n¡PALABRA DE ACTIVACION DETECTADA!\n";

                            std::cout
                                << "Frase: "
                                << result.keyword
                                << "\n\n";

                            spotter.Reset(&stream);
                        }
                    }
                }
                else {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(5)
                    );
                }
            }
            });

        // 5. Esperar a que el usuario pulse ENTER.
        std::cin.get();

        running = false;

        detectionThread.join();

        audio.stop();

        std::cout << "\nNotesAssistant detenido.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
