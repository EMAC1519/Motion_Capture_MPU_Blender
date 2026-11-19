# Instalación y diagnóstico

## Preparar la placa

Necesitas Arduino IDE con soporte de placas ESP32 de Espressif. Selecciona el modelo ESP32-S3 correspondiente a tu placa y su puerto USB. El programa utiliza Arduino, Wire, WiFi, WiFiUDP y math; accede directamente a los registros de los sensores y no incluye una biblioteca MPU6050 externa.

Abre `Brazo_esp32s3_v3/Brazo_esp32s3_v3.ino`. Mantén el nombre de la carpeta y del sketch iguales. Las versiones exactas de IDE y núcleo utilizadas para este archivo no se verificaron en esta preparación; las versiones del informe pertenecen a otra revisión.

## Red

Para un router, sustituye `TU_RED_WIFI`, `TU_PASSWORD` y `192.168.1.100` por tus datos y la IP local de la computadora. No publiques las credenciales reales al guardar los cambios. `USAR_BROADCAST_LAN` está desactivado.

Sin conexión al router después de 15 segundos, `USAR_AP_RESPALDO` permite crear `ESP32-Brazo`, con clave de ejemplo `12345678`. Conecta la computadora a esa red. El destino pasa a `192.168.4.255:5005`. No se requiere acceso a Internet para esa comunicación local.

La placa inicia UDP local en `5006` y envía a `5005`. Permite recepción de Blender en UDP 5005 en la red utilizada.

## Blender

El archivo original tiene cabecera de formato `v0502`. El Blender 4.3.2 disponible durante la preparación no pudo abrirlo. Utiliza la versión con la que se guardó el archivo, o una versión capaz de leerlo; la compatibilidad de una versión concreta no se comprobó aquí.

En el editor de texto se identificó un bloque llamado `1_Archivo Principal` con socket UDP en puerto 5005. También existen una animación de prueba, utilidades y un receptor USB en `Texto.001`. No ejecutes todos los bloques: pueden competir por el control de las articulaciones.

El script principal incluye `RUTA_IMG` y `RUTA_VIDEO` con rutas absolutas del equipo original. Configúralas para tu computadora si dispones de esos recursos. La imagen faltante contempla un aviso y color plano; el funcionamiento completo sin recursos externos sigue pendiente de comprobar en una versión compatible.

El receptor USB usa `COM5` y PySerial, pero el `.ino` de este paquete no emite los tres ángulos como CSV por serie. Instalar PySerial o cambiar el COM no vuelve compatibles esos dos programas. Para esta entrega sigue el recorrido UDP.

## Comandos serie

| Comando | Acción |
| --- | --- |
| `z` | Establecer cero angular en la postura actual |
| `c` | Recalibrar los giroscopios y establecer cero |
| `d` | Alternar diagnóstico de sensores y Wi-Fi |

Monitor serie: 115200 baudios. Las líneas de diagnóstico son texto, no el protocolo CSV del receptor USB.

## Problemas frecuentes

| Síntoma | Qué revisar |
| --- | --- |
| Missing FQBN | Seleccionar la placa antes de compilar |
| Error I2C | Pines 8/9, masa común, alimentación y AD0 diferente |
| Calibración con aviso | Inmovilizar sensores y repetir con `c` |
| Wi-Fi sin conexión | Datos de red o conexión de la PC al AP de respaldo |
| Blender sin movimiento | Receptor UDP activo, puerto 5005, destino y cortafuegos |
| USB lee pero Blender no cambia | Incompatibilidad de formato serie de esta entrega |
| Movimiento artificial | Detener el bloque de demostración y revisar `DEMO` |
| Texturas/video ausentes | Rutas locales del script y recursos disponibles |
| Blender no abre la escena | Versión compatible con el formato del archivo |

## Comprobación manual pendiente

Registra modelo de placa, versión de Arduino ESP32 y versión de Blender. Guarda la salida de compilación. Comprueba detección de ambos sensores, calibración, envío UDP y movimiento por separado de hombro y codo. Repite desconectando un sensor, desconectando la red y ejecutando otra vez el receptor. Anota resultados reales y conserva evidencias; esta guía no implica que esas pruebas ya hayan pasado.
