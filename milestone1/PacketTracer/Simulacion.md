# Simulación de red, para interconexión de servidores

Este programa busca simular una comunicación de los diferentes dispositivos de red
presentes en la comunicación de nuestro proyecto, utilizando la herramienta Packet Tracer como entorno de desarrollo.

La simulación cuenta con 4 figuras principales:

* Switch: dispositivo de red que conecta otros equipos mediante sus cables Ethernet, este dirige los paquetes utilizando direccionamiento MAC. Además, está configurado para habilitar una VLAN (220) y sus respectivos puertos  (1-12), de manera que hace una segmentación lógica de la red para asegurar la comunicación. Utiliza una búsqueda para encontrar a correctos destinatarios de los paquetes en la red, lo que permite una comunicación correcta.

* Cliente: quien solicita las piezas y establece una conexión con el servidor de las piezas por medio del intermediario (servidor tenedor).este cliente es la parte que representa al usuario y debe poder hacer conexión con el intermediario para solicitar piezas.

* Servidor tenedor: Es el encargado de mediar la conexión entre el cliente y el servidor de piezas, funciona como un "servidor" desde el punto de vista del cliente, pero se dedica a redirigir paquetes a otros intermediarios o servidores.

* Servidor Piezas: es el servidor principal; su trabajo es atender las solicitudes y responderlas, de manera que responde  al cliente con los recursos que necesite.

En conclusión, la simulación de interconexión de dispositivos responde a un modelo de capa 2 en el modelo OSI, este permite comprender el flujo de comunicación entre cliente y servidor mediante el uso de dispositivos intermedios, destacando el papel del switch en la segmentación de red y del servidor intermediario en la gestión de solicitudes.

* Una representacion grafica de la comnicacion:

    Ciente <-Puerto1-> Switch1 <-Puerto2-> ServidorTenedor 
    ServidorPieza <-Puerto3-^