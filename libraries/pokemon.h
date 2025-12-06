#ifndef POKEMON_H
#define POKEMON_H

#include <mgba/core/core.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <torch/torch.h>
#include <string>
#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <memory>

#include "MemoriaJuego.h"
#include "SistemaRecompensas.h"

#ifndef GBA_KEY_A
#define GBA_KEY_A      (1 << 0)
#define GBA_KEY_B      (1 << 1)
#define GBA_KEY_SELECT (1 << 2)
#define GBA_KEY_START  (1 << 3)
#define GBA_KEY_RIGHT  (1 << 4)
#define GBA_KEY_LEFT   (1 << 5)
#define GBA_KEY_UP     (1 << 6)
#define GBA_KEY_DOWN   (1 << 7)
#define GBA_KEY_R      (1 << 8)
#define GBA_KEY_L      (1 << 9)
#endif

struct DQNImpl : torch::nn::Module {
    torch::nn::Conv2d conv1{nullptr}, conv2{nullptr}, conv3{nullptr};
    torch::nn::Linear fc1{nullptr}, fc2{nullptr};
    
    DQNImpl();
    torch::Tensor forward(torch::Tensor x);
};
TORCH_MODULE(DQN);

struct Experience {
    torch::Tensor estado;
    int accion;
    float recompensa;
    torch::Tensor siguienteEstado;
    bool terminado;
};

struct MovimientoHistorial {
    std::string descripcion;
    float recompensa;
    int frame;
};

struct ConfiguracionAgente {
    std::string rutaROM = "pokemon_emerald.gba";
    std::string modeloPath = "pokemon_dqn_model.pt";
    int offsetX = 0;
    int offsetY = 0;
    int instanciaID = 0;
    
    float gamma = 0.99f;
    float epsilonStart = 1.0f;
    float epsilonEnd = 0.01f;
    float epsilonDecay = 0.995f;
    size_t batchSize = 32;
    size_t replayBufferSize = 10000;
    int targetUpdate = 100;
    
    int framesPorAccion = 4;
    int renderCadaNFrames = 15;
    int entrenarCadaNPasos = 4;
};

class PokemonRLAgent {
public:
    PokemonRLAgent(const ConfiguracionAgente& config);
    ~PokemonRLAgent();
    
    bool inicializar();
    void entrenar();
    void guardarModelo();
    void cargarModelo();
    
private:
    struct mCore* core;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* gameTexture;
    TTF_Font* font;
    TTF_Font* fontSmall;
    
    static constexpr int GBA_WIDTH = 240;
    static constexpr int GBA_HEIGHT = 160;
    static constexpr int SCALE = 3;
    static constexpr int STATS_WIDTH = 450;
    static constexpr int MAX_HISTORIAL = 10;
    
    std::vector<uint32_t> videoBuffer;
    ConfiguracionAgente config;
    
    std::shared_ptr<MemoriaJuego> memoria;
    std::shared_ptr<SistemaRecompensas> sistemaRecompensas;
    
    DQN qNetwork;
    DQN targetNetwork;
    std::unique_ptr<torch::optim::Adam> optimizer;
    
    std::deque<Experience> replayBuffer;
    
    float epsilon;
    int posXAnterior;
    int posYAnterior;
    bool ejecutando;
    std::mt19937 rng;
    
    struct Estadisticas {
        int episodio = 0;
        int pasosTotales = 0;
        int pasosEpisodio = 0;
        float recompensaTotal = 0.0f;
        float recompensaEpisodio = 0.0f;
        float perdidaPromedio = 0.0f;
        int exploraciones = 0;
        int explotaciones = 0;
        std::chrono::steady_clock::time_point inicioTiempo;
        std::vector<float> historicoRecompensas;
        
        int medallasActuales = 0;
        int pokemonCapturados = 0;
        int dineroActual = 0;
        std::string zonaActual = "";
        int posicionesExploradas = 0;
        int framesConocidos = 0;
    } stats;
    
    std::deque<MovimientoHistorial> historialMovimientos;
    
    const std::vector<int> ACCIONES = {
        0, GBA_KEY_A, GBA_KEY_B, GBA_KEY_UP, 
        GBA_KEY_DOWN, GBA_KEY_LEFT, GBA_KEY_RIGHT 
    };
    
    const std::vector<std::string> NOMBRES_ACCIONES = {
        "NADA", "A", "B", "UP", "DOWN", "LEFT", "RIGHT"
    };
    
    torch::Tensor obtenerEstado();
    int seleccionarAccion(const torch::Tensor& estado);
    void ejecutarAccion(int indiceAccion, int frames);
    std::pair<std::string, float> calcularRecompensa();
    void guardarExperiencia(const torch::Tensor& estado, int accion, 
                           float recompensa, const torch::Tensor& siguienteEstado,
                           bool terminado);
    void entrenarModelo();
    void actualizarRedObjetivo();
    
    std::pair<int, int> obtenerPosicion();
    void pasarIntro();
    bool puedeMoverse();
    void actualizarEstadisticasJuego();
    
    void renderizar();
    void renderizarEstadisticas();
    void renderizarTexto(const std::string& texto, int x, int y, 
                        SDL_Color color, TTF_Font* fuente = nullptr);
    void renderizarHistorial();
    bool procesarEventos();
    
    std::string formatearTiempo(int segundos);
    void agregarMovimientoHistorial(const std::string& descripcion, float recompensa);
};

#endif // POKEMON_H
