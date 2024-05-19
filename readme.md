# Monitor de Temperatura

La idea es crear un sistema que pueda estar registrando informacion cada 5 minutos sobre la temperatura/humedad de la tierra.

Infrestructura

- ESP32
- Sensor de humedad y temperatura
- AWS (DynamoDB, Lambda Functions, IoT)


El Webserver almacenado en el ESP32:
- Registrara continuamente la temperatura y humedad.
- Actuara de API para la visual de historial y temperatura actual.
- Enviara a la nube por protocolo MQTT donde se tomara el mensaje y por medio de Lambda se guardara el dato en DynamoDB donde se podra utilizar a posteori para crear un dashboard mas funcional.
- Enviara una alerta a el usuario por medio de un bot de telegram, alertando de un aumento.