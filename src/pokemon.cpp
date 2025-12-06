 #include "pokemon.h"
#include "SistemaRecompensas.h"
#include <mgba/gba/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/gba/interface.h>
#include <mgba-util/vfs.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <utility>

DQNImpl::DQNImpl() {
    conv1 = register_module("conv1", torch::nn::Conv2d(
        torch::nn::Conv2dOptions(3, 32, 8).stride(4)));
    conv2 = register_module("conv2", torch::nn::Conv2d(
        torch::nn::Conv2dOptions(32, 64, 4).stride(2)));
    conv3 = register_module("conv3", torch::nn::Conv2d(
        torch::nn::Conv2dOptions(64, 64, 3).stride(1)));
    fc1 = register_module("fc1", torch::nn::Linear(3136, 512));
    fc2 = register_module("fc2", torch::nn::Linear(512, 7));
}

torch::Tensor DQNImpl::forward(torch::Tensor x) {
    x = torch::relu(conv1->forward(x));
    x = torch::relu(conv2->forward(x));
    x = torch::relu(conv3->forward(x));
    x = x.view({x.size(0), -1});
    x = torch::relu(fc1->forward(x));
    return fc2->forward(x);
}

PokemonRLAgent::PokemonRLAgent(const ConfiguracionAgente& cfg)
    : core(nullptr), window(nullptr), renderer(nullptr), 
      gameTexture(nullptr), font(nullptr), fontSmall(nullptr),
      config(cfg), qNetwork(), targetNetwork(),
      epsilon(cfg.epsilonStart), posXAnterior(0), posYAnterior(0),
      ejecutando(true), rng(std::random_device{}()) {
    
    videoBuffer.resize(GBA_WIDTH * GBA_HEIGHT);
    
    memoria = std::make_shared<MemoriaJuego>();
    sistemaRecompensas = std::make_shared<SistemaRecompensas>(memoria);
    
    optimizer = std::make_unique<torch::optim::Adam>(
        qNetwork->parameters(), torch::optim::AdamOptions(1e-4));
    
    torch::NoGradGuard no_grad;
    for (size_t i = 0; i < qNetwork->parameters().size(); i++) {
        targetNetwork->parameters()[i].copy_(qNetwork->parameters()[i]);
    }
}

PokemonRLAgent::~PokemonRLAgent() {
    if (font) TTF_CloseFont(font);
    if (fontSmall) TTF_CloseFont(fontSmall);
    if (gameTexture) SDL_DestroyTexture(gameTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (core) core->deinit(core);
    TTF_Quit();
    SDL_Quit();
}

bool PokemonRLAgent::inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (TTF_Init() < 0) {
        std::cerr << "Error TTF: " << TTF_GetError() << std::endl;
        return false;
    }
    
    std::string titulo = "Pokémon RL Agent #" + std::to_string(config.instanciaID);
    window = SDL_CreateWindow(
        titulo.c_str(),
        SDL_WINDOWPOS_CENTERED + config.offsetX,
        SDL_WINDOWPOS_CENTERED + config.offsetY,
        GBA_WIDTH * SCALE + STATS_WIDTH,
        GBA_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) return false;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;
    
    gameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    GBA_WIDTH, GBA_HEIGHT);
    if (!gameTexture) return false;
    
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16);
    fontSmall = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12);
    
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/Adwaita/AdwaitaMono-Bold.ttf", 16);
        fontSmall = TTF_OpenFont("/usr/share/fonts/Adwaita/AdwaitaSans-Regular.ttf", 12);
    }
    
    if (!font || !fontSmall) {
        std::cout << "Fonts Error..." << std::endl;
    }
    
    core = GBACoreCreate();
    if (!core) return false;
    
    core->init(core);
    mCoreInitConfig(core, nullptr);
    
    if (!mCoreLoadFile(core, config.rutaROM.c_str())) {
        std::cerr << "Error ROM..." << config.rutaROM << std::endl;
        return false;
    }
    
    core->setVideoBuffer(core, reinterpret_cast<uint32_t*>(videoBuffer.data()), GBA_WIDTH);
    core->reset(core);
    
    stats.inicioTiempo = std::chrono::steady_clock::now();
    
    cargarModelo();
    
    return true;
}

