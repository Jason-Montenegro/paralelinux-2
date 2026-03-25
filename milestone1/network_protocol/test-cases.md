# Casos de prueba

Los casos de prueba planteados tienen que ver con el comportamiento que el
flujo de comunicación por paquetes del protocolo BTP debe cumplir.

## Atributos

Los casos planeados están planteados utilizando unas tablas que indican
distintos atributos. La siguiente lista describe a los atributos:

- ```ID```: es un identificador para referirse a un caso
de prueba específico.
- ```Caso```: es la descripción del caso sucedido en un programa que utilice
el protocolo de comunicación.
- ```Cliente```: es el tipo de comportamiento que se espera que realice el
cliente.
- ```Intermediario```: es el tipo de comportamiento que se espera que realice
el intermediario.
- ```Servidor```: es el tipo de comportamiento que se espera que realice el
servidor.
- ```Resultado```: es el resultado o comportamiento esperado resultante de
realizar una operación.

## Tablas de casos de prueba

### Casos normales

| ID | Caso | Cliente | Intermediario | Servidor | Resultado |
|----|------|---------|---------------|----------|-----------|
| 1 | Cliente envía un paquete `text/all-lego` con contenido vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-lego` con todos los legos, sus cantidades y descripciones | El cliente recibe todos los legos almacenados en formato `"quantity":$,"description":"description text"` por cada entrada |
| 2 | Cliente envía un paquete `text/lego` con contenido igual a la descripción de un lego existente | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/lego` con la cantidad actual y descripción de esa pieza | El cliente recibe `"quantity":$,"description":"description text"` con los datos del lego solicitado |
| 3 | Cliente envía un paquete `text/all-figures` con contenido vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-figures` con los nombres de todas las figuras que se pueden crear | El cliente recibe todas las figuras en formato `"name&#","name&#",...` |
| 4 | Cliente envía un paquete `text/figure` con contenido igual al nombre exacto de una figura existente | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/figure` con todas las piezas Lego que conforman esa figura | El cliente recibe las piezas en formato `"quantity":$,"description":"description text"` por cada pieza de la figura |
| 5 | Cliente envía un paquete `text/html/all-lego` con contenido vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/html/all-lego` con todos los legos en formato HTML | El cliente recibe el inventario completo como contenido HTML válido |
| 6 | El inventario del servidor está vacío y el cliente solicita `text/all-lego` | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-lego` con `content-length: 0` y contenido vacío | El cliente recibe un paquete de respuesta válido pero sin ninguna entrada de legos |
| 7 | El inventario del servidor está vacío y el cliente solicita `text/all-figures` | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-figures` con los nombres de todas las figuras que se pueden crear | El cliente recibe todas las figuras en formato `"name&#","name&#",...` |

---

### Casos de contenido no reconocido o malformado

| ID | Caso | Cliente | Intermediario | Servidor | Resultado |
|----|------|---------|---------------|----------|-----------|
| 8 | Cliente envía un paquete `text/lego` con contenido que no corresponde a ningún lego existente | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y no reconoce la pieza. Responde con `text/lego` indicando desconocido | El cliente recibe `"quantity":0,"description":"unknown lego"` |
| 9 | Cliente envía un paquete `text/lego` con contenido vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete con descripción vacía. Al no reconocer ningún lego, responde con `text/lego` indicando desconocido | El cliente recibe `"quantity":0,"description":"unknown lego"` |
| 10 | Cliente envía un paquete `text/figure` con contenido vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete con nombre de figura vacío. Al no reconocer ninguna figura, responde con `text/figure` indicando desconocida | El cliente recibe `"quantity":0,"description":"unknown figure"` |
| 11 | Cliente envía un paquete `text/figure` con contenido que no corresponde a ninguna figura existente | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y no reconoce la figura. Responde con `text/figure` indicando desconocida | El cliente recibe `"quantity":0,"description":"unknown figure"` |
| 12 | Cliente envía un paquete `text/lego` cuya pieza sí existe pero tiene cantidad 0 en el inventario | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete, reconoce la pieza pero no tiene unidades disponibles, responde con `text/lego` | El cliente recibe `"quantity":0,"description":"no inventory"` |
| 13 | Cliente envía un paquete `text/all-lego` con contenido no vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-lego` con todos los legos, sus cantidades y descripciones | El cliente recibe todos los legos almacenados en formato `"quantity":$,"description":"description text"` por cada entrada |
| 14 | Cliente envía un paquete `text/all-figures` con contenido no vacío | Crea y envía el paquete al intermediario | Redirige el paquete del cliente al servidor, y la respuesta del servidor al cliente | Recibe el paquete y responde con `text/all-figures` con los nombres de todas las figuras que se pueden crear | El cliente recibe todas las figuras en formato `"name&#","name&#",...` |

---

### Casos de red y conectividad (para simulaciones)

| ID | Caso | Cliente | Intermediario | Servidor | Resultado |
|----|------|---------|---------------|----------|-----------|
| 15 | Cliente envía un paquete cuya `destination-ip` no corresponde a ningún servidor conectado al intermediario | Crea y envía el paquete al intermediario | No encuentra ninguna conexión con la `destination-ip` indicada. Genera un paquete `UNREACHABLE` hacia el cliente | No participa | El cliente recibe un paquete de tipo `UNREACHABLE` con `content-type: text/plain` cuyo contenido es la IP del servidor inalcanzable |
| 16 | Cliente envía un paquete `STOP` al intermediario | Envía el paquete `STOP` y se desconecta | Recibe el paquete `STOP` y lo reenvía a todos sus nodos conectados para notificar la desconexión. No responde al cliente, y se desconecta | Recibe el paquete `STOP` y finaliza su ejecución | La red se desconecta ordenadamente; ningún actor intenta responder al cliente |
