<div align="center">

<img width="535"  alt="image" src="https://github.com/GonzAnth/tdse-tpf_1-06/blob/main/logo-fiuba.png" />

# UNIVERSIDAD DE BUENOS AIRES
**Facultad de Ingeniería** **TA134 - Taller de Sistemas Embebidos** Curso 1 - Grupo 6

## Sistema de Riego Automático 

**Autores:** Gonzalo Antahuara (109965) & Dante Mele Ientile (109494)  
**Fecha:** 06/03/2026  
**Cuatrimestre de cursada:** 1er cuatrimestre 2025

---

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![STM32](https://img.shields.io/badge/MCU-STM32F103RB-orange.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103rb.html)

</div>

El presente trabajo consiste en la implementación de un **sistema de monitoreo y control de riego autónomo** basado en la placa de desarrollo **NUCLEO-F103RB**. 
El sistema mide variables ambientales tales como la temperatura y la humedad para gestionar la activación de una electroválvula, optimizando el uso del agua en cultivos de pequeña escala.

## Características Principales

* **Mediciones:** Uso del sensor digital **SHT85** para mediciones de temperatura (± 0,1 °C) y humedad (± 1,5 %RH).
* **Modos de Operación:** Selección mediante la interfaz de usuario entre los modos *Manual*, *Timer* (por tiempo fijo) y *Sensor & Timer* (automático por umbrales).
* **Interfaz de Usuario:** Navegación por menús en pantalla **LCD 20x4** con retroalimentación auditiva mediante un **buzzer**.
* **Almacenamiento de Datos:** Almacenamiento de los parámetros de la configuración del usuario (umbrales y tiempos) en la **memoria Flash interna** del microcontrolador.
* **Gestión de Energía:** Implementación de **Modo Sleep** para reducir el consumo eléctrico durante periodos de inactividad.

## Especificaciones Técnicas

### Hardware
* **Microcontrolador:** Placa NUCLEO-F103RB basada en **ARM Cortex-M3**.
* **Sensor:** Sensor SHT85 comunicado vía protocolo **I2C** con verificación **CRC-8**.
* **Actuador:** Módulo relé con **optoacoplador** para aislamiento eléctrico de la etapa de potencia.
* **Alimentación:** Fuente externa de 5 V / 1 A para garantizar la autonomía del sistema.
* **Diseño Físico:** PCB diseñada en formato **Shield** para robustez mecánica y reducción de ruido eléctrico.

### Firmware
* **Arquitectura:** Bare-metal con esquema de *Super-Loop* y tick de 1 ms.
* **Paradigma:** Sistema disparado por eventos *Event-Triggered System* utilizando Máquinas de Estados no bloqueantes.
* **Rendimiento:** Optimización del peor tiempo de ejecución **WCET**, logrando una ocupación máxima de CPU del 59,1 % en el peor de los casos.

## Estructura del Proyecto

* `task_system`: Máquina de estados principal que administra los modos de operación y lógica de riego.
* `task_menu`: Gestión del menú interactivo y actualización parcial de la pantalla LCD.
* `task_sht85`: Controlador del sensor de humedad y temperatura mediante interrupciones.
* `task_adc`: Monitoreo de la salud del sistema (tensión y temperatura interna) vía DMA.
* `task_actuator`: Control escalable del módulo relé y buzzer.

## Organización del Repositorio

El ciclo de desarrollo se gestiona aplicando estándares de control de versiones y revisión de Pull Requests:
* **main**: Rama de entregables. Contiene los archivos en PDF, la presentación y esta documentación.
* **develop**: Rama activa de desarrollo. Contiene el firmware completo del proyecto (/App, /Drivers, etc.).
* **hardware**: Desarrollo de hardware del proyecto.

## Documentación Adicional

* **[Memoria del Trabajo Final - Grupo 6](https://github.com/GonzAnth/tdse-tpf_1-06/blob/main/1_Memoria%20del%20Trabajo%20Final%20-%20Grupo%206.pdf)**.
* **[Presentación TDSE - Grupo 6](https://github.com/GonzAnth/tdse-tpf_1-06/blob/main/3_Presentaci%C3%B3n%20TDSE%20-%20Grupo%206.pdf)**.

## 👥 Autores
* **Gonzalo Antahuara**
* **Dante Mele Ientile**

---
*Este proyecto fue desarrollado como Trabajo Final para el Taller de Sistemas Embebidos de la Facultad de Ingeniería de la Universidad de Buenos Aires (FIUBA)*.
