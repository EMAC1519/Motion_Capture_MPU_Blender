# Publicar el proyecto en GitHub

Nombre sugerido: `motion-capture-esp32s3-blender`.

Descripción sugerida: `Prototipo académico de captura de movimiento de brazo con ESP32-S3, dos MPU6050 y visualización en Blender mediante Wi-Fi UDP.`

Temas sugeridos: `esp32-s3`, `mpu6050`, `blender`, `motion-capture`, `arduino`, `udp`, `i2c`.

## Antes de publicar

Lee `ESTADO_Y_LIMITACIONES.md`. Conserva los marcadores de ejemplo de Wi-Fi: `.gitignore` no oculta contraseñas escritas dentro de un `.ino` versionado. El `.blend` original incluye rutas personales de recursos; revísalas desde tu Blender compatible. El informe incluye nombres del equipo y profesor.

## Subida con Git

Descomprime el paquete y abre una terminal dentro de `Motion_Capture_repositorio`, donde está `README.md`. Crea en GitHub un repositorio vacío; no generes allí otro README, licencia ni `.gitignore`, porque esta carpeta ya contiene parte de esos archivos.

Ejecuta estos comandos uno por uno. Sustituye `TU_USUARIO` por tu usuario y utiliza el nombre exacto del repositorio que creaste:

```bash
git init
git add .
git status
git commit -m "Agregar proyecto Motion Capture y documentacion"
git branch -M main
git remote add origin https://github.com/TU_USUARIO/motion-capture-esp32s3-blender.git
git push -u origin main
```

Si Git solicita identidad, configura tu nombre y correo para los commits. Autentícate con el mecanismo que ofrezca tu instalación de Git/GitHub. Estos comandos son para una carpeta nueva y un repositorio remoto vacío.

Sube el contenido descomprimido, no solo el ZIP: así GitHub mostrará el README y permitirá consultar los archivos. Verifica después que aparezcan las carpetas y que los enlaces de documentación funcionen.

## Publicaciones posteriores

```bash
git add .
git diff --cached
git commit -m "Describir el cambio realizado"
git push
```

Revisa los cambios preparados antes del commit para evitar subir credenciales o archivos personales.

Fuente oficial consultada para el flujo de Git: [Adding locally hosted code to GitHub](https://docs.github.com/en/migrations/importing-source-code/using-the-command-line-to-import-source-code/adding-locally-hosted-code-to-github). No se creó ni publicó un repositorio desde esta preparación.
