#ifndef SISTEMA_RECOMPENSAS_H
#define SISTEMA_RECOMPENSAS_H

#include "MemoriaJuego.h"
#include "EventosEmerald.h"
#include <memory>
#include <deque> 
#include <string>

class SistemaRecompensas {
private:
    std::shared_ptr<MemoriaJuego> memoria;
    
    struct EstadoAnterior {
        int hp;
        int hpMax;
        int dinero;
        int medallas;
        int pokemonCapturados;
        int posX, posY;
        uint8_t mapaID;
        bool enBatalla;
        int pasosTotal;
    } estadoAnterior;
    
    int pasosRepetidosConsecutivos;
    int framesSinMovimiento;
    
    std::deque<std::pair<int, int>> historialPosiciones; 
    const size_t MAX_HISTORIAL_POS = 10000;
    
    float recompensaAcumulada;
    
public:
    SistemaRecompensas(std::shared_ptr<MemoriaJuego> mem);
    
    std::pair<std::string, float> calcularRecompensa(struct mCore* core);
    
    std::pair<std::string,float> recompensaExploracion(const InfoMapa& mapaActual);
    std::pair<std::string,float>  recompensaProgresion(struct mCore* core);
    std::pair<std::string,float> recompensaBatalla(struct mCore* core, const std::vector<InfoPokemon>& equipo);
    std::pair<std::string,float>  recompensaCaptura(struct mCore* core);
    std::pair<std::string,float> recompensaMedallas(struct mCore* core);
    std::pair<std::string,float> recompensaEventos(struct mCore* core);
    std::pair<std::string,float>  recompensaDinero(struct mCore* core);
    std::pair<std::string,float> recompensaNivel(const std::vector<InfoPokemon>& equipo);
    std::pair<std::string,float>  recompensaDialogos(struct mCore* core);
    
    std::pair<std::string,float> penalizacionRetroceso(const InfoMapa& mapaActual);
    std::pair<std::string,float>  recompensaHito(struct mCore* core);
    
    // Good Ideas But ... MEH
    //std::pair<std::string,float> penalizacionRepeticion(const InfoMapa& mapaActual);
    //float penalizacionInactividad();
    //float penalizacionDanio(const std::vector<InfoPokemon>& equipo);
    //float penalizacionMuerte();
    
    void actualizarEstado(struct mCore* core);
    void resetear();
    float getRecompensaAcumulada() const { return recompensaAcumulada; }
    std::string obtenerDesglose() const;
};
namespace Hitos {
     constexpr float PRIMERA_CAPTURA = 10.0f;
     constexpr float PRIMERA_MEDALLA = 50.0f;
     constexpr float CADA_MEDALLA = 100.0f;
     constexpr float DERROTA_GIMNASIO = 75.0f;
     constexpr float LLEGADA_LIGA = 200.0f;
     constexpr float DERROTA_ELITE_FOUR = 500.0f;
     constexpr float DERROTA_CAMPEON = 1000.0f;
     constexpr float COMPLETAR_POKEDEX_25 = 30.0f;
     constexpr float COMPLETAR_POKEDEX_50 = 60.0f;
     constexpr float COMPLETAR_POKEDEX_75 = 90.0f;
      constexpr float COMPLETAR_POKEDEX_100 = 150.0f;
     constexpr float POKEMON_NIVEL_50 = 20.0f;
     constexpr float POKEMON_NIVEL_100 = 50.0f;
     constexpr float POKEMON_EVOLUCIONADO = 25.0f;
    constexpr float EQUIPO_COMPLETO = 15.0f;
} 
namespace RecompensasBase {
    constexpr float NUEVA_POSICION = 0.0f;        
    constexpr float NUEVO_MAPA = 5.0f;
    constexpr float ZONA_GIMNASIO = 10.0f;
    constexpr float ZONA_LIGA = 25.0f;
    
    constexpr float ESTAR_EN_BATALLA = 0.1f;      
    constexpr float INICIAR_BATALLA = 5.0f;       
    constexpr float GANAR_BATALLA = 15.0f;        
    constexpr float ATACAR_EN_BATALLA = 0.2f;     
    constexpr float DEBILITAR_POKEMON = 3.0f;     
    constexpr float POKEMON_CAPTURADO = 20.0f;    
    
    constexpr float SUBIR_NIVEL = 10.0f;          
    constexpr float GANAR_DINERO = 0.001f;
    constexpr float NUEVA_MEDALLA = 100.0f;
    constexpr float EVENTO_HISTORIA = 20.0f;
    
    constexpr float MANTENER_HP_ALTO = 0.001f;
    constexpr float FRAME_VIVO = 0.001f;
    
    constexpr float DIALOGO_NUEVO = 2.0f;      
}

namespace Penalizaciones {
    constexpr float PERDER_BATALLA = -5.0f;              
    constexpr float PERDER_TODO_EQUIPO = -20.0f;         
    constexpr float DIALOGO_REPETIDO = -1.0f;            
    constexpr float GASTAR_DINERO_INNECESARIO = -0.001f; 
    constexpr float RETROCESO_EXCESIVO = -0.3f;          
    
}

#endif // SISTEMA_RECOMPENSAS_H