torch::Tensor PokemonRLAgent::obtenerEstado() {
    const int ESTADO_WIDTH = 84;
    const int ESTADO_HEIGHT = 84;
    
    std::vector<float> pixels;
    pixels.reserve(ESTADO_WIDTH * ESTADO_HEIGHT * 3);
    
    float stepX = static_cast<float>(GBA_WIDTH) / ESTADO_WIDTH;
    float stepY = static_cast<float>(GBA_HEIGHT) / ESTADO_HEIGHT;
    
    for (int y = 0; y < ESTADO_HEIGHT; y++) {
        for (int x = 0; x < ESTADO_WIDTH; x++) {
            int srcX = static_cast<int>(x * stepX);
            int srcY = static_cast<int>(y * stepY);
            int idx = srcY * GBA_WIDTH + srcX;
            
            uint32_t pixel = videoBuffer[idx];
            pixels.push_back(((pixel >> 16) & 0xFF) / 255.0f);
            pixels.push_back(((pixel >> 8) & 0xFF) / 255.0f);
            pixels.push_back((pixel & 0xFF) / 255.0f);
        }
    }
    
    return torch::from_blob(pixels.data(), 
                           {1, 3, ESTADO_HEIGHT, ESTADO_WIDTH},
                           torch::kFloat32).clone();
}

int PokemonRLAgent::seleccionarAccion(const torch::Tensor& estado) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    if (dist(rng) < epsilon) {
        stats.exploraciones++;
        std::uniform_int_distribution<int> accionDist(0, ACCIONES.size() - 1);
        return accionDist(rng);
    } else {
        stats.explotaciones++;
        torch::NoGradGuard no_grad;
        auto qValues = qNetwork->forward(estado);
        return qValues.argmax(1).item<int>();
    }
}

void PokemonRLAgent::ejecutarAccion(int indiceAccion, int frames) {
    int boton = ACCIONES[indiceAccion];
    
    for (int i = 0; i < frames; i++) {
        if (boton != 0) {
            core->setKeys(core, boton);
        }
        core->runFrame(core);
        stats.pasosTotales++;
        stats.pasosEpisodio++;
        
        if (stats.pasosTotales % config.renderCadaNFrames == 0) {
            renderizar();
            procesarEventos();
        }
    }
    
    if (boton != 0) {
        core->clearKeys(core, boton);
    }
}

std::pair<std::string, float> PokemonRLAgent::calcularRecompensa() {
    auto frameHash = memoria->calcularHashFrame(videoBuffer);
    float recompensaFrame = 0.0f;
    
    if (memoria->esFrameNuevo(frameHash)) {
        memoria->registrarFrame(frameHash);
        recompensaFrame = 0.01f;
    }
    
    auto aux = sistemaRecompensas->calcularRecompensa(core);
    float recompensa =aux.second;
    recompensa += recompensaFrame;
    
    return {aux.first,recompensa};
}

void PokemonRLAgent::guardarExperiencia(const torch::Tensor& estado, int accion,
                                        float recompensa, const torch::Tensor& siguienteEstado,
                                        bool terminado) {
    if (replayBuffer.size() >= config.replayBufferSize) {
        replayBuffer.pop_front();
    }
    replayBuffer.push_back({estado.clone(), accion, recompensa, 
                           siguienteEstado.clone(), terminado});
}

void PokemonRLAgent::entrenarModelo() {
    if (replayBuffer.size() < config.batchSize) return;
    
    std::vector<Experience> batch;
    std::uniform_int_distribution<size_t> dist(0, replayBuffer.size() - 1);
    
    for (size_t i = 0; i < config.batchSize; i++) {
        batch.push_back(replayBuffer[dist(rng)]);
    }
    
    std::vector<torch::Tensor> estados, siguientesEstados;
    std::vector<int64_t> acciones;
    std::vector<float> recompensas, noTerminados;
    
    for (const auto& exp : batch) {
        estados.push_back(exp.estado);
        siguientesEstados.push_back(exp.siguienteEstado);
        acciones.push_back(exp.accion);
        recompensas.push_back(exp.recompensa);
        noTerminados.push_back(exp.terminado ? 0.0f : 1.0f);
    }
    
    auto estadosBatch = torch::cat(estados, 0);
    auto siguientesEstadosBatch = torch::cat(siguientesEstados, 0);
    auto accionesTensor = torch::tensor(acciones, torch::kLong).unsqueeze(1);
    auto recompensasTensor = torch::tensor(recompensas, torch::kFloat32);
    auto noTerminadosTensor = torch::tensor(noTerminados, torch::kFloat32);
    
    auto qValues = qNetwork->forward(estadosBatch).gather(1, accionesTensor);
    
    torch::Tensor nextQValues;
    {
        torch::NoGradGuard no_grad;
        nextQValues = std::get<0>(targetNetwork->forward(siguientesEstadosBatch).max(1));
    }
    
    auto targetQValues = recompensasTensor + config.gamma * nextQValues * noTerminadosTensor;
    auto loss = torch::mse_loss(qValues.squeeze(), targetQValues);
    
    optimizer->zero_grad();
    loss.backward();
    torch::nn::utils::clip_grad_norm_(qNetwork->parameters(), 1.0);
    optimizer->step();
    
    stats.perdidaPromedio = loss.item<float>();
}

