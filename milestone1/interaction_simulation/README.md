# Interaction Simulation

This program is a simulation of a client-intermediary-server architecture
utilizing the chosen network protocol BTP (Basic Transfer Protocol).

The program has a console UI to interact with it from the perspective of the
client. An intermediary will redirect client-sent packets to the server, and
server-sent packets to the client. The program works concurrently with
thread-safe queues, therefore, only one process is needed.

## How to interact with it

The client-side UI prompts the user to choose a request type.
After receiving input, it may display an error message if the input is
ill-formed, or prompt for content to send in a packet.

After submitting both request type and content, a packet is created and sent
through the simulated network. A response packet is received on the client's
end. Both, sent and received packets are printed as is in the console.

The available request types are:

- `PLAIN` — just a plain text message, the server doesn't know what to do with
it. This type is intended for testing purposes.
- `ALL_LEGO` — get the full inventory of lego pieces.
- `LEGO` — look up a specific lego piece by description, to see its
availability.
- `ALL_FIGURES` — get all the figure names that can be built.
- `FIGURE` — get the pieces needed for a specific figure and part,
formatted as `figurename&partnumber`
- `STOP` — ends the simulation cleanly across all threads.

## How to run it

To build the program:

```bash
make
```

To run the program:

```bash
make run
```

For other targets, check the help target:

```bash
make help
```

## Notes

The lego inventory is hardcoded in memory for this simulation. The figures
available are `fish` and `duck`, each with two parts.
