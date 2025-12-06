#include "pokemon.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void mostrarAyuda() {
  std::cout << "Usage: ./pokemon_rl_agent [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  -r, --rom <file>          Path to the ROM file (default: "
               "pokemon_emerald.gba)\n";
  std::cout << "  -n, --num <amount>        Number of parallel instances "
               "(default: 1)\n";
  std::cout << "  -h, --help                Show this help message\n\n";
  std::cout << "Generated files:\n";
  std::cout << "  pokemon_dqn_model_<ID>.pt   - Trained model\n";
  std::cout << "  training_stats_<ID>.txt     - Training statistics\n";
  std::cout
      << "═══════════════════════════════════════════════════════════\n\n";
}

struct ArgumentosPrograma {
  std::string rutaROM = "pokemon_emerald.gba";
  int numInstancias = 1;
  bool mostrarHelp = false;
};

ArgumentosPrograma parsearArgumentos(int argc, char *argv[]) {
  ArgumentosPrograma args;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      args.mostrarHelp = true;
      return args;
    } else if ((arg == "-r" || arg == "--rom") && i + 1 < argc) {
      args.rutaROM = argv[++i];
    } else if ((arg == "-n" || arg == "--num") && i + 1 < argc) {
      args.numInstancias = std::stoi(argv[++i]);
      if (args.numInstancias < 1)
        args.numInstancias = 1;
      if (args.numInstancias > 10) {
        std::cout << "10 instances max, no more" << std::endl;
        args.numInstancias = 10;
      }
    } else {
      std::cout << "Args?: " << arg << std::endl;
      args.mostrarHelp = true;
      return args;
    }
  }

  return args;
}

void entrenarInstancia(ConfiguracionAgente config) {
  PokemonRLAgent agente(config);

  if (!agente.inicializar()) {
    std::cerr << "Error instance..." << config.instanciaID << std::endl;
    return;
  }

  agente.entrenar();
}

int main(int argc, char *argv[]) {
  auto args = parsearArgumentos(argc, argv);

  if (args.mostrarHelp) {
    mostrarAyuda();
    return 0;
  }

  std::ifstream romFile(args.rutaROM);
  if (!romFile.good()) {
    std::cerr << "Error: ROM don't found: " << args.rutaROM << std::endl;
    return 1;
  }
  romFile.close();

  if (args.numInstancias == 1) {

    ConfiguracionAgente config;
    config.rutaROM = args.rutaROM;
    config.instanciaID = 0;
    config.offsetX = 0;
    config.offsetY = 0;

    entrenarInstancia(config);
  } else {
    std::vector<std::thread> threads;

    int cols = (args.numInstancias <= 2) ? args.numInstancias : 2;
    int anchoVentana = 240 * 3 + 450;
    int altoVentana = 160 * 3;

    for (int i = 0; i < args.numInstancias; i++) {
      ConfiguracionAgente config;
      config.rutaROM = args.rutaROM;
      config.instanciaID = i;
      config.offsetX = (i % cols) * (anchoVentana + 10);
      config.offsetY = (i / cols) * (altoVentana + 40);

      threads.emplace_back(entrenarInstancia, config);

      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    for (auto &thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  return 0;
}