void PokemonRLAgent::actualizarRedObjetivo() {
    torch::NoGradGuard no_grad;
    for (size_t i = 0; i < qNetwork->parameters().size(); i++) {
        targetNetwork->parameters()[i].copy_(qNetwork->parameters()[i]);
    }
}

std::pair<int, int> PokemonRLAgent::obtenerPosicion() {
    uint8_t x = core->rawRead8(core, DireccionesMemoria::POSICION_X, -1);
    uint8_t y = core->rawRead8(core, DireccionesMemoria::POSICION_Y, -1);
    return {x, y};
}

bool PokemonRLAgent::puedeMoverse() {
    auto [x, y] = obtenerPosicion();
    return (x > 0 || y > 0);
}

void PokemonRLAgent::pasarIntro() {
    int intentos = 0;
    const int maxIntentos = 1000;
    
    while (!puedeMoverse() && intentos < maxIntentos && ejecutando) {
        core->setKeys(core, GBA_KEY_A);
        for (int i = 0; i < 3; i++) {
            core->runFrame(core);
        }
        core->clearKeys(core, GBA_KEY_A);
        core->runFrame(core);
        
        if (intentos % 50 == 0) {
            core->setKeys(core, GBA_KEY_START);
            core->runFrame(core);
            core->clearKeys(core, GBA_KEY_START);
            
            if (intentos % 100 == 0) {
                renderizar();
            }
            procesarEventos();
        }
        
        intentos++;
    }
    
    auto [x, y] = obtenerPosicion();
    posXAnterior = x;
    posYAnterior = y;
    
    for (int i = 0; i < 60; i++) {
        core->runFrame(core);
    }
    
}

void PokemonRLAgent::actualizarEstadisticasJuego() {
    auto mapa = memoria->leerMapaActual(core);
    stats.medallasActuales = memoria->contarMedallas(core);
    stats.dineroActual = memoria->leerDinero(core);
    stats.zonaActual = mapa.nombreZona;
    stats.posicionesExploradas = memoria->getTotalPosicionesVisitadas();
    stats.framesConocidos = memoria->getTotalFramesConocidos();
}

bool PokemonRLAgent::procesarEventos() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            ejecutando = false;
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                ejecutando = false;
                return false;
            }
            // R - Reset Game
            if (event.key.keysym.sym == SDLK_r) {
                core->reset(core);
                pasarIntro();
                return true;
            }
        }
    }
    return true;
}

void PokemonRLAgent::renderizar() {
    SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
    SDL_RenderClear(renderer);
    
    SDL_UpdateTexture(gameTexture, nullptr, videoBuffer.data(), 
                     GBA_WIDTH * sizeof(uint32_t));
    SDL_Rect gameRect = {0, 0, GBA_WIDTH * SCALE, GBA_HEIGHT * SCALE};
    SDL_RenderCopy(renderer, gameTexture, nullptr, &gameRect);
    
    renderizarEstadisticas();
    
    SDL_RenderPresent(renderer);
}

