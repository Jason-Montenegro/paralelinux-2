// Copyright 2026 Paralelinux-2
#include "LegoService.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include "BtpPacket.hpp"
#include "BtpTypes.hpp"
#include "LegoRepository.hpp"

BtpPacket LegoService::handle(const BtpPacket& request) {
  std::string content;
  switch (request.getBtpContentType()) {
    case BtpTypes::BtpContentType::TEXT_ALL_FIGURES:
      content = this->legoRepository.getAllFigures();
      break;
    case BtpTypes::BtpContentType::TEXT_ALL_LEGO:
      content = this->legoRepository.getAllLego();
      break;
    case BtpTypes::BtpContentType::TEXT_FIGURE: {
      std::string requestContent = BtpTypes::contentToMessage(
        request.getContent());
      const size_t separatorIndex = requestContent.find("&");
      if (separatorIndex == std::string::npos) {
        content = "\"quantity\":0,\"description\":\"unknown figure\"";
      } else {
        const std::string name = requestContent.substr(0, separatorIndex);
        const std::string partNumber = requestContent.substr(
          separatorIndex + 1);
        content = this->legoRepository.getFigure(name,
          static_cast<size_t>(std::stoi(partNumber)));
      }
      break;
    }
    case BtpTypes::BtpContentType::TEXT_LEGO:
      content = this->legoRepository.getLego(
        BtpTypes::contentToMessage(request.getContent()));
      break;
    case BtpTypes::BtpContentType::TEXT_PLAIN:
      // This content type is for mocking connections, not for simulations even
      content = "Hey there! I don't know how to respond to your plain "
        "message, but I'm rooting for whatever you're saying :D";
      break;
    default:
      // This implementation only counts for content types relevant to
      // simulations
      content = "Could not resolve any content.";
      break;
  }

  return LegoService::buildResponse(request, request.getBtpContentType(),
    std::move(content));
}

BtpPacket LegoService::buildResponse(const BtpPacket& request,
  BtpTypes::BtpContentType contentType, std::string&& content) {
  return BtpPacket::builder()
    .setIpType(request.getIpType())
    .setSourceIp(request.getDestinationIp())
    .setDestinationIp(request.getSourceIp())
    .setBtpPacketType(BtpTypes::BtpPacketType::RESPONSE)
    .setBtpContentType(contentType)
    .setContent(std::move(BtpTypes::messageToBtpContent(content)))
    .build();
}
