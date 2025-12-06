#ifndef MEMORIA_JUEGO_H
#define MEMORIA_JUEGO_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct mCore;

struct EventoHistoria {
    std::string nombre;
    std::string descripcion;
    uint16_t flagID;
    float recompensa;
    bool completado;
    
    EventoHistoria(const std::string& n, const std::string& d, uint16_t f, float r)
        : nombre(n), descripcion(d), flagID(f), recompensa(r), completado(false) {}
};

namespace DireccionesMemoria {
    constexpr uint32_t POSICION_X = 0x02024284;
    constexpr uint32_t POSICION_Y = 0x02024286;
    constexpr uint32_t MAPA_ID = 0x02036E38;
    
    constexpr uint32_t PARTY_COUNT = 0x02024029;
    constexpr uint32_t PARTY_DATA_START = 0x020244EC;
    constexpr uint32_t PARTY_POKEMON_SIZE = 100;
    
    constexpr uint32_t DINERO = 0x02024490;
    
    constexpr uint32_t DIALOGO_ACTIVO = 0x0202E8D8;
    constexpr uint32_t BATALLA_ACTIVA = 0x02024BE0;
    constexpr uint32_t EN_MENU = 0x0202E8DC;
    
    constexpr uint32_t FLAGS_BASE = 0x02039A5E;
    
    constexpr uint16_t FLAG_MEDALLA_STONE = 0x807;
    constexpr uint16_t FLAG_MEDALLA_KNUCKLE = 0x808;
    constexpr uint16_t FLAG_MEDALLA_DYNAMO = 0x809;
    constexpr uint16_t FLAG_MEDALLA_HEAT = 0x80A;
    constexpr uint16_t FLAG_MEDALLA_BALANCE = 0x80B;
    constexpr uint16_t FLAG_MEDALLA_FEATHER = 0x80C;
    constexpr uint16_t FLAG_MEDALLA_MIND = 0x80D;
    constexpr uint16_t FLAG_MEDALLA_RAIN = 0x80E;
    
    constexpr uint32_t POKEDEX_VISTO = 0x02039D48;
    constexpr uint32_t POKEDEX_CAPTURADO = 0x02039D88;
}

struct InfoPokemon {
    uint16_t species;
    uint16_t hp;
    uint16_t hpMax;
    uint8_t level;
    uint8_t slot;
    
    float vidaPorcentaje() const {
        return hpMax > 0 ? (float)hp / (float)hpMax : 0.0f;
    }
};

struct InfoMapa {
    uint8_t mapaID;
    uint8_t zonaID;
    int16_t posX;
    int16_t posY;
    std::string nombreZona;
    
    bool operator==(const InfoMapa& otro) const {
        return mapaID == otro.mapaID && posX == otro.posX && posY == otro.posY;
    }
};


struct FrameHash {
    uint64_t hash;
    
    bool operator==(const FrameHash& otro) const {
        return hash == otro.hash;
    }
};

namespace std {
    template<>
    struct hash<FrameHash> {
        size_t operator()(const FrameHash& fh) const {
            return static_cast<size_t>(fh.hash);
        }
    };
}

class MemoriaJuego {
private:
    std::unordered_set<FrameHash> framesConocidos;
    
    std::unordered_set<uint32_t> posicionesVisitadas; // (mapaID << 24) | (x << 12) | y
    
    std::unordered_map<uint16_t, bool> flagsEventos;
    std::vector<EventoHistoria> eventosImportantes;
    
    InfoMapa mapaAnterior;
    int dineroAnterior;
    int medallasAnteriores;
    std::vector<InfoPokemon> equipoAnterior;
    
    int totalBatallasGanadas;
    int totalPokemonCapturados;
    int totalPokemonVistos;
    int totalPokemonDebilitados;
    
    bool batallaAnterior;
    
    std::unordered_set<uint64_t> dialogosVistos; 
    uint64_t ultimoFrameDialogo;
    
public:
    MemoriaJuego();
    
    void inicializarEventos();
    
    InfoMapa leerMapaActual(struct mCore* core);
    std::vector<InfoPokemon> leerEquipo(struct mCore* core);
    int leerDinero(struct mCore* core);
    int contarMedallas(struct mCore* core);
    bool leerFlag(struct mCore* core, uint16_t flagID);
    int contarPokemonCapturados(struct mCore* core);
    
    FrameHash calcularHashFrame(const std::vector<uint32_t>& videoBuffer);
    bool esFrameNuevo(const FrameHash& frameHash);
    void registrarFrame(const FrameHash& frameHash);
    
    bool hayCambioDeMapa(const InfoMapa& actual);
    bool hayNuevaPosicion(int x, int y, uint8_t mapaID);
    void registrarPosicion(int x, int y, uint8_t mapaID);
    bool dialogoActivo(struct mCore* core);
    bool batallaActiva(struct mCore* core);
    
    bool esDialogoNuevo(uint64_t hashDialogo);
    void registrarDialogo(uint64_t hashDialogo);
    
    void actualizarHistorial(const InfoMapa& mapa);
    void registrarPokemonCapturado();
    void registrarBatallaGanada();
    
    int getTotalFramesConocidos() const { return framesConocidos.size(); }
    int getTotalPosicionesVisitadas() const { return posicionesVisitadas.size(); }
    int getTotalBatallasGanadas() const { return totalBatallasGanadas; }
    int getTotalPokemonCapturados() const { return totalPokemonCapturados; }
    const std::vector<EventoHistoria>& getEventos() const { return eventosImportantes; }
    std::vector<EventoHistoria>& getEventosMutable() { return eventosImportantes; }
    
    int getDineroAnterior() const { return dineroAnterior; }
    int getMedallasAnteriores() const { return medallasAnteriores; }
    const std::vector<InfoPokemon>& getEquipoAnterior() const { return equipoAnterior; }
    const InfoMapa& getMapaAnterior() const { return mapaAnterior; }
    bool getBatallaAnterior() const { return batallaAnterior; }
    
    void actualizarEstadoAnterior(int dinero, int medallas, const std::vector<InfoPokemon>& equipo, bool enBatalla);
    
    float calcularProgreso() const;
    int getNumMedallas(struct mCore* core);
    bool haCompletadoEvento(uint16_t flagID) const;
    
    void guardarEstado(const std::string& archivo);
    void cargarEstado(const std::string& archivo);
    
};

namespace NombresMapas {
    const std::unordered_map<uint8_t, std::string> NOMBRES = {
        {0, "Littleroot Town"},
        {1, "Oldale Town"},
        {2, "Dewford Town"},
        {3, "Lavaridge Town"},
        {4, "Fallarbor Town"},
        {5, "Verdanturf Town"},
        {6, "Pacifidlog Town"},
        {7, "Ever Grande City"},
        {8, "Slateport City"},
        {9, "Mauville City"},
        {10, "Rustboro City"},
        {11, "Fortree City"},
        {12, "Lilycove City"},
        {13, "Mossdeep City"},
        {14, "Sootopolis City"},
        {15, "Route 101"},
        {16, "Route 102"},
        {17, "Route 103"},
        {30, "Rustboro Gym"},
        {31, "Dewford Gym"},
        {32, "Mauville Gym"},
        {33, "Lavaridge Gym"},
        {34, "Petalburg Gym"},
        {35, "Fortree Gym"},
        {36, "Mossdeep Gym"},
        {37, "Sootopolis Gym"},
        {40, "Pokemon League - Elite Four"},
        {41, "Champion Room"}
    };
}

#endif // MEMORIA_JUEGO_H
