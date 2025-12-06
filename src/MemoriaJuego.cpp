#include "MemoriaJuego.h"
#include "EventosEmerald.h"
#include <mgba/core/core.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream> 

MemoriaJuego::MemoriaJuego() 
    : mapaAnterior{0, 0, 0, 0, ""},
      dineroAnterior(0),
      medallasAnteriores(0),
      totalBatallasGanadas(0),
      totalPokemonCapturados(0),
      totalPokemonVistos(0),
      totalPokemonDebilitados(0),
      batallaAnterior(false),
      ultimoFrameDialogo(0) {
    inicializarEventos();
}

void MemoriaJuego::inicializarEventos() {
    eventosImportantes = EventosEmerald::obtenerTodosLosEventos();
}

InfoMapa MemoriaJuego::leerMapaActual(struct mCore* core) {
    InfoMapa info;
    info.posX = static_cast<int16_t>(core->rawRead16(core, DireccionesMemoria::POSICION_X, -1));
    info.posY = static_cast<int16_t>(core->rawRead16(core, DireccionesMemoria::POSICION_Y, -1));
    info.mapaID = core->rawRead8(core, DireccionesMemoria::MAPA_ID, -1);
    info.zonaID = core->rawRead8(core, DireccionesMemoria::MAPA_ID + 1, -1);
    
    auto it = NombresMapas::NOMBRES.find(info.mapaID);
    info.nombreZona = (it != NombresMapas::NOMBRES.end()) ? it->second : "Unknown Area";
    
    return info;
}

std::vector<InfoPokemon> MemoriaJuego::leerEquipo(struct mCore* core) {
    std::vector<InfoPokemon> equipo;
    
    uint8_t partyCount = core->rawRead8(core, DireccionesMemoria::PARTY_COUNT, -1);
    if (partyCount > 6) partyCount = 6;
    
    for (uint8_t i = 0; i < partyCount; i++) {
        uint32_t baseAddr = DireccionesMemoria::PARTY_DATA_START + (i * DireccionesMemoria::PARTY_POKEMON_SIZE);
        
        InfoPokemon pokemon;
        pokemon.slot = i;
        pokemon.species = core->rawRead16(core, baseAddr + 32, -1);
        pokemon.hp = core->rawRead16(core, baseAddr + 86, -1);
        pokemon.hpMax = core->rawRead16(core, baseAddr + 88, -1);
        pokemon.level = core->rawRead8(core, baseAddr + 84, -1);
        
        equipo.push_back(pokemon);
    }
    
    return equipo;
}

int MemoriaJuego::leerDinero(struct mCore* core) {
    uint32_t dinero = core->rawRead32(core, DireccionesMemoria::DINERO, -1);
    return static_cast<int>(dinero & 0xFFFFFF);
}

bool MemoriaJuego::leerFlag(struct mCore* core, uint16_t flagID) {
    uint32_t byteOffset = flagID / 8;
    uint8_t bitOffset = flagID % 8;
    uint8_t byte = core->rawRead8(core, DireccionesMemoria::FLAGS_BASE + byteOffset, -1);
    return (byte & (1 << bitOffset)) != 0;
}

int MemoriaJuego::contarMedallas(struct mCore* core) {
    int medallas = 0;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_STONE)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_KNUCKLE)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_DYNAMO)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_HEAT)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_BALANCE)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_FEATHER)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_MIND)) medallas++;
    if (leerFlag(core, DireccionesMemoria::FLAG_MEDALLA_RAIN)) medallas++;
    return medallas;
}

int MemoriaJuego::getNumMedallas(struct mCore* core) {
    return contarMedallas(core);
}

int MemoriaJuego::contarPokemonCapturados(struct mCore* core) {
    int capturados = 0;
    
    for (int i = 0; i < 52; i++) { // 52 bytes = 416 bits
        uint8_t byte = core->rawRead8(core, DireccionesMemoria::POKEDEX_CAPTURADO + i, -1);
        for (int bit = 0; bit < 8; bit++) {
            if (byte & (1 << bit)) {
                capturados++;
            }
        }
    }
    
    return capturados;
}

FrameHash MemoriaJuego::calcularHashFrame(const std::vector<uint32_t>& videoBuffer) {
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t fnvPrime = 1099511628211ULL;
    
    const size_t step = 8;
    
    for (size_t i = 0; i < videoBuffer.size(); i += step) {
        hash ^= videoBuffer[i];
        hash *= fnvPrime;
    }
    
    return FrameHash{hash};
}

bool MemoriaJuego::esFrameNuevo(const FrameHash& frameHash) {
    return framesConocidos.find(frameHash) == framesConocidos.end();
}

void MemoriaJuego::registrarFrame(const FrameHash& frameHash) {
    framesConocidos.insert(frameHash);
}

bool MemoriaJuego::hayCambioDeMapa(const InfoMapa& actual) {
    return actual.mapaID != mapaAnterior.mapaID;
}

bool MemoriaJuego::hayNuevaPosicion(int x, int y, uint8_t mapaID) {
    uint32_t posicionCodificada = (static_cast<uint32_t>(mapaID) << 24) | 
                                  (static_cast<uint32_t>(x & 0xFFF) << 12) | 
                                  (static_cast<uint32_t>(y & 0xFFF));
    return posicionesVisitadas.find(posicionCodificada) == posicionesVisitadas.end();
}

