// Copyright 2026 Paralelinux-2

#include <condition_variable>
#include <mutex>
#include <thread>

#include "BtpPacket.hpp"
#include "Client.hpp"
#include "ControlTuple.hpp"
#include "Intermediary.hpp"
#include "Log.hpp"
#include "Server.hpp"
#include "ThreadSafeQueue.hpp"

int main() {
	ThreadSafeQueue<BtpPacket> clientToIntermediary;
	ThreadSafeQueue<BtpPacket> intermediaryToClient;
	ThreadSafeQueue<BtpPacket> intermediaryToServer;
	ThreadSafeQueue<BtpPacket> serverToIntermediary;
	Client client(clientToIntermediary, intermediaryToClient);
	Intermediary intermediary(
		clientToIntermediary, 
		intermediaryToClient,
		intermediaryToServer,
		serverToIntermediary
	);
	Server server(intermediaryToServer, serverToIntermediary);

	// // Make intermediary concurrency control group
	std::mutex intermediaryMtx;
	std::condition_variable intermediaryCv;
	bool intermediaryReady = false;
	ControlTuple intermediaryControl(std::tie(intermediaryMtx,
		intermediaryCv, intermediaryReady));

	// // Make server concurrency control group
	std::mutex serverMtx;
	std::condition_variable serverCv;
	bool serverReady = false;
	ControlTuple serverControl(std::tie(serverMtx, serverCv, serverReady));

	// Mutex to protect standard output.
	std::mutex coutMtx;

	Log& log = Log::getInstance();
	log.start();
	{
		std::jthread clientThread([&client, &intermediaryControl, &coutMtx] {
			client.run(intermediaryControl, coutMtx);
		});

		std::jthread intermediaryThread([&intermediary, &intermediaryControl,
			&serverControl, &coutMtx] {
			intermediary.run(intermediaryControl, serverControl, coutMtx);
		});

		std::jthread serverThread([&server, &serverControl, &coutMtx] {
			server.run(serverControl, coutMtx);
		});	
	}
	log.stop();
}
