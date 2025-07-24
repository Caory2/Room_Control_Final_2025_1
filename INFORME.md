
# INFORME.md DEL PROYECTO: Sistema de Control de Habitación con STM32



## 📘 Proyecto Final - Sistema de Control de Acceso y Climatización Remoto  
**Curso:** Estructuras Computacionales (4100901)  
**Universidad Nacional de Colombia – Sede Manizales**
## 👥 Integrantes

- Angelica Dayanna Benavides Ossa  
- Liseth Carolina Yela Medicis

---
##  Investigación Previa:
- **Super Loop:**
El súper-loop es una arquitectura de programación donde el código se ejecuta cíclicamente y de manera secuencial dentro de un bucle infinito (comúnmente un while(1)). Típicamente, el programa se divide en tres fases principales dentro de cada ciclo: entrada de datos (sensores, comunicaciones), procesamiento de datos, y salida (actuadores, pantallas, transmisiones).:

Ventajas:
- Tiempos deterministas: Debido a su naturaleza secuencial, el tiempo total que toma cada ciclo es predecible y preciso, lo cual es crucial para aplicaciones críticas donde la temporización es vital.

- Mínima sobrecarga: No requiere mecanismos complejos de planificación o comunicación entre tareas, lo que resulta en un uso muy eficiente del CPU (prácticamente todo el tiempo se dedica a las tareas del sistema) y un bajo consumo de memoria FLASH y RAM. Esto lo hace ideal para sistemas con recursos limitados.

- Compartición de recursos simple y segura: La ejecución secuencial garantiza que las tareas accedan a los recursos de hardware (como ADC o SPI) una tras otra, evitando conflictos y corrupción de datos.

Pero también tiene limitaciones que hay que considerar:

- No escala bien: A medida que la complejidad del proyecto crece, añadiendo más tareas o interacciones, el código tiende a volverse "código espagueti" (difícil de mantener y depurar) y el uso excesivo de variables globales compromete la fiabilidad. Un ejemplo claro es la función loop() en Arduino.

- Todas las tareas tienen la misma prioridad: El súper-loop opera con un esquema round-robin, donde todas las tareas se ejecutan en secuencia sin distinción de importancia. Esto significa que una tarea crítica debe esperar su turno para ser atendida, lo que puede causar que eventos importantes se pierdan o se atiendan demasiado tarde.

**¿Porque usar dos UART?**
| UART               | Uso en el proyecto                | Interfaz       | Finalidad                                                               |
| ------------------ | --------------------------------- | -------------- | ----------------------------------------------------------------------- |
| **UART2 (USART2)** | Debug serial (puerto virtual USB) | USB a PC       | Comunicación con el desarrollador (debug, pruebas en consola)           |
| **UART3 (USART3)** | Comunicación con ESP-01 (WiFi)    | UART a ESP8266 | Recibir comandos desde red WiFi (por ejemplo desde una app o webserver) |

## 🧰 Arquitectura de Hardware

### 🔌 Diagrama de Conexiones


![alt text](image.png)

| Componente        | Interfaz STM32       | Periférico Clave | Observaciones                           |
| ----------------- | -------------------- | ---------------- | --------------------------------------- |
| Keypad Matricial  | GPIO (8 pines)       | EXTI             | Usado para ingresar contraseña          |
| OLED SSD1306      | I2C (SDA, SCL)       | I2C              | Interfaz de usuario visual              |
| Sensor de Temp    | Pin analógico (ADC)  | ADC              | Sensor () para lectura de temperatura |
| Ventilador DC     | PWM vía Transistor   | Timer PWM        | Control de climatización                |
| LED Cerradura     | GPIO (salida)        | GPIO             | Indica estado de acceso                 |
| ESP-01 (esp-link) | USART3 (TX/RX)       | UART             | Control remoto por WiFi                 |
| Consola Debug PC  | USART2 (ST-Link USB) | UART             | Depuración local                        |



![alt text](<Untitled diagram _ Mermaid Chart-2025-07-15-170810-1.png>)

## ⚙️ Funciones Implementadas en el Sistema

El firmware desarrollado para el proyecto de control de acceso y climatización implementa una arquitectura modular basada en periféricos de STM32 configurados mediante STM32CubeIDE. A continuación se detallan todas las funciones clave:

---

### 🔐 `void keypad_isr_handler(uint8_t key)`

**Ubicación**: `keypad.c`  
**Descripción general**:  
Gestiona la lógica asociada al ingreso por teclado, actualizando el estado del sistema según la tecla presionada y el contexto actual.

