# Protocolo UDP

Transporte implementado: datagramas UDP con texto ASCII, tres números separados por comas y dos decimales. No se añade salto de línea en `enviarUDP()`.

```text
30.00,10.00,90.00
```

| Posición | Campo | Unidad | Origen |
| --- | --- | --- | --- |
| 1 | brazo_roll | Grados | Brazo superior, dirección 0x69 |
| 2 | brazo_pitch | Grados | Brazo superior, dirección 0x69 |
| 3 | antebrazo_roll | Grados | Antebrazo, dirección 0x68 |

Destino predeterminado: `192.168.1.100:5005`. Respaldo AP: `192.168.4.255:5005`. Puerto local de la placa: `5006`.

La flexión del codo se aproxima mediante `antebrazo_roll - brazo_roll`; la conversión gráfica depende de los signos y límites del receptor. Para el ejemplo, la diferencia es 60°. Cerca de ±180° hace falta normalizar la diferencia para evitar saltos; no debe asumirse que todas las variantes internas del receptor lo hacen.

El formato no contiene marca temporal, número de secuencia, identidad de sensor ni bandera de validez. El período objetivo es 10 ms. El firmware puede transmitir el último ángulo conservado tras un fallo aislado de lectura mientras ambos sensores sigan considerados saludables. No se garantiza que cada datagrama contenga dos muestras nuevas.

El puerto USB se utiliza para comandos y mensajes de diagnóstico. No comparte este formato de salida en el firmware recibido.