void PokemonRLAgent::renderizarEstadisticas() {
    SDL_Rect statsArea = {GBA_WIDTH * SCALE, 0, STATS_WIDTH, GBA_HEIGHT * SCALE};
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderFillRect(renderer, &statsArea);
    
    actualizarEstadisticasJuego();
    
    auto [x, y] = obtenerPosicion();
    auto ahora = std::chrono::steady_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::seconds>(
        ahora - stats.inicioTiempo);
    int tiempoJugado = duracion.count();
    
    int yPos = 10;
    int spacing = 22;
    SDL_Color blanco = {255, 255, 255, 255};
    SDL_Color verde = {100, 255, 100, 255};
    SDL_Color amarillo = {255, 255, 100, 255};
    SDL_Color cyan = {100, 200, 255, 255};
    
    renderizarTexto("=== AGENT #" + std::to_string(config.instanciaID) + " ===", 
                   GBA_WIDTH * SCALE + 10, yPos, cyan, font);
    yPos += spacing * 1.5;
    
    renderizarTexto("Epoch: " + std::to_string(stats.episodio), 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Step: " + std::to_string(stats.pasosEpisodio),
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Time: " + formatearTiempo(tiempoJugado), 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing * 1.5;
    
    renderizarTexto("=== GAME STATE ===", 
                   GBA_WIDTH * SCALE + 10, yPos, verde, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Zone: " + stats.zonaActual, 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Pos: (" + std::to_string(x) + ", " + std::to_string(y) + ")", 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing;
    
    SDL_Color colorMedallas = stats.medallasActuales > 0 ? amarillo : blanco;
    renderizarTexto("Badges: " + std::to_string(stats.medallasActuales) + "/8", 
                   GBA_WIDTH * SCALE + 10, yPos, colorMedallas, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Money: $" + std::to_string(stats.dineroActual), 
                   GBA_WIDTH * SCALE + 10, yPos, verde, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Positions: " + std::to_string(stats.posicionesExploradas), 
                   GBA_WIDTH * SCALE + 10, yPos, cyan, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Unique Frames: " + std::to_string(stats.framesConocidos), 
                   GBA_WIDTH * SCALE + 10, yPos, cyan, fontSmall);
    yPos += spacing * 1.5;
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << epsilon;
    renderizarTexto("Epsilon: " + ss.str(), 
                   GBA_WIDTH * SCALE + 10, yPos, amarillo, fontSmall);
    yPos += spacing;
    
    ss.str("");
    ss << std::fixed << std::setprecision(2) << stats.recompensaEpisodio;
    renderizarTexto("Reward: " + ss.str(), 
                   GBA_WIDTH * SCALE + 10, yPos, verde, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Explore: " + std::to_string(stats.exploraciones), 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing;
    
    renderizarTexto("Exploit: " + std::to_string(stats.explotaciones), 
                   GBA_WIDTH * SCALE + 10, yPos, blanco, fontSmall);
    yPos += spacing * 1.5;
    
    renderizarHistorial();
}
void PokemonRLAgent::renderizarHistorial() {
    int yPos = 120;
    SDL_Color cyan = {100, 200, 255, 255};
    SDL_Color verde = {100, 255, 100, 255};
    SDL_Color rojo = {255, 100, 100, 255};
    
    renderizarTexto("=== REWARDS ===", GBA_WIDTH * SCALE + 150, yPos, cyan, fontSmall);
    yPos += 20;
    
    int count = 0;
    for (auto it = historialMovimientos.rbegin(); 
         it != historialMovimientos.rend() && count < MAX_HISTORIAL; ++it, ++count) {
        
        if(it->recompensa != 0){ 
        SDL_Color color = it->recompensa >= 0 ? verde : rojo;
        
        std::stringstream ss;
        ss << it->descripcion << " " 
           << std::fixed << std::setprecision(1) << std::showpos << it->recompensa;
        renderizarTexto(ss.str(), GBA_WIDTH * SCALE + 150, yPos, color, fontSmall);
        yPos += 18;
         }
    }
}

void PokemonRLAgent::renderizarTexto(const std::string& texto, int x, int y,
                                    SDL_Color color, TTF_Font* fuente) {
    if (!fuente) fuente = fontSmall;
    if (!fuente) return;
    
    SDL_Surface* surface = TTF_RenderText_Blended(fuente, texto.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dstRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

std::string PokemonRLAgent::formatearTiempo(int segundos) {
    int horas = segundos / 3600;
    int minutos = (segundos % 3600) / 60;
    int segs = segundos % 60;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << horas << ":"
       << std::setfill('0') << std::setw(2) << minutos << ":"
       << std::setfill('0') << std::setw(2) << segs;
    return ss.str();
}

void PokemonRLAgent::agregarMovimientoHistorial(const std::string& descripcion, 
                                               float recompensa) {
    if (historialMovimientos.size() >= MAX_HISTORIAL) {
        historialMovimientos.pop_front();
    }
    historialMovimientos.push_back({descripcion, recompensa, stats.pasosTotales});
}

void PokemonRLAgent::guardarModelo() {
    try {
        std::string modelPath = "pokemon_dqn_model_" + 
                               std::to_string(config.instanciaID) + ".pt";
        torch::save(qNetwork, modelPath);
        
        // ✨ Guardar memoria del juego
        std::string memoryPath = "pokemon_memory_" + 
                                std::to_string(config.instanciaID) + ".bin";
        memoria->guardarEstado(memoryPath);
        
        std::string statsPath = "training_stats_" + 
                               std::to_string(config.instanciaID) + ".txt";
        std::ofstream stats_file(statsPath);
        stats_file << "Episodio: " << stats.episodio << "\n";
        stats_file << "Pasos totales: " << stats.pasosTotales << "\n";
        stats_file << "Recompensa total: " << stats.recompensaTotal << "\n";
        stats_file << "Epsilon: " << epsilon << "\n";
        stats_file << "Medallas: " << stats.medallasActuales << "\n";
        stats_file << "Dinero: " << stats.dineroActual << "\n";
        stats_file << "Posiciones exploradas: " << stats.posicionesExploradas << "\n";
        stats_file << "Frames únicos: " << stats.framesConocidos << "\n";
        stats_file.close();
        
    } catch (const std::exception& e) {
        std::cerr << "Model error..." << e.what() << std::endl;
    }
}

void PokemonRLAgent::cargarModelo() {
    try {
        std::string modelPath = "pokemon_dqn_model_" + 
                               std::to_string(config.instanciaID) + ".pt";
        
        if (std::ifstream(modelPath).good()) {
            torch::load(qNetwork, modelPath);
            
            torch::NoGradGuard no_grad;
            for (size_t i = 0; i < qNetwork->parameters().size(); i++) {
                targetNetwork->parameters()[i].copy_(qNetwork->parameters()[i]);
            }
            
            std::string memoryPath = "pokemon_memory_" + 
                                    std::to_string(config.instanciaID) + ".bin";
            if (std::ifstream(memoryPath).good()) {
                memoria->cargarEstado(memoryPath);
            }
            
            std::string statsPath = "training_stats_" + 
                                   std::to_string(config.instanciaID) + ".txt";
            std::ifstream stats_file(statsPath);
            if (stats_file.good()) {
                std::string line;
                while (std::getline(stats_file, line)) {
                    if (line.find("Epsilon:") != std::string::npos) {
                        epsilon = std::stof(line.substr(line.find(":") + 2));
                    }
                }
            }
            
        }
    } catch (const std::exception& e) {
        std::cout << "No Previous training" << std::endl;
    }
}

void PokemonRLAgent::entrenar() {
    pasarIntro();
    if (!ejecutando) return;
    
    while (ejecutando) {
        stats.episodio++;
        stats.pasosEpisodio = 0;
        stats.recompensaEpisodio = 0.0f;
        sistemaRecompensas->resetear();
        
        auto estado = obtenerEstado();
        
        while (ejecutando) {
            if (!procesarEventos()) {
                break;
            }
            
            int accion = seleccionarAccion(estado);
            ejecutarAccion(accion, config.framesPorAccion);
            
            auto siguienteEstado = obtenerEstado();
            std::pair<std::string, float> recompensa = calcularRecompensa();
            
            agregarMovimientoHistorial(recompensa.first, recompensa.second);
            
            stats.recompensaEpisodio += recompensa.second;
            stats.recompensaTotal += recompensa.second;
            
            guardarExperiencia(estado, accion, recompensa.second, siguienteEstado, false);
            
            if (stats.pasosTotales % config.entrenarCadaNPasos == 0) {
                entrenarModelo();
            }
            
            if (stats.pasosTotales % config.targetUpdate == 0) {
                actualizarRedObjetivo();
            }
            
            estado = siguienteEstado;
        }
        
        epsilon = std::max(config.epsilonEnd, epsilon * config.epsilonDecay);
        stats.historicoRecompensas.push_back(stats.recompensaEpisodio);
        
        std::cout << "   Reward: " << stats.recompensaEpisodio 
                 << " | Loss: " << stats.perdidaPromedio
                 << " | Zone: " << stats.zonaActual 
                 << " | Frames: " << stats.framesConocidos << std::endl;
        
        if (stats.episodio % 10 == 0){
                guardarModelo();

        }
        
        core->reset(core);
        pasarIntro();
}
guardarModelo();
}
