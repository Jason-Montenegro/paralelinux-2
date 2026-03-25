# Protocolo de registro de eventos en bitácoras

Aunado al protocolo de red BTP, se necesita que los servidores de la red
utilicen un formato de bitácoras específico. Las bitácoras se deberán de
almacenar en un directorio ```logs/```, y tendrán todos el mismo nombre, pero
el nombre de archivo estará terminado por ```-#.log``` donde ```#``` representa
el identificador de la ejecución del programa.

## Formato de bitácora

Se almacenarán una serie continua de eventos ocurridos en el archivo
de bitácora. Dentro de las bitácoras se utilizará el siguiente formato por línea.

```bash
[Fecha] - [Tipo de evento]: [Mensaje de lo ocurrido]
```

Un ejemplo de esto sería el siguiente:

```bash
Mon Mar 23 07:05:57 2026 - [INFO]: Intermediary (192.168.0.12) redirecting packet to client (192.168.0.10).
```

## Tipos de evento

### INFO

Este tipo de evento se utiliza para anunciar que ocurrió algún tipo de evento
que no interrumpe el flujo esperado de la red, pero que es relevante para el
monitoreo del servidor que esté en ejecución.

### ERROR

Este tipo de evento se utiliza para anunciar que ocurrió un comportamiento
inesperado durante la ejecución de las rutinas del servidor. Este tipo de
evento engloba cualquier tipo de error, ya sea, un error que impida el
funcioamiento de la red, un error de lógica, o cualquier evento que sea anómalo
al funcionamiento correcto y esperado del servidor.