**Detalles**:
- Valida la tecla presionada.
- Si el sistema está en `INPUT_PASSWORD`, la agrega a un buffer temporal.
- Compara el buffer con la contraseña almacenada.
- Si coincide, cambia el estado a `UNLOCKED`.
- Si no, y se completó el intento, cambia a `ACCESS_DENIED`.
- En `LOCKED`, puede iniciar el ingreso de clave al detectar la primera tecla.

**Dependencias**:
- `room_control_t` (estructura global del sistema)
- Máquina de estados (enum de estados del sistema)
- GPIO e interrupciones

---

### 🌡️ `float temperature_sensor_read(void)`

**Ubicación**: `temperature_sensor.c`  
**Descripción general**:  
Convierte el valor digital leído desde el ADC a una temperatura en grados Celsius, considerando un divisor resistivo con una NTC.

**Implementación paso a paso**:
1. Inicia conversión ADC con `HAL_ADC_Start(&hadc1)`.
2. Espera resultado con `HAL_ADC_PollForConversion(...)`.
3. Lee el valor con `HAL_ADC_GetValue(...)`.
Implementación paso a paso:

4. Calcula el voltaje del divisor resistivo con:
V_out = (adc_value / 4095.0) * V_ref

5. Calcula la resistencia de la NTC con:
R_ntc = R_fixed * ((V_ref / V_out) - 1)

6. Aplica la ecuación de Beta para obtener la temperatura en Kelvin:
T = 1 / ((1 / T0) + (1 / beta) * log(R_ntc / R0))
Si necesitas convertir la temperatura a grados Celsius:
7. Convierte a grados Celsius (opcional):
T_C = T - 273.15





---

### 💨 `void fan_set_speed(uint8_t duty_cycle)`

**Ubicación**: `fan.c`  
**Descripción general**:  
Controla la velocidad del ventilador ajustando el ciclo de trabajo de la señal PWM.

**Implementación**:
- PWM configurado con `TIM3`, canal 1.
- Se transforma el duty cycle de 0–100 a un valor proporcional al `htim3.Init.Period`.
- Se usa `__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, value)` para ajustar el ancho de pulso.

**Dependencias**:
- Timer 3 (`htim3`) configurado en modo PWM
- Ventilador conectado a pin de salida PWM

---

### 📟 `void oled_display_status(room_control_t *room)`

**Ubicación**: `display.c`  
**Descripción general**:  
Actualiza el contenido mostrado en la pantalla OLED según el estado actual del sistema.

**Contenido típico mostrado**:
- Temperatura actual (ej. "Temp: 26.5 °C")
- Estado de acceso (LOCKED, UNLOCKED, etc.)
- Velocidad del ventilador
- Mensajes como "ACCESO DENEGADO"

**Implementación**:
- Usa la librería SSD1306 (I2C)
- Formatea cadenas con `sprintf()`
- Usa funciones como `SSD1306_GotoXY`, `SSD1306_Puts`, `SSD1306_UpdateScreen`

**Dependencias**:
- Librería SSD1306 (basada en I2C)
- HAL I2C
- `room_control_t` como fuente de datos

---

### 🔁 `void room_control_update(room_control_t *room)`

**Ubicación**: `room_control.c`  
**Descripción general**:  
Función central del sistema. Llama a todos los módulos para realizar una iteración del control de ambiente y estado.

**Implementación**:
1. Llama a `temperature_sensor_read()` y actualiza `room->temperature`.
2. Si el sistema está en estado `UNLOCKED`:
   - Evalúa la temperatura.
   - Ajusta velocidad del ventilador según umbrales.
   - Llama a `fan_set_speed(...)`.
3. En todos los estados, llama a `oled_display_status(...)`.

**Dependencias**:
- Todas las funciones anteriores
- Acceso a la estructura `room_control_t`

---

### 📡 `void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)`

**Ubicación**: `main.c`  
**Descripción general**:  
Callback de interrupciones externas de pines GPIO, usado para detectar teclas del teclado matricial.

**Implementación**:
- Verifica qué fila fue activada.
- Llama a la rutina de escaneo para determinar la tecla.
- Llama a `keypad_isr_handler(...)`.

**Dependencias**:
- Configuración de EXTI (interrupciones GPIO)
- Driver del teclado
- `keypad_isr_handler()`

---

### 🧪 `void system_init(void)`

**Ubicación**: `main.c` o `init.c`  
**Descripción general**:  
Inicializa todos los módulos de hardware del sistema.

