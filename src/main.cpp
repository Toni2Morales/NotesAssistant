
#include <iostream>
#include <string>

#include <sherpa-onnx/c-api/cxx-api.h>

int main() {
    try {
        // Configuracion del detector
        sherpa_onnx::cxx::KeywordSpotterConfig config;

        const std::string modelDir = "models/spanish-kws/";

        // Modelos ONNX
        config.model_config.transducer.encoder =
            modelDir + "encoder-epoch-20-avg-2-chunk-16-left-64.onnx";

        config.model_config.transducer.decoder =
            modelDir + "decoder-epoch-20-avg-2-chunk-16-left-64.onnx";

        config.model_config.transducer.joiner =
            modelDir + "joiner-epoch-20-avg-2-chunk-16-left-64.onnx";

        // Vocabulario
        config.model_config.tokens = modelDir + "tokens.txt";

        // Ejecucion en CPU
        config.model_config.provider = "cpu";
        config.model_config.num_threads = 2;

        // Palabras clave
        config.keywords_file = modelDir + "keywords.txt";

        // Crear el detector mediante la API C++
        auto spotter =
            sherpa_onnx::cxx::KeywordSpotter::Create(config);

        if (spotter.Get() == nullptr) {
            std::cerr << "No se pudo crear el detector." << std::endl;
            return 1;
        }

        std::cout << "----------------------------------\n";
        std::cout << " NotesAssistant\n";
        std::cout << "----------------------------------\n";
        std::cout << "Modelo espanol cargado correctamente.\n";
        std::cout << "Detector preparado.\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
