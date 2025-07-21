# Laboratorio 2: Medición y Optimización de Drivers Embebidos

## Introducción

En esta sesión de laboratorio, aplicaremos de forma práctica los conceptos de **funcionamiento de drivers y la optimización de tiempo de ejecución y consumo de energía** que hemos estudiado. El objetivo es aprender a medir el rendimiento de nuestro código y a utilizar las herramientas del compilador y los modos de bajo consumo del microcontrolador para hacerlo más eficiente.

Utilizaremos un analizador lógico para medir tiempos con precisión de microsegundos, un multímetro para analizar el consumo de corriente, y un script de Python para medir la latencia de comunicación entre diferentes protocolos. Estas mediciones son fundamentales en el desarrollo de sistemas embebidos robustos y eficientes.

---

## Objetivos

### General
* Entender cómo funcionan, se miden y se optimizan los drivers embebidos en un microcontrolador.

### Específicos
* Medir el tiempo de transmisión de datos a través de un periférico I2C usando el código existente del proyecto.
* Analizar cómo los diferentes niveles de optimización del compilador afectan el tiempo de respuesta a un evento externo.
* Comparar la latencia de comunicación entre USB (ST-Link) y WiFi (ESP01) usando mediciones automáticas con Python.
* Medir y comparar el consumo de corriente del microcontrolador en diferentes modos de operación.

---

## Prerrequisitos

### Hardware
* NUCLEO-L476RG
* Display OLED SSD1306 (ya configurado en el proyecto)
* Teclado matricial 4x4 (ya configurado en el proyecto)
* Módulo ESP01 con firmware esp-link (opcional)
* Analizador Lógico (Logic Analyzer)
* Multímetro

### Software
* Proyecto Room Control Final 2025_1 (código base existente)
* Python 3.x con librerías `pyserial` y `socket`
* Software de análisis lógico

---

## Parte 1: Medición de Tiempo de Transmisión I2C

En esta primera parte, mediremos cuánto tiempo toma enviar un *frame* completo (actualizar toda la pantalla) al display OLED usando el código ya existente del proyecto.

1.  **Preparación del Proyecto:**
    * Utiliza el proyecto Room Control Final 2025_1 que ya tiene configurado el driver SSD1306.
    * **No necesitas crear código nuevo** - el demo del display ya está implementado y funcional.

2.  **Conexiones:**
    * El display OLED ya está conectado a I2C1 (`PB8` para SCL y `PB9` para SDA) según la configuración del proyecto.
    * Conecta las puntas de prueba de dos canales del analizador lógico a los mismos pines (`PB8` y `PB9`).

3.  **Código:**
    * Compila y carga el proyecto actual. El sistema ya incluye un bucle que actualiza la pantalla periódicamente.

4.  **Medición:**
    * Configura tu software de análisis lógico para decodificar el protocolo I2C en los canales conectados.
    * Inicia una captura y observa la transmisión de datos que ocurre cuando se actualiza la pantalla.
    * Usa los cursores o marcadores de tiempo del analizador para medir la duración total desde el inicio hasta el final de la transmisión del frame.
    * **Anota este valor y toma la imagen.** Te servirá como referencia para el reporte de resultados.

---

## Parte 2: Medición de Latencia de Comunicación USB vs WiFi

En esta parte mediremos y compararemos la latencia de comunicación entre dos protocolos: USB (ST-Link) y WiFi (ESP01). Usaremos el script Python para mediciones automáticas y precisas.

### 2.1 Preparación del Sistema

1.  **Configuración USB (ST-Link):**
    * El código en `main.c` ya maneja UART2 (ST-Link).

2.  **Configuración ESP01:**
    * Si tienes ESP01 configurado, debe estar conectado a USART3 (opcionalmente al mismo Tx de USART2 para el lab).
    * El módulo debe tener firmware `esp-link` para actuar como puente UART-WiFi.

3.  **Preparación del Script:**
    * El archivo `timing_comparison.py` ya está listo.
    * Ajusta las variables `USB_PORT` y `ESP_IP` según tu configuración.

### 2.2 Procedimiento de Medición

1.  **Ejecutar Medición Automática:**
    ```bash
    # Instalar dependencias si es necesario
    pip install pyserial
    
    # Ejecutar script de medición
    python timing_comparison.py
    ```

2.  **El script realizará:**
    * Monitoreo continuo del puerto USB (ST-Link)
    * Monitoreo continuo del puerto TCP/WiFi (ESP01)
    * Log de paquetes recibidos con timestamps precisos
    * **Ustedes deben analizar los logs y calcular las diferencias manualmente**

### 2.3 Optimización del Compilador

Para analizar el impacto de la optimización en la latencia de procesamiento:

1.  **Cambiar Nivel de Optimización:**
    * En las propiedades del proyecto, busca configuración del compilador
    * Cambia entre `-O0`, `-Og`, `-O3`, `-Os`

2.  **Repetir Mediciones:**
    * Compila con cada nivel de optimización
    * Ejecuta el script Python nuevamente
    * Compara los resultados

### Tabla de Resultados

| Protocolo | Latencia Promedio (ms) | Desviación Estándar (ms) | Min (ms) | Max (ms) | Observaciones |
| :--- | :--- | :--- | :--- | :--- | :--- |
| USB (ST-Link) | | | | | |
| WiFi (ESP01) | | | | | |

| Nivel de Optimización | USB Latencia (ms) | WiFi Latencia (ms) | Diferencia (%) |
| :--- | :--- | :--- | :--- |
| `-O0` (Sin optimización) | | | |
| `-Og` (Debug optimizado) | | | |
| `-O3` (Máxima velocidad) | | | |
| `-Os` (Tamaño optimizado) | | | |

---

## Parte 3: Medición de Consumo de Corriente

Finalmente, mediremos el consumo de energía del sistema en diferentes modos de operación. Para ello, debes conectar el multímetro en modo amperímetro (mA) **en serie** con la alimentación de 3.3V de la placa Nucleo.

1.  **Modo RUN:**
    * Usa el código de la **Parte 1**. El microcontrolador está constantemente activo ejecutando el bucle `while(1)`.
    * Mide y anota el consumo de corriente.

2.  **Modo SLEEP:**
    * Modifica el código para que el microcontrolador entre en modo de bajo consumo y solo despierte ante una interrupción (la pulsación de una tecla).

    ```c
    /* main.c en el bucle infinito */
    while (1)
    {
        // Entrar en modo Sleep, se detiene la CPU hasta la próxima interrupción (EXTI)
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

        // El código se reanuda aquí después de que la interrupción del teclado es atendida
    }
    ```
    * Mide y anota el nuevo consumo de corriente. Deberías ver una reducción significativa.

| Modo de Operación | Descripción | Consumo de Corriente medido (mA) |
| :--- | :--- | :--- |
| **Run Mode** | CPU activa a 80 MHz, ejecutando el bucle. | |
| **Sleep Mode** | CPU detenida, esperando una interrupción externa. | |

---

## Reglas del Entregable

1.  El reporte debe presentar de forma breve y clara los **resultados y conclusiones** de cada una de las tres partes. Incluye los valores medidos en las tablas, capturas de pantalla del analizador lógico y una breve explicación de lo que observaste.
2.  El reporte se puede realizar en grupos de máximo 2 personas.
3.  La entrega está condicionada a demostrar un avance significativo durante la sesión de laboratorio.
4.  Puedes usar el formato de archivo que prefieras (PDF, Markdown, etc.).
