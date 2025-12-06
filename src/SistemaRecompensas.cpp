#include "SistemaRecompensas.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mgba/core/core.h>
#include <sstream>
#include <string>
#include <utility>

SistemaRecompensas::SistemaRecompensas(std::shared_ptr<MemoriaJuego> mem)
    : memoria(mem), pasosRepetidosConsecutivos(0), framesSinMovimiento(0),
      recompensaAcumulada(0.0f) {

  estadoAnterior = {0, 0, 0, 0, 0, 0, 0, 0, false, 0};
  historialPosiciones.clear();
}

// TF this?? -> fix for something decent
std::pair<std::string, float> SistemaRecompensas::calcularRecompensa(struct mCore *core) {
  float recompensa = 0.0f;
    std::string descFull= "";
 std::pair<std::string, float> aux;   
  auto mapaActual = memoria->leerMapaActual(core);
  auto equipoActual = memoria->leerEquipo(core);

    //News Frames
    aux = recompensaExploracion(mapaActual);
    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
    
    //Progress
    aux = recompensaProgresion(core);
    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;

    //Battle
    aux = recompensaBatalla(core, equipoActual);
    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  
    // Captures
    aux = recompensaCaptura(core);
    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;

  // Medals
    aux = recompensaMedallas(core);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  //EVENTS
  aux= recompensaEventos(core);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  //Money money money
  aux = recompensaDinero(core);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  // 8. Level
  aux = recompensaNivel(equipoActual);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  // NPCs
  aux = recompensaDialogos(core);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  aux = penalizacionRetroceso(mapaActual);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  aux = recompensaHito(core);

    if (aux.second > 0.0f) {
        descFull += aux.first+" : ";
    }
    recompensa += aux.second;
  
  recompensa += RecompensasBase::FRAME_VIVO;

  actualizarEstado(core);

  recompensaAcumulada += recompensa;
  return {descFull,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaExploracion(const InfoMapa &mapaActual) {
  float recompensa = 0.0f;
  std::string desc="";
  if (memoria->hayNuevaPosicion(mapaActual.posX, mapaActual.posY,
                                mapaActual.mapaID)) {
    memoria->registrarPosicion(mapaActual.posX, mapaActual.posY,
                               mapaActual.mapaID);
    pasosRepetidosConsecutivos = 0;
    framesSinMovimiento = 0;
  }

  if (memoria->hayCambioDeMapa(mapaActual)) {
    recompensa += RecompensasBase::NUEVO_MAPA;
    desc = "New map";
    if (mapaActual.nombreZona.find("Gym") != std::string::npos) {
      recompensa += RecompensasBase::ZONA_GIMNASIO;
      desc += " Gym";
    }
    if (mapaActual.nombreZona.find("League") != std::string::npos ||
        mapaActual.nombreZona.find("Elite") != std::string::npos) {
      recompensa += RecompensasBase::ZONA_LIGA;
            desc += " League";
    }

    memoria->actualizarHistorial(mapaActual);
  }

  
  return {desc,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaProgresion(struct mCore *core) {
  float recompensa = 0.0f;
  float progresoActual = memoria->calcularProgreso();
  recompensa += progresoActual * 0.01f;

  return {"Progress",recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaBatalla(struct mCore *core, const std::vector<InfoPokemon> &equipo) {
  float recompensa = 0.0f;
    std::string desc = "";
  if (estadoAnterior.pasosTotal < 10000) {
    return {"",0.0f};
  }
  bool enBatallaAhora = memoria->batallaActiva(core);
  bool batallaAnterior = memoria->getBatallaAnterior(); 
  if (enBatallaAhora) {
    recompensa += RecompensasBase::ESTAR_EN_BATALLA;
        desc = "Battle";
  }

  if (enBatallaAhora && !batallaAnterior) {
    recompensa += RecompensasBase::INICIAR_BATALLA;
  }

  if (!enBatallaAhora && batallaAnterior) {
    bool equipoVivo = false;
    for (const auto &pkmn : equipo) {
      if (pkmn.hp > 0) {
        equipoVivo = true;
        break;
      }
    }

    if (equipoVivo) {
      recompensa += RecompensasBase::GANAR_BATALLA;
      memoria->registrarBatallaGanada();
    } else {
      recompensa += Penalizaciones::PERDER_BATALLA;
            desc = "Loss Battle";
    }
  }

  if (enBatallaAhora && !equipo.empty()) {
    float hpPromedio = 0.0f;
    for (const auto &pkmn : equipo) {
      hpPromedio += pkmn.vidaPorcentaje();
    }
    hpPromedio /= equipo.size();

    if (hpPromedio > 0.8f) {
      recompensa += RecompensasBase::ATACAR_EN_BATALLA;
    }
  }

  return {desc,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaCaptura(struct mCore *core) {
  float recompensa = 0.0f;
    std::string desc="Capture";

  int capturasActuales = memoria->contarPokemonCapturados(core);

  if (estadoAnterior.pasosTotal < 10000) {
    estadoAnterior.pokemonCapturados = capturasActuales;
    return {"",0.0f};
  }

  if (capturasActuales > estadoAnterior.pokemonCapturados) {
    int capturasNuevas = capturasActuales - estadoAnterior.pokemonCapturados;
    recompensa += RecompensasBase::POKEMON_CAPTURADO * capturasNuevas;

    if (capturasActuales == 1) {
      recompensa += Hitos::PRIMERA_CAPTURA;
        desc = "First Capture";
    }

    memoria->registrarPokemonCapturado();
  }

  return {desc,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaMedallas(struct mCore *core) {
  float recompensa = 0.0f;
    std::string desc = "Medal";
  int medallasActuales = memoria->contarMedallas(core);

  if (medallasActuales > estadoAnterior.medallas) {
    int medallasNuevas = medallasActuales - estadoAnterior.medallas;
    recompensa += RecompensasBase::NUEVA_MEDALLA * medallasNuevas;
    recompensa += Hitos::CADA_MEDALLA * medallasNuevas;

    if (medallasActuales == 1) {
      recompensa += Hitos::PRIMERA_MEDALLA;
        desc = "First Medal";
    }

    if (medallasActuales == 8) {
      recompensa += Hitos::LLEGADA_LIGA;
        desc = "Full Medals";
    }
  }

  return {desc,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaEventos(struct mCore *core) {
  float recompensa = 0.0f;
  int eventosNuevos = 0;
    std::string desc = "";

  auto &eventos = memoria->getEventosMutable();
  for (auto &evento : eventos) {
    if (evento.flagID == 0xFFFF)
      continue;

    if (!evento.completado && memoria->leerFlag(core, evento.flagID)) {
      recompensa += evento.recompensa;
      evento.completado = true;
      eventosNuevos++;
        desc = evento.descripcion;
    }
  }

  return {desc,recompensa};
}

std::pair<std::string, float>  SistemaRecompensas::recompensaDinero(struct mCore *core) {
  float recompensa = 0.0f;

  int dineroActual = memoria->leerDinero(core);
  int diferencia = dineroActual - estadoAnterior.dinero;

  if (diferencia > 0) {
    recompensa += diferencia * RecompensasBase::GANAR_DINERO;
  }
  else if (diferencia < 0) {
    recompensa +=
        std::abs(diferencia) * Penalizaciones::GASTAR_DINERO_INNECESARIO;
  }

  return {"Dinero",recompensa};
}

 std::pair<std::string, float> SistemaRecompensas::recompensaNivel(const std::vector<InfoPokemon> &equipo) {
  float recompensa = 0.0f;
    std::string desc = "LevelUp";

  auto equipoAnterior = memoria->getEquipoAnterior();

  for (size_t i = 0; i < equipo.size() && i < equipoAnterior.size(); i++) {
    if (equipo[i].level > equipoAnterior[i].level) {
      int nivelesSubidos = equipo[i].level - equipoAnterior[i].level;
      recompensa += RecompensasBase::SUBIR_NIVEL * nivelesSubidos;
    }

    if (equipo[i].species != equipoAnterior[i].species &&
        equipoAnterior[i].species != 0) {
      recompensa += Hitos::POKEMON_EVOLUCIONADO;
            desc ="Evo";
    }

    if (equipo[i].level >= 50 && equipo[i].level < 100) {
      recompensa += Hitos::POKEMON_NIVEL_50 * 0.001f;
            desc = "Lvl50";
    }
    if (equipo[i].level == 100) {
      recompensa += Hitos::POKEMON_NIVEL_100 * 0.001f;
            desc = "Lvl100";
    }
  }

  if (equipo.size() == 6) {
    recompensa += Hitos::EQUIPO_COMPLETO * 0.001f;
        desc = "FullTeam";
  }

  return {desc,recompensa};
}
 std::pair<std::string, float> SistemaRecompensas::penalizacionRetroceso(const InfoMapa &mapaActual) {
  float penalizacion = 0.0f;
    std::string desc = "";

  std::pair<int, int> posicionActual = {mapaActual.posX, mapaActual.posY};
  historialPosiciones.push_back(posicionActual);

  const size_t VENTANA_HISTORIAL = 20000;
  if (historialPosiciones.size() > VENTANA_HISTORIAL) {
    historialPosiciones.erase(historialPosiciones.begin());
  }

  if (historialPosiciones.size() < 15000) {
    return {desc,0.0f};
  }

  int vecesVisitada = 0;
  for (const auto &pos : historialPosiciones) {
    if (pos.first == posicionActual.first && pos.second == posicionActual.second) {
      vecesVisitada++;
    }
  }

  if (vecesVisitada >= 10) {
    penalizacion = Penalizaciones::RETROCESO_EXCESIVO * (vecesVisitada - 2);
    desc = "Retroceso";
    if (penalizacion < -5.0f) {
      penalizacion = -5.0f;
    }
  }

  if (historialPosiciones.size() >= 10) {
    auto &p1 = historialPosiciones[historialPosiciones.size() - 4];
    auto &p2 = historialPosiciones[historialPosiciones.size() - 3];
    auto &p3 = historialPosiciones[historialPosiciones.size() - 2];
    auto &p4 = historialPosiciones[historialPosiciones.size() - 1];

    // A-B-A-B
    if (p1 == p3 && p2 == p4 && p1 != p2) {
      penalizacion += Penalizaciones::RETROCESO_EXCESIVO;
            desc="Ping Pong";
    }
  }

  return {desc,penalizacion};
}

std::pair<std::string, float>  SistemaRecompensas::recompensaHito(struct mCore *core) {
  float recompensa = 0.0f;
    std::string desc="";

  if (memoria->leerFlag(core, 0x82E) && !memoria->haCompletadoEvento(0x82E)) {
    recompensa += Hitos::DERROTA_ELITE_FOUR * 0.25f;
        desc="Lider 1";
  }
  if (memoria->leerFlag(core, 0x82F) && !memoria->haCompletadoEvento(0x82F)) {
    recompensa += Hitos::DERROTA_ELITE_FOUR * 0.25f;
        desc="Lider 2";
  }
  if (memoria->leerFlag(core, 0x830) && !memoria->haCompletadoEvento(0x830)) {
    recompensa += Hitos::DERROTA_ELITE_FOUR * 0.25f;
        desc="Lider 3";
  }
  if (memoria->leerFlag(core, 0x831) && !memoria->haCompletadoEvento(0x831)) {
    recompensa += Hitos::DERROTA_ELITE_FOUR * 0.25f;
        desc="Lider 4";
  }
  if (memoria->leerFlag(core, 0x832) && !memoria->haCompletadoEvento(0x832)) {
    recompensa += Hitos::DERROTA_CAMPEON;
        desc="Goat";
  }

  return {desc,recompensa};
}

std::pair<std::string, float> SistemaRecompensas::recompensaDialogos(struct mCore *core) {
  float recompensa = 0.0f;
    std::string desc ="";
  if (!memoria->dialogoActivo(core)) {
    return {"",0.0f};
  }

  auto mapa = memoria->leerMapaActual(core);
  uint64_t hashDialogo = (static_cast<uint64_t>(mapa.mapaID) << 32) |
                         (static_cast<uint64_t>(mapa.posX) << 16) |
                         static_cast<uint64_t>(mapa.posY);

  if (memoria->esDialogoNuevo(hashDialogo)) {
    recompensa += RecompensasBase::DIALOGO_NUEVO;
        desc = "Txt";
    memoria->registrarDialogo(hashDialogo);
  } else {
        desc ="Same Txt";
    recompensa += Penalizaciones::DIALOGO_REPETIDO;
  }

  return {desc,recompensa};
}

void SistemaRecompensas::actualizarEstado(struct mCore *core) {
  auto equipo = memoria->leerEquipo(core);
  auto mapa = memoria->leerMapaActual(core);
  bool enBatalla = memoria->batallaActiva(core);

  if (!equipo.empty()) {
    estadoAnterior.hp = equipo[0].hp;
    estadoAnterior.hpMax = equipo[0].hpMax;
  }

  estadoAnterior.dinero = memoria->leerDinero(core);
  estadoAnterior.medallas = memoria->contarMedallas(core);
  estadoAnterior.pokemonCapturados = memoria->contarPokemonCapturados(core); 
  estadoAnterior.posX = mapa.posX;
  estadoAnterior.posY = mapa.posY;
  estadoAnterior.mapaID = mapa.mapaID;
  estadoAnterior.enBatalla = enBatalla;
  estadoAnterior.pasosTotal++;

  memoria->actualizarEstadoAnterior(estadoAnterior.dinero, estadoAnterior.medallas, equipo, enBatalla);
}

void SistemaRecompensas::resetear() {
  pasosRepetidosConsecutivos = 0;
  framesSinMovimiento = 0;
  recompensaAcumulada = 0.0f;
  estadoAnterior = {0, 0, 0, 0, 0, 0, 0, 0, false, 0};
  historialPosiciones.clear();
}

std::string SistemaRecompensas::obtenerDesglose() const {
  std::stringstream ss;
  ss << "Accumulated reward: " << recompensaAcumulada << "\n";
  ss << "Steps without movement: " << framesSinMovimiento << "\n";
  ss << "Repeated steps: " << pasosRepetidosConsecutivos << "\n";
  return ss.str();
}
