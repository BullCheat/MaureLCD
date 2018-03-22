#include <Arduino.h>

void setup()
{
	Serial.begin(115200);
}

void loop()
{
	while (Serial.read() != 'T'); // On attend d'avoir T

    int index = Serial.read();
    if (index >= 80) return; // Hors limite
    // move cursor to index
    while(true) {
        char c = Serial.read();
        if (c == -1) continue; // Pas de caract�re lu, on retente
        if (c == 0) break; // Caract�re de stop�: on sort de la fonction
        // print c
    }
}
