# Estado real de los archivos

Revisión de empaquetado: 18 de noviembre de 2026. Se inspeccionaron el firmware, el texto del informe y cadenas legibles de los bloques internos del `.blend`. No se ejecutó una prueba física ni se compiló el firmware en esta preparación.

## Diferencias entre el informe y esta entrega

| Tema | Informe original | Archivo o evidencia recibida |
| --- | --- | --- |
| Pines I2C | SDA 3, SCL 4 | `.ino`: SDA 8, SCL 9 |
| Calibración | 500 muestras | `.ino`: 300, hasta 3 intentos |
| Identificación | Solo WHO_AM_I 0x68 | También acepta 0x70 a 0x73 con aviso |
| Envío tras fallo | Solo dos lecturas nuevas | Envía mientras ambos sensores sean saludables; puede conservar valores anteriores |
| Fuente de Blender | `codigo/modelo_blender.py` | Scripts internos; ningún `.py` independiente en el ZIP |
| Receptor USB | No es la ruta principal descrita | Hay un receptor serie interno, pero falta firmware CSV serie compatible |
| Pruebas | Nueve escenarios, arnés C++ y compilación | Los archivos de evidencia citados no están en el ZIP |
| Escena | `modelo_demostracion.blend` | `BrazoNuevo.blend` |
| Blender | Pruebas citadas con 4.3.2 | 4.3.2 no pudo abrir el archivo actual; cabecera v0502 |

Las pruebas y cifras del Word no se deben atribuir automáticamente a este `.ino` ni a esta escena. El Word se conserva íntegro como antecedente académico; esta nota aclara la versión disponible.

## Límites observados

El cálculo del codo es una aproximación escalar, no una orientación relativa tridimensional completa. No hay mediciones adjuntas de error angular, latencia, pérdida de paquetes ni autonomía. La recepción UDP y la compatibilidad visual deben comprobarse en la versión adecuada de Blender.

El `.blend` conserva rutas absolutas a recursos del equipo original y contenido de scripts de trabajo. Es conveniente exportar los scripts y sustituir esas rutas antes de una publicación definitiva. No se modificó el binario al no poder abrirlo con la versión disponible.

## Qué se verificó en esta preparación

- Inventario del ZIP y lectura del firmware completo.
- Extracción del texto del informe para comparar parámetros.
- Descompresión de la escena para inspección estática de nombres y fragmentos de scripts.
- Intento de apertura con Blender 4.3.2, fallido por incompatibilidad del formato.
- Conservación byte a byte de firmware, escena e informe en el nuevo paquete.
- Integridad del ZIP final y vínculos relativos Markdown.

No se incluyen certificados de pruebas, resultados fabricados ni badges de compilación exitosa.
