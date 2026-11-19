# Motion Capture · ESP32-S3 y Blender

Captura de inclinaciones de un brazo mediante dos sensores MPU6050, una ESP32-S3 y un personaje articulado en Blender. Proyecto académico de Matemáticas Aplicadas a la Ingeniería, Ciudad Victoria, Tamaulipas.

El prototipo conecta adquisición de señales, trigonometría, integración numérica, filtrado y animación 3D. El firmware estima dos inclinaciones del brazo superior y una del antebrazo, y las transmite por Wi-Fi mediante UDP.

## Demostración

[Ver el video del proyecto](https://youtube.com/shorts/cUrcQaBQ2yg). Enlace aportado en el informe original; no verificado durante esta preparación.

## Estado de esta entrega

Esta edición organiza y documenta los archivos recibidos; no modifica el firmware, la escena ni el informe original. **El código y el informe describen versiones diferentes.** Para conectar esta entrega, utiliza los valores del archivo `.ino` y consulta [las diferencias identificadas](docs/ESTADO_Y_LIMITACIONES.md).

El `.blend` contiene scripts de UDP, demostración y USB. El firmware incluido transmite ángulos por **UDP**, no como líneas CSV por USB. El receptor USB requiere otro firmware compatible.

## Características del firmware incluido

- Dos módulos en un bus I2C, direcciones `0x68` y `0x69`.
- Estimación de roll/pitch con filtro complementario, peso nominal 0.98.
- Período objetivo de adquisición de 10 ms; no es una tasa medida.
- Calibración de 300 muestras por intento y hasta tres intentos.
- Comandos serie para cero, recalibración y diagnóstico.
- Reintento de inicialización de sensores con fallos.
- Conexión a un router o punto de acceso de respaldo `ESP32-Brazo`.

## Materiales

| Elemento | Cantidad | Uso |
| --- | --- | --- |
| ESP32-S3 | 1 | Lectura de sensores y envío UDP |
| Módulos MPU6050 | 2 | Brazo superior y antebrazo |
| Cable USB de datos | 1 | Programación y alimentación |
| Cables y soportes | Según montaje | Bus y fijación de los módulos |
| Computadora con Blender | 1 | Representación gráfica |
| Router Wi-Fi | Opcional | Alternativa al AP de respaldo |

Ver [conexiones y montaje](docs/CONEXIONES.md).

## Inicio rápido

1. Conecta los sensores según la guía: el firmware usa **SDA GPIO 8 y SCL GPIO 9**.
2. Abre `Brazo_esp32s3_v3/Brazo_esp32s3_v3.ino` con Arduino IDE y selecciona tu placa ESP32-S3 y su puerto.
3. Configura `WIFI_SSID`, `WIFI_PASS` e `IP_DESTINO` para tu red, o utiliza el AP de respaldo.
4. Carga el programa y mantén los sensores quietos durante la calibración. Abre el monitor serie a `115200` baudios.
5. Abre `BrazoNuevo.blend` con una versión compatible, entra al editor de texto y revisa `1_Archivo Principal` antes de ejecutarlo. Es el script principal que contiene recepción UDP en el puerto `5005`.
6. Ajusta sus rutas locales de imagen/video si deseas utilizarlos. No ejecutes simultáneamente el receptor USB ni la animación de prueba.
7. Envía `z` desde el monitor serie con el brazo en la postura de referencia.

Consulta [instalación y solución de problemas](docs/INSTALACION.md) antes de la primera ejecución.

## Archivos

| Ruta | Contenido |
| --- | --- |
| `Brazo_esp32s3_v3/` | Firmware Arduino original |
| `BrazoNuevo.blend` | Escena original y scripts internos |
| `Documentacion_Brazo_Blender.docx` | Informe académico original |
| `docs/` | Guías, protocolo, limitaciones y publicación |
| `CONTRIBUTING.md` | Cómo proponer y documentar cambios |
| `CHANGELOG.md` | Cambios de esta preparación |
| `.github/` | Plantillas de incidencias y propuestas de cambio |
| `.gitignore` | Exclusión de respaldos y archivos temporales |

## Protocolo y alcance

Cada datagrama contiene `brazo_roll,brazo_pitch,antebrazo_roll` en grados. Ejemplo: `30.00,10.00,90.00`. La diferencia simple del antebrazo y el brazo da 60° en ese ejemplo. Ver [protocolo](docs/PROTOCOLO.md).

La resta de inclinaciones aproxima la flexión del codo cuando los ejes de los sensores están alineados. No reconstruye orientación 3D completa ni mide posición absoluta. No se incluyen mediciones nuevas de precisión, latencia o autonomía.

## Equipo

Autores indicados en el informe:

- Zamarripa Villanueva Danna Itzel.
- Avalos Martínez Erick Guadalupe.
- Cepeda Hernandez Marco Antonio.
- Herrera Martinez Saul Emmanuel.
- Carrizales Castro Jose Alberto.

Profesor: Ruiz Méndez Manuel.

## Próximas mejoras

Ver [hoja de ruta](docs/ROADMAP.md): exportar los scripts de Blender como `.py`, unificar documentación y firmware, añadir evidencias reproducibles y registrar sesiones.

## Publicación y licencia

La [guía para GitHub](docs/PUBLICAR_EN_GITHUB.md) explica cómo publicar el contenido de esta carpeta. La elección de licencia queda pendiente del equipo; esta preparación no añade una licencia ni cambia los créditos de los archivos recibidos.