**Pasos comunes**:
- Inicializa la pantalla OLED: `ssd1306_Init()`
- Inicializa variables de estado (`room_control_t`)
- Inicia timers (PWM)
- Habilita ADC, I2C, UART

---

### 🧠 `void main(void)`

**Ubicación**: `main.c`  
**Descripción general**:  
Punto de entrada del programa. Ejecuta la inicialización y entra en el bucle principal de operación.

**Implementación**:
1. `HAL_Init()` y `SystemClock_Config()`
2. Llamadas a `MX_GPIO_Init()`, `MX_ADC1_Init()`, `MX_TIM3_Init()`, etc.
3. Llamada a `system_init()`
4. En `while (1)`:
   - Llama periódicamente a `room_control_update(...)`

**Dependencias**:
- Todas las funciones anteriores
- Configuración generada por STM32CubeIDE

---

## 🧠 Máquina de Estados (`room_control_t.state`)

El sistema implementa una máquina de estados sencilla que define el comportamiento según el contexto de acceso:

| Estado | Descripción |
|--------|-------------|
| `LOCKED` | Sistema bloqueado, espera inicio de ingreso por teclado |
| `INPUT_PASSWORD` | Usuario está ingresando clave |
| `UNLOCKED` | Clave correcta, se activa climatización |
| `ACCESS_DENIED` | Clave incorrecta, muestra mensaje de error |

La transición entre estados se controla principalmente desde `keypad_isr_handler()` y `room_control_update()`.

---

## ⚙️ Arquitectura de Firmware

### 🧠 **Patrón de Diseño: Super Loop**
El sistema está diseñado siguiendo el patrón Super Loop no bloqueante, donde el bucle principal ejecuta de forma rápida tareas reactivas. Se evita el uso de funciones bloqueantes como HAL_Delay(), utilizando en su lugar HAL_GetTick() para tareas periódicas.

**Ventajas:**

- Baja latencia de respuesta a eventos.

- Alta eficiencia energética.

- Facilidad de mantenimiento y extensión.

**Máquina de Estados (State Machine)**:
  - `ROOM_STATE_LOCKED`: Sistema bloqueado.
  - `ROOM_STATE_INPUT_PASSWORD`: Esperando ingreso de clave.
  - `ROOM_STATE_UNLOCKED`: Acceso concedido.
  - `ROOM_STATE_ACCESS_DENIED`: Acceso denegado.

### 🧩 Diagrama de Estados



 ### 🛰️ Protocolo de Comandos
El sistema permite control remoto vía comandos USART3 & Wifi en formato COMANDO:VALOR\n. Se implementaron los siguientes comandos:

| Comando         | Descripción                                                     |
| --------------- | --------------------------------------------------------------- |
| `GET_TEMP`      | Devuelve la temperatura actual (ej. `28.3°C`)                   |
| `GET_STATUS`    | Estado del sistema: `LOCKED/UNLOCKED`, velocidad del ventilador |
| `SET_PASS:XXXX` | Cambia la contraseña de acceso a `XXXX`                         |
| `FORCE_FAN:N`   | Fuerza la velocidad del ventilador (N = 0-3)                    |

**Evidencia GET_TEMP**

![alt text](image-4.png)

**Evidencia GEt_STATUS**

![alt text](image-3.png)

**Evidencia SET_PASS:XXXX**

![alt text](image-5.png)

**Cuando se introduce mal contraseña**
![alt text](image-2.png)

**Evidencia FORCE_FAN:**

![alt text](image-1.png)


---

## ⚙️ Optimización

### Técnicas Aplicadas

| Técnica                                  | Beneficio                                     |
| ---------------------------------------- | --------------------------------------------- |
| Super Loop no bloqueante                 | Baja latencia, mayor eficiencia energética    |
| Interrupciones en teclado                | Menor uso de CPU, respuesta inmediata         |
| Condicional de actualización OLED        | Reducción de carga I2C                        |
| Uso directo de `__HAL_TIM_SET_COMPARE()` | Cambio inmediato del PWM sin detener el timer |
| Lógica separada de estados y periféricos | Mayor mantenibilidad y escalabilidad          |
| Consolidación de comandos seriales       | Facilita integración con ESP-01 y consola     |



✅ Conclusiones
Este proyecto permitió integrar periféricos, lógica de estado, protocolos de comunicación y diseño modular en un sistema funcional y eficiente. El uso de un patrón Super Loop no bloqueante y una máquina de estados clara garantizó una arquitectura mantenible, escalable y confiable para sistemas embebidos reactivos.