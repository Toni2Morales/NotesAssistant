
#include <iostream>
#include "audio_capture.h"

int main() {
    std::cout << "-----------------------------\n";
    std::cout << "       NotesAssistant\n";
    std::cout << "-----------------------------\n";

    AudioCapture audio;

    if (!audio.start()) {
        std::cerr << "No se pudo iniciar la captura.\n";
        return 1;
    }

    std::cout << "\nCapturando audio...\n";
    std::cout << "Habla cerca del microfono.\n";
    std::cout << "Pulsa ENTER para detener.\n";

    std::cin.get();

    audio.stop();

    return 0;
}
