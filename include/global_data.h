#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#define NIVEL_AGUA_LIMITE  70  // 70%
#define VOLUME_CHUVA_LIMITE 80  // 80%

typedef struct {
    uint8_t nivel_agua;      // 0-100%
    uint8_t volume_chuva;    // 0-100%
    bool modo_alerta;        // true: modo alerta, false: modo normal
} DadosSensor;

#endif