## Simulacion con Vsocket 
esta carpeta propone una implementacion con switch tentativa, se requiere tener clases VSocket para que realmente funcione.
Se requiere modificar metodo de multicast de intermediarios, asi como una estructura de paquetes,
Falta de switch Handshake/Join UDP, Iterar intermediarios con sockets, tomar el primero en responder, 

### Comunicación por `Connection`
 
No hay sockets reales. Cada par de actores se comunica a través de un `Connection`, que contiene dos colas thread-safe compartidas:
 
```
clientSide.outbound  ->  interClientSide.inbound   (cliente → intermediario)
clientSide.inbound   <-  interClientSide.outbound  (intermediario → cliente)
```
 
**Importante:** al construir las conexiones en `Simulation::run()`, los punteros se cruzan manualmente. Si se pasa la misma `Connection` a ambos lados sin cruzar, los dos extremos leen y escriben en las mismas colas y el sistema se bloquea.
 
### Flujo de un mensaje
 
```
Usuario
  │  escribe opción en consola
  
Client1
  │  push -clientSide.outbound
Intermediary1
  │  pop  - interClientSide.inbound
  │  consulta RouteTable
  │  push -interServerSide.outbound
Server1 / LegoManager
  │  pop  -serverSide.inbound
  │  busca figura en LegoRepository
  │  push - serverSide.outbound
Intermediary1
  │  pop  - interServerSide.inbound
  │  push - interClientSide.outbound
Client1
  │  pop  <- clientSide.inbound
Consola (resultado)
```
 
### Sincronización de arranque
 
El orden de inicio importa. Se usan dos `Control` (mutex + condition_variable):
 
1. `Server1` arranca y señala `serverControl.ready = true`.
2. `Intermediary1` espera `serverControl`, inicializa la tabla de rutas (ASK_FIGURE ALL), luego señala `interControl.ready = true`.
3. `Client1` espera `interControl` y solo entonces muestra el menú.
Si el servidor no está corriendo cuando el intermediario intenta `initRouteTable()`, el sistema se bloquea indefinidamente.
 
---




