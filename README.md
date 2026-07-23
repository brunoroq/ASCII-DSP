# ASCII DSP

[English](#english) · [Español](#español)

---

## English

ASCII DSP is an experimental, real-time audio visualizer built in C++ with
[JUCE](https://juce.com/). It can run as a VST3 plug-in or as a standalone
application and is intended to be placed on an individual track, an instrument
or vocal bus, or the master channel of a DAW.

The project converts an embedded image into ASCII art and makes its appearance
react to the incoming audio signal. It is currently a visualizer only: it
analyzes the signal but does not intentionally modify the audio that passes
through it.

> **Project status:** early visual prototype. The current implementation uses a
> block-level RMS measurement as its only audio feature. More expressive audio
> analysis and animation systems are planned.

### Current behavior

At startup, the plug-in loads `Assets/character.png` from its compiled binary
data and converts it into a fixed 90 × 55 character grid. Each cell samples the
source image, estimates its brightness, applies stable ordered dithering, and
maps the resulting value to a character.

While audio is playing:

1. The audio processor receives each input buffer from the host.
2. It calculates the RMS energy across all channels and samples in the block.
3. The original audio buffer is left unchanged.
4. The editor reads the latest RMS value at 30 Hz.
5. The RMS value controls the brightness and character density of the ASCII
   image.

```text
Incoming audio
      │
      ├── unchanged audio ──────────────────────────────> Output
      │
      └── block RMS ──> visual level ──> ASCII renderer
                                           │
Embedded image ──> brightness grid ─────────┘
```

### ASCII rendering

The current renderer:

- Uses a 90-column by 55-row ASCII grid.
- Preserves the source image aspect ratio and centers it with letterboxing.
- Calculates brightness from the average red, green, and blue pixel values.
- Applies an 8 × 8 Bayer ordered-dither matrix to reduce hard brightness bands.
- Precalculates normal, inverted, and heavy character variants.
- Switches to a heavier character set at higher audio levels.
- Uses a monospaced font and scales the text block to the plug-in window.
- Maps the measured audio level to grayscale brightness.

The inverted variant is already generated but is not yet selected by the
current animation logic.

### Project architecture

#### `PluginProcessor`

`ASCIIDSPAudioProcessor` owns the audio-facing part of the plug-in. It declares
stereo input and output buses and receives audio through `processBlock()`.
Currently, `processBlock()` only measures the signal RMS; it does not apply gain,
filtering, distortion, or any other effect.

#### `PluginEditor`

`ASCIIDSPAudioProcessorEditor` owns the graphical interface and ASCII renderer.
It embeds the source image, generates the ASCII variants once when the editor is
created, and repaints the visualization using a 30 Hz JUCE timer.

Audio processing and rendering are deliberately separated: the audio callback
collects a small measurement, while all drawing happens on the UI thread.

#### Embedded assets

`Assets/character.png` is embedded at build time with `juce_add_binary_data`.
The resulting generated `BinaryData` symbols allow the plug-in to load the
image without depending on an external file at runtime.

### Supported targets

The current CMake configuration produces:

- **VST3:** for loading the visualizer as an effect in a compatible DAW.
- **Standalone:** for running and testing it as a desktop application.

Other formats supported by JUCE, such as AU, AUv3, AAX, and LV2, are not
currently enabled by this project.

### Requirements

- CMake 3.22 or newer.
- A C++17-compatible compiler.
- A compatible JUCE source tree.
- A native build toolchain:
  - Visual Studio with Desktop Development with C++ on Windows.
  - Xcode on macOS.
  - GCC or Clang and the required JUCE system libraries on Linux.
- A VST3-compatible host or DAW to test the plug-in format.

Recent JUCE versions include the files needed to build VST3 plug-ins, so a
separate VST3 SDK is not normally required for this configuration.

### JUCE dependency

The root `CMakeLists.txt` uses:

```cmake
add_subdirectory(JUCE)
```

This means that a compatible JUCE checkout must exist at `ASCII-DSP/JUCE`
before CMake configures the project.

The repository currently records `JUCE` as a Git link but does not include a
`.gitmodules` definition. On a machine where JUCE is not already available,
place or clone JUCE into that directory first:

```bash
git clone https://github.com/juce-framework/JUCE.git JUCE
```

For reproducible builds, this dependency should eventually be converted into a
proper pinned Git submodule or declared with CMake `FetchContent`.

### Building

From the project root:

```bash
cmake -S . -B build
cmake --build build --config Debug
```

For an optimized build:

```bash
cmake --build build --config Release
```

On Windows, an explicit Visual Studio configuration can be generated with:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

JUCE normally places the generated products under a directory similar to:

```text
build/
└── ASCII_DSP_artefacts/
    ├── Debug/
    │   ├── Standalone/
    │   └── VST3/
    └── Release/
        ├── Standalone/
        └── VST3/
```

The exact output layout may vary depending on the platform and CMake generator.

### Testing the visualizer

#### Standalone

The standalone target provides the fastest development loop for UI and renderer
work:

```text
Edit code → build Debug → run ASCII DSP
```

Select a valid audio input device in the standalone application's audio
settings. To feed it desktop or DAW audio, the operating system may require a
loopback device or virtual audio cable.

#### VST3

For realistic testing, load the VST3 build as an audio effect in a DAW:

1. Copy or install the generated `.vst3` bundle into a folder scanned by the
   host.
2. Rescan plug-ins if required.
3. Insert ASCII DSP on a track, bus, or master channel.
4. Open the plug-in editor and play audio through that channel.

On Windows, the VST3 user-level development location is:

```text
%LOCALAPPDATA%\Programs\Common\VST3
```

The system-wide location is:

```text
C:\Program Files\Common Files\VST3
```

JUCE's `AudioPluginHost` can also be used as a lightweight environment for
loading and debugging the VST3 without starting a complete DAW.

### Real-time development considerations

Code inside `processBlock()` runs on the real-time audio thread. It should avoid
file access, UI work, memory allocation, locks, and other operations with
unpredictable execution time. Analysis results should be transferred to the UI
using lock-free or atomic state.

The current `rmsValue` member is a plain `float` shared by the audio and UI
threads. Replacing it with `std::atomic<float>` is planned so that this transfer
is explicitly thread-safe.

### Current limitations

- RMS is the only audio feature.
- The RMS measurement has no attack/release envelope or adaptive normalization.
- Audio intensity mainly changes brightness and switches the character set.
- There is no multiband or frequency-domain analysis.
- There is no transient, onset, beat, or drop detection.
- The image is precalculated, so there are no spatial deformations yet.
- The inverted ASCII variant is not used.
- Plug-in state serialization and program management are empty.
- The editor has a fixed initial size and no interactive controls.
- Bus-layout validation currently accepts every layout even though the processor
  is initialized as stereo.

### Planned direction

The visual system will be developed before the final DSP analysis is selected.
A synthetic visual-control layer can provide energy, bass, mids, highs,
transient, and drop values without requiring a real analyzer. This makes it
possible to design and evaluate the animation language independently.

Potential visual features include:

- Brightness, contrast, and character-density modulation.
- Image breathing, scaling, and bass-driven bouncing.
- Row and column waves.
- Region-based jitter and displacement.
- Character-set morphing.
- Inversion flashes and transient-triggered glitches.
- Block fragmentation and image reconstruction.
- ASCII persistence, trails, and echoes.

Once those visual controls are defined, the DSP stage can generate them using:

- Smoothed broadband energy.
- Low-, mid-, and high-frequency envelopes.
- Adaptive signal normalization.
- Spectral flux and transient detection.
- Fast-versus-slow energy comparisons for build-up and drop intensity.

### Repository layout

```text
ASCII-DSP/
├── Assets/
│   └── character.png
├── JUCE/
├── Source/
│   ├── PluginEditor.cpp
│   ├── PluginEditor.h
│   ├── PluginProcessor.cpp
│   └── PluginProcessor.h
├── CMakeLists.txt
└── README.md
```

---

## Español

ASCII DSP es un visualizador de audio experimental y en tiempo real desarrollado
en C++ con [JUCE](https://juce.com/). Puede ejecutarse como plugin VST3 o como
aplicación independiente y está pensado para insertarse en una pista, un bus de
instrumentos o voces, o el canal master de un DAW.

El proyecto convierte una imagen incrustada en arte ASCII y hace que su
apariencia responda a la señal de audio de entrada. Actualmente funciona
únicamente como visualizador: analiza la señal, pero no modifica
intencionalmente el audio que lo atraviesa.

> **Estado del proyecto:** prototipo visual inicial. La implementación actual
> utiliza una medición RMS por bloque como única característica de audio. Se
> planea incorporar un análisis de señal y un sistema de animaciones más
> expresivos.

### Comportamiento actual

Al iniciarse, el plugin carga `Assets/character.png` desde los datos compilados
dentro del binario y convierte la imagen en una cuadrícula fija de 90 × 55
caracteres. Cada celda muestrea la imagen original, estima su brillo, aplica
dithering ordenado estable y transforma el resultado en un carácter.

Mientras se reproduce audio:

1. El procesador recibe cada buffer de entrada desde el host.
2. Calcula la energía RMS de todos los canales y muestras del bloque.
3. El buffer de audio original permanece intacto.
4. El editor consulta el último valor RMS a 30 Hz.
5. El RMS controla el brillo y la densidad de caracteres de la imagen ASCII.

```text
Audio de entrada
      │
      ├── audio sin modificar ──────────────────────────> Salida
      │
      └── RMS del bloque ─> nivel visual ─> renderer ASCII
                                               │
Imagen incrustada ─> cuadrícula de brillo ──────┘
```

### Renderizado ASCII

El renderer actual:

- Utiliza una cuadrícula ASCII de 90 columnas por 55 filas.
- Conserva la relación de aspecto de la imagen y la centra con letterboxing.
- Calcula el brillo a partir del promedio de los canales rojo, verde y azul.
- Aplica una matriz Bayer de dithering ordenado de 8 × 8 para reducir bandas de
  brillo demasiado marcadas.
- Precalcula variantes de caracteres normal, invertida y pesada.
- Cambia a un conjunto de caracteres más pesado cuando aumenta el audio.
- Utiliza una fuente monoespaciada y ajusta el bloque de texto a la ventana.
- Transforma el nivel de audio medido en brillo de escala de grises.

La variante invertida ya se genera, pero todavía no forma parte de la lógica de
animación.

### Arquitectura del proyecto

#### `PluginProcessor`

`ASCIIDSPAudioProcessor` contiene la parte del plugin relacionada con el audio.
Declara buses de entrada y salida estéreo y recibe el audio mediante
`processBlock()`. Actualmente, `processBlock()` solo mide el RMS de la señal; no
aplica ganancia, filtros, distorsión ni ningún otro efecto.

#### `PluginEditor`

`ASCIIDSPAudioProcessorEditor` contiene la interfaz gráfica y el renderer ASCII.
Carga la imagen incrustada, genera las variantes ASCII una sola vez al crear el
editor y repinta la visualización mediante un temporizador JUCE de 30 Hz.

El procesamiento y el renderizado están separados deliberadamente: el callback
de audio recoge una medición pequeña, mientras que todo el dibujo ocurre en el
hilo de interfaz.

#### Recursos incrustados

`Assets/character.png` se incorpora al binario durante la compilación mediante
`juce_add_binary_data`. Los símbolos `BinaryData` generados permiten que el
plugin cargue la imagen sin depender de un archivo externo durante su ejecución.

### Formatos generados

La configuración actual de CMake produce:

- **VST3:** permite cargar el visualizador como efecto en un DAW compatible.
- **Standalone:** permite ejecutarlo y probarlo como aplicación de escritorio.

Otros formatos compatibles con JUCE, como AU, AUv3, AAX y LV2, no están
habilitados actualmente.

### Requisitos

- CMake 3.22 o superior.
- Un compilador compatible con C++17.
- Una copia compatible del código fuente de JUCE.
- Una cadena de herramientas nativa:
  - Visual Studio con Desktop Development with C++ en Windows.
  - Xcode en macOS.
  - GCC o Clang y las bibliotecas de sistema requeridas por JUCE en Linux.
- Un host o DAW compatible con VST3 para probar el formato plugin.

Las versiones recientes de JUCE incluyen los archivos necesarios para compilar
plugins VST3, por lo que normalmente no se requiere instalar el SDK de VST3 por
separado para esta configuración.

### Dependencia de JUCE

El `CMakeLists.txt` principal utiliza:

```cmake
add_subdirectory(JUCE)
```

Esto significa que debe existir una copia compatible de JUCE en
`ASCII-DSP/JUCE` antes de configurar el proyecto con CMake.

Actualmente el repositorio registra `JUCE` como un enlace de Git, pero no incluye
una definición `.gitmodules`. En un equipo donde JUCE no esté disponible, debe
colocarse o clonarse primero dentro de ese directorio:

```bash
git clone https://github.com/juce-framework/JUCE.git JUCE
```

Para conseguir compilaciones reproducibles, esta dependencia debería
convertirse posteriormente en un submódulo correctamente fijado a una versión o
declararse mediante `FetchContent` de CMake.

### Compilación

Desde la raíz del proyecto:

```bash
cmake -S . -B build
cmake --build build --config Debug
```

Para producir una versión optimizada:

```bash
cmake --build build --config Release
```

En Windows puede generarse explícitamente una configuración de Visual Studio:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

Normalmente JUCE deja los productos generados en una estructura similar a:

```text
build/
└── ASCII_DSP_artefacts/
    ├── Debug/
    │   ├── Standalone/
    │   └── VST3/
    └── Release/
        ├── Standalone/
        └── VST3/
```

La ubicación exacta puede variar según la plataforma y el generador de CMake.

### Pruebas del visualizador

#### Standalone

El target independiente proporciona el ciclo de desarrollo más rápido para
trabajar en la interfaz y el renderer:

```text
Editar código → compilar Debug → ejecutar ASCII DSP
```

Debe seleccionarse un dispositivo de entrada válido desde la configuración de
audio de la aplicación. Para enviarle audio del escritorio o de un DAW, el
sistema operativo puede requerir loopback o un cable de audio virtual.

#### VST3

Para una prueba realista, debe cargarse el VST3 como efecto dentro de un DAW:

1. Copiar o instalar el paquete `.vst3` generado en una carpeta escaneada por el
   host.
2. Volver a escanear los plugins si fuera necesario.
3. Insertar ASCII DSP en una pista, bus o canal master.
4. Abrir el editor del plugin y reproducir audio a través de ese canal.

En Windows, la ubicación VST3 de usuario recomendada para desarrollo es:

```text
%LOCALAPPDATA%\Programs\Common\VST3
```

La ubicación global es:

```text
C:\Program Files\Common Files\VST3
```

También puede utilizarse `AudioPluginHost` de JUCE como entorno liviano para
cargar y depurar el VST3 sin iniciar un DAW completo.

### Consideraciones de tiempo real

El código de `processBlock()` se ejecuta en el hilo de audio de tiempo real. Debe
evitar acceso a archivos, trabajo de interfaz, asignaciones de memoria, locks y
otras operaciones con tiempos de ejecución impredecibles. Los resultados del
análisis deben transferirse a la interfaz mediante estado atómico o lock-free.

El miembro `rmsValue` actual es un `float` normal compartido entre los hilos de
audio e interfaz. Está previsto reemplazarlo por `std::atomic<float>` para que
esta transferencia sea explícitamente segura entre hilos.

### Limitaciones actuales

- RMS es la única característica extraída del audio.
- La medición RMS no tiene envolvente attack/release ni normalización adaptativa.
- La intensidad modifica principalmente el brillo y el conjunto de caracteres.
- No existe análisis multibanda ni en el dominio de la frecuencia.
- No existe detección de transientes, onsets, beats o drops.
- La imagen se precalcula, por lo que todavía no hay deformaciones espaciales.
- La variante ASCII invertida no se utiliza.
- La serialización de estado y la gestión de programas están vacías.
- El editor tiene un tamaño inicial fijo y no ofrece controles interactivos.
- La validación de buses acepta cualquier layout aunque el procesador se
  inicializa como estéreo.

### Dirección futura

El sistema visual se desarrollará antes de elegir el análisis DSP definitivo.
Una capa sintética de controles visuales podrá proporcionar valores de energía,
graves, medios, agudos, transientes y drops sin requerir un analizador real.
Esto permitirá diseñar y evaluar el lenguaje de animación de forma independiente.

Las posibles características visuales incluyen:

- Modulación de brillo, contraste y densidad de caracteres.
- Respiración, escalado y rebote de la imagen controlado por graves.
- Ondas en filas y columnas.
- Jitter y desplazamiento por regiones.
- Transición entre conjuntos de caracteres.
- Flashes invertidos y glitches activados por transientes.
- Fragmentación en bloques y reconstrucción de la imagen.
- Persistencia, estelas y ecos ASCII.

Una vez definidos estos controles visuales, la etapa DSP podrá generarlos
mediante:

- Energía general suavizada.
- Envolventes de frecuencias graves, medias y agudas.
- Normalización adaptativa de la señal.
- Flujo espectral y detección de transientes.
- Comparaciones entre energía rápida y lenta para estimar build-ups y drops.

### Estructura del repositorio

```text
ASCII-DSP/
├── Assets/
│   └── character.png
├── JUCE/
├── Source/
│   ├── PluginEditor.cpp
│   ├── PluginEditor.h
│   ├── PluginProcessor.cpp
│   └── PluginProcessor.h
├── CMakeLists.txt
└── README.md
```