void MemoriaJuego::registrarPosicion(int x, int y, uint8_t mapaID) {
    uint32_t posicionCodificada = (static_cast<uint32_t>(mapaID) << 24) | 
                                  (static_cast<uint32_t>(x & 0xFFF) << 12) | 
                                  (static_cast<uint32_t>(y & 0xFFF));
    posicionesVisitadas.insert(posicionCodificada);
}

bool MemoriaJuego::dialogoActivo(struct mCore* core) {
    uint8_t flag = core->rawRead8(core, DireccionesMemoria::DIALOGO_ACTIVO, -1);
    return flag != 0;
}

bool MemoriaJuego::batallaActiva(struct mCore* core) {
    uint8_t flag = core->rawRead8(core, DireccionesMemoria::BATALLA_ACTIVA, -1);
    return flag != 0;
}

bool MemoriaJuego::esDialogoNuevo(uint64_t hashDialogo) {
    return dialogosVistos.find(hashDialogo) == dialogosVistos.end();
}

void MemoriaJuego::registrarDialogo(uint64_t hashDialogo) {
    dialogosVistos.insert(hashDialogo);
    ultimoFrameDialogo = hashDialogo;
}

void MemoriaJuego::actualizarHistorial(const InfoMapa& mapa) {
    registrarPosicion(mapa.posX, mapa.posY, mapa.mapaID);
    mapaAnterior = mapa;
}

void MemoriaJuego::registrarPokemonCapturado() {
    totalPokemonCapturados++;
    
    if (totalPokemonCapturados == 10 || totalPokemonCapturados == 25 || 
        totalPokemonCapturados == 50 || totalPokemonCapturados == 100 ||
        totalPokemonCapturados == 150 || totalPokemonCapturados == 200) {
    }
}

void MemoriaJuego::registrarBatallaGanada() {
    totalBatallasGanadas++;
}

void MemoriaJuego::actualizarEstadoAnterior(int dinero, int medallas, 
                                           const std::vector<InfoPokemon>& equipo,
                                           bool enBatalla) {
    dineroAnterior = dinero;
    medallasAnteriores = medallas;
    equipoAnterior = equipo;
    batallaAnterior = enBatalla;
}

float MemoriaJuego::calcularProgreso() const {
    float progresoMedallas = (medallasAnteriores / 8.0f) * 0.6f;
    float progresoPokedex = (totalPokemonCapturados / 386.0f) * 0.2f;
    
    int eventosCompletados = 0;
    for (const auto& evento : eventosImportantes) {
        if (evento.completado) eventosCompletados++;
    }
    float progresoEventos = (eventosCompletados / static_cast<float>(eventosImportantes.size())) * 0.2f;
    
    return progresoMedallas + progresoPokedex + progresoEventos;
}

bool MemoriaJuego::haCompletadoEvento(uint16_t flagID) const {
    auto it = flagsEventos.find(flagID);
    return it != flagsEventos.end() && it->second;
}

void MemoriaJuego::guardarEstado(const std::string& archivo) {
    std::ofstream out(archivo, std::ios::binary);
    if (!out) return;
    
    out.write(reinterpret_cast<const char*>(&totalBatallasGanadas), sizeof(totalBatallasGanadas));
    out.write(reinterpret_cast<const char*>(&totalPokemonCapturados), sizeof(totalPokemonCapturados));
    out.write(reinterpret_cast<const char*>(&totalPokemonVistos), sizeof(totalPokemonVistos));
    
    size_t numPosiciones = posicionesVisitadas.size();
    out.write(reinterpret_cast<const char*>(&numPosiciones), sizeof(numPosiciones));
    for (const auto& pos : posicionesVisitadas) {
        out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
    }
    
    size_t numFrames = framesConocidos.size();
    out.write(reinterpret_cast<const char*>(&numFrames), sizeof(numFrames));
    for (const auto& frame : framesConocidos) {
        out.write(reinterpret_cast<const char*>(&frame.hash), sizeof(frame.hash));
    }
    
    out.close();
}

void MemoriaJuego::cargarEstado(const std::string& archivo) {
    std::ifstream in(archivo, std::ios::binary);
    if (!in) return;
    
    in.read(reinterpret_cast<char*>(&totalBatallasGanadas), sizeof(totalBatallasGanadas));
    in.read(reinterpret_cast<char*>(&totalPokemonCapturados), sizeof(totalPokemonCapturados));
    in.read(reinterpret_cast<char*>(&totalPokemonVistos), sizeof(totalPokemonVistos));
    
    size_t numPosiciones;
    in.read(reinterpret_cast<char*>(&numPosiciones), sizeof(numPosiciones));
    posicionesVisitadas.clear();
    for (size_t i = 0; i < numPosiciones; i++) {
        uint32_t pos;
        in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        posicionesVisitadas.insert(pos);
    }
    
    size_t numFrames;
    in.read(reinterpret_cast<char*>(&numFrames), sizeof(numFrames));
    framesConocidos.clear();
    for (size_t i = 0; i < numFrames; i++) {
        uint64_t hash;
        in.read(reinterpret_cast<char*>(&hash), sizeof(hash));
        framesConocidos.insert(FrameHash{hash});
    }
    
    in.close();
}
