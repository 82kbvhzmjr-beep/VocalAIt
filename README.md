# Vocal AI Studio

Plugin vocal VST3 experimental, con interfaz monocromática y una cadena DSP compartida basada en JUCE. El proyecto está en fase inicial: incluye controles reales para filtro pasa-altos, shelf de brillo, compresión sencilla, saturación, reverb, mezcla, ganancia de entrada/salida y presets configurables.

## Estado real

**Implementado en esta base:** proyecto CMake/JUCE; interfaz en español; parámetros automatizables; persistencia del estado; diez presets iniciales; comparación A/B (conmutación a señal seca); DSP básico de filtro, dinámica, saturación y reverb; prueba automatizada de contratos de código; workflow CI multiplataforma.

**Todavía no implementado:** análisis vocal inteligente, análisis/importación de referencias de 15 s, separación vocal, afinación real, desplazamiento de tono de una octava, TALK/STT/LLM, biblioteca de aprendizaje, undo/redo y gestión completa de módulos reordenables. Los botones de análisis y TALK son indicativos y no deben interpretarse como funciones de IA terminadas. La interfaz de forma de onda es ilustrativa, no representa entrada de audio medida.

## Requisitos

- CMake 3.22 o posterior.
- Compilador C++17.
- Git y conexión a Internet en la primera configuración, salvo que se coloque JUCE en `./JUCE`.
- Dependencias de compilación de JUCE para el sistema operativo. En Linux, el workflow enumera los paquetes habituales.

JUCE se descarga de su repositorio oficial fijando la etiqueta `8.0.10`. Revisa sus términos/licencia en https://juce.com/legal/ antes de distribuir el plugin.

## Compilar

```bash
git clone <URL-DE-TU-REPOSITORIO>
cd VocalAIStudio
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Para una copia local de JUCE, colócala en `./JUCE`. Para desactivar la descarga automática: `-DVAS_FETCH_JUCE=OFF`.

## Instalar

Busca el bundle `Vocal AI Studio.vst3` dentro del directorio `build` (la ruta exacta depende del sistema y generador). Copia el bundle a la carpeta VST3 del usuario/sistema y vuelve a escanear plugins en tu DAW. En macOS, esta primera versión solo declara VST3; AU no está configurado aún.

## Subir a GitHub

1. Crea un repositorio vacío en GitHub.
2. Desde esta carpeta ejecuta:

```bash
git init
git add .
git commit -m "Base inicial de Vocal AI Studio"
git branch -M main
git remote add origin https://github.com/USUARIO/REPOSITORIO.git
git push -u origin main
```

GitHub Actions compila en Linux, Windows y macOS y ejecuta las pruebas de contrato. La compilación real debe verificarse en esos runners; no se afirma que haya compilado en el entorno de creación.

## Arquitectura y limitaciones DSP

Los presets solo cambian parámetros del mismo procesador; no crean motores independientes. La etapa llamada compresión es un control dinámico sencillo por muestra y no sustituye un compresor profesional con detector/ataque/release. El parámetro de afinación y el de segunda voz se reservan en el estado, pero aún no procesan el audio. No uses esta versión como una solución de afinación o pitch-shifting terminada.

## Licencia

Código de este repositorio: MIT (ver `LICENSE`). JUCE y cualquier dependencia conservan sus propias licencias.

## Compilar automáticamente en GitHub para FL Studio en macOS

El workflow de GitHub Actions compila VST3 y Audio Unit (AU) en el runner macOS. VST3 es el formato recomendado para FL Studio; AU queda disponible para hosts compatibles. El runner `macos-14` genera actualmente una compilación para Apple Silicon (ARM64); no es una compilación universal Intel/Apple Silicon.

1. Crea un repositorio en GitHub y sube esta carpeta (incluido `.github/workflows/build.yml`).
2. En GitHub, abre la pestaña **Actions** y selecciona **Compilar Vocal AI Studio**.
3. Espera a que termine el trabajo **Compilar en macos-14**. Debe aparecer con marca verde; si falla, abre el paso rojo y consulta el registro.
4. En la ejecución terminada, baja el artefacto **VocalAIStudio-macOS-VST3-AU**. Dentro estarán `Vocal-AI-Studio-macOS-VST3.zip` y `Vocal-AI-Studio-macOS-AU.zip`.
5. Descomprime el VST3 y copia `Vocal AI Studio.vst3` a `~/Library/Audio/Plug-Ins/VST3/`. Para AU, copia `Vocal AI Studio.component` a `~/Library/Audio/Plug-Ins/Components/`.
6. Cierra y vuelve a abrir FL Studio. Abre **Options > Manage plugins** (el nombre puede variar según la versión), ejecuta un escaneo y busca **Vocal AI Studio**. En FL Studio, prueba primero el formato VST3.

Si macOS bloquea un plugin descargado, no elimines protecciones del sistema indiscriminadamente. Este artefacto de CI no está firmado ni notarizado; para distribución pública en macOS será necesario configurar firma Developer ID y notarización. La instalación local puede requerir autorización explícita de macOS.

### Subir el proyecto desde Terminal

```bash
git init
git add .
git commit -m "Preparar compilación macOS VST3 y AU"
git branch -M main
git remote add origin https://github.com/USUARIO/REPOSITORIO.git
git push -u origin main
```

Si el repositorio ya existe, no repitas `git init` ni `git remote add`; guarda los cambios y haz `git push`.
