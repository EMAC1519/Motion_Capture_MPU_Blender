# Conexiones y montaje

La siguiente tabla corresponde a las constantes del firmware incluido, no a la tabla del informe Word.

| Señal | ESP32-S3 | MPU6050 antebrazo | MPU6050 brazo superior |
| --- | --- | --- | --- |
| SDA | GPIO 8 | SDA | SDA |
| SCL | GPIO 9 | SCL | SCL |
| GND | GND | GND | GND |
| Alimentación | 3.3 V para módulos compatibles | VCC | VCC |
| Dirección AD0 | Según módulo | GND: `0x68` | 3.3 V: `0x69` |
| INT | Sin conexión | No utilizado | No utilizado |

Comprueba las etiquetas y alimentación admitida de tus módulos y la disponibilidad de esos GPIO en tu placa concreta. Mantén masa común y señales I2C a 3.3 V. Realiza el cableado sin alimentación.

Fija un sensor en el brazo superior y otro en el antebrazo; evita que se desplacen respecto al segmento. Alinea sus ejes en la postura inicial. La muñeca no forma parte del modelo de codo: un sensor en la mano puede añadir movimiento de muñeca.

Al iniciar, mantén ambos módulos inmóviles. La calibración del giroscopio estima sesgos; el comando `z` guarda la postura actual como cero. Son operaciones distintas. Prueba primero una articulación a la vez y luego ajusta los signos del script de Blender si el movimiento aparece invertido.

Si tu montaje físico utiliza GPIO 3 y 4 como el Word, cambia `I2C_SDA_PIN` e `I2C_SCL_PIN` para hacerlo coincidir antes de cargar. No cambies el cableado basándote únicamente en el informe.
