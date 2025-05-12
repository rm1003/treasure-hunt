#include "RawSocket.hpp"
#include <cstddef>
#include <sys/socket.h>

int CustomSocket::RawSocket::CreateSocket(char *nome_interface_rede) {
  // Cria arquivo para o socket sem qualquer protocolo
  this->socketFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (this->socketFd == -1) {
    fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
    return 1;
  }

  int ifindex = if_nametoindex(nome_interface_rede);

  struct sockaddr_ll endereco = {0};
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ifindex;
  // Inicializa socket
  if (bind(this->socketFd, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
      fprintf(stderr, "Erro ao fazer bind no socket\n");
      return 1;
  }

  struct packet_mreq mr = {0};
  mr.mr_ifindex = ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  // Não joga fora o que identifica como lixo: Modo promíscuo
  if (setsockopt(this->socketFd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
    fprintf(stderr, "Erro ao fazer setsockopt: "
        "Verifique se a interface de rede foi especificada corretamente.\n");
    return 1;
  }

  return 0;
}

CustomSocket::RawSocket::RawSocket(char *ethInterfaceName) {
  if (this->CreateSocket(ethInterfaceName)) {
    exit(1);
  }
}

int CustomSocket::RawSocket::Send(char *str, size_t len) {
  int ret;

  ret = send(this->socketFd, str, len, 0);
  return ret;
}

int CustomSocket::RawSocket::Recv(char *str, size_t len) {
  int ret;

  ret = recv(this->socketFd, str, len, 0);
  return ret;
}
