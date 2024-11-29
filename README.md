# SoundMeter | Medidor de Sonido

Este repositorio es para el proyecto electronico final de la carrera de Ingeniería en Electrónica

El mismo consiste en un medidor de ruido ambiente acumulativo que permite conocer en decibeles,
el ruido al que se esta exponiendo el equipo de manera parcial y acumulativa, permitiendo que el
mismo sea contrastado con los estándares de límites de sonido, para lo que se emplea también un sensor
DHT para conocer la humedad y la temperatura relativas presentes en el ambiente a medir.

### Modo de funcionamiento: detección de ruido y pre-procesamiento

El sonido captado a través de un micrófono de alt fidelidad llega hasta una etapa de preamplificación que
entrega una señal variable de voltaje amplificada que es proporcional al sonido captado por el micrófono.

Luego, esta señal preamplificada se envía a un circuito filtro de ponderación para convertir las variaciones
de amplitud de la señal en variaciones de frecuencia, lo que permitirá conocer más adelante los cambios en el sonido
sin necesidad de exponer los circuitos a las variaciones de amplitud o voltaje habituales, esto es, por la simple
razón de que el sistema microcontrolado empleado recibe pulsos máximos con un límite de amplitud limitado.

Luego de este filtrado, la señal se pasa por una etapa de regulación que convierte la señal en una señal idéntica pero
con amplitud constante de un pico de 3.3V, para luego se enviada a la etapa de microcontrolador.
