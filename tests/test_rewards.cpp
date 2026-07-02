

#include <gtest/gtest.h>
#include "SistemaRecompensas.h"
#include "MemoriaJuego.h"

namespace {
class SistemaRecompensasTest : public ::testing::Test {
protected:
    void SetUp() override {
        memoria = std::make_shared<MemoriaJuego>();
        sistema = std::make_unique<SistemaRecompensas>(memoria);
    }

    std::shared_ptr<MemoriaJuego> memoria;
    std::unique_ptr<SistemaRecompensas> sistema;
};

}


TEST_F(SistemaRecompensasTest, NuevoMapaOtorgaRecompensaBase) {
    InfoMapa mapa{5, 0, 100, 200, "Route 101"};
    auto [desc, recompensa] = sistema->recompensaExploracion(mapa);

    EXPECT_FLOAT_EQ(recompensa, RecompensasBase::NUEVO_MAPA);
    EXPECT_EQ(desc, "New map");
}

TEST_F(SistemaRecompensasTest, EntrarAGimnasioSumaBonusDeZona) {
    InfoMapa mapa{30, 0, 0, 0, "Rustboro Gym"};
    auto [desc, recompensa] = sistema->recompensaExploracion(mapa);

    EXPECT_FLOAT_EQ(recompensa,
                     RecompensasBase::NUEVO_MAPA + RecompensasBase::ZONA_GIMNASIO);
    EXPECT_EQ(desc, "New map Gym");
}

TEST_F(SistemaRecompensasTest, EntrarALaLigaSumaBonusMayor) {
    InfoMapa mapa{40, 0, 0, 0, "Pokemon League - Elite Four"};
    auto [desc, recompensa] = sistema->recompensaExploracion(mapa);

    EXPECT_FLOAT_EQ(recompensa,
                     RecompensasBase::NUEVO_MAPA + RecompensasBase::ZONA_LIGA);
    EXPECT_EQ(desc, "New map League");
}

TEST_F(SistemaRecompensasTest, PermanecerEnElMismoMapaNoRepiteRecompensa) {
    InfoMapa mapa{5, 0, 100, 200, "Route 101"};

    auto primera = sistema->recompensaExploracion(mapa);
    EXPECT_GT(primera.second, 0.0f);

    // no pay again for the same map (maybe)
    auto segunda = sistema->recompensaExploracion(mapa);
    EXPECT_FLOAT_EQ(segunda.second, 0.0f);
    EXPECT_EQ(segunda.first, "");
}


TEST_F(SistemaRecompensasTest, EquipoDeSeisOtorgaBonusEquipoCompleto) {
    std::vector<InfoPokemon> equipo(6, InfoPokemon{1, 20, 20, 5, 0});

    auto [desc, recompensa] = sistema->recompensaNivel(equipo);

    EXPECT_FLOAT_EQ(recompensa, Hitos::EQUIPO_COMPLETO * 0.001f);
    EXPECT_EQ(desc, "FullTeam");
}

TEST_F(SistemaRecompensasTest, SubirDeNivelOtorgaRecompensaProporcional) {
    std::vector<InfoPokemon> equipoInicial = {InfoPokemon{1, 20, 20, 5, 0}};
    memoria->actualizarEstadoAnterior(0, 0, equipoInicial, false);

    std::vector<InfoPokemon> equipoNuevo = {InfoPokemon{1, 30, 30, 8, 0}}; 

    auto [desc, recompensa] = sistema->recompensaNivel(equipoNuevo);

    EXPECT_FLOAT_EQ(recompensa, RecompensasBase::SUBIR_NIVEL * 3);
}

TEST_F(SistemaRecompensasTest, EvolucionarOtorgaHitoDeEvolucion) {
    std::vector<InfoPokemon> equipoInicial = {InfoPokemon{/*species=*/255, 20, 20, 16, 0}};
    memoria->actualizarEstadoAnterior(0, 0, equipoInicial, false);


    std::vector<InfoPokemon> equipoEvolucionado = {InfoPokemon{/*species=*/256, 40, 40, 16, 0}};

    auto [desc, recompensa] = sistema->recompensaNivel(equipoEvolucionado);

    EXPECT_GE(recompensa, Hitos::POKEMON_EVOLUCIONADO);
    EXPECT_EQ(desc, "Evo");
}


TEST_F(SistemaRecompensasTest, SinHistorialSuficienteNoHayPenalizacion) {
    InfoMapa mapa{5, 0, 50, 50, "Route 101"};
    auto [desc, penalizacion] = sistema->penalizacionRetroceso(mapa);

    EXPECT_FLOAT_EQ(penalizacion, 0.0f);
}

TEST_F(SistemaRecompensasTest, QuedarseEnLaMismaCasillaActivaPenalizacionMaxima) {
    InfoMapa mapa{5, 0, 50, 50, "Route 101"};

    std::pair<std::string, float> resultado;
    for (int i = 0; i < 15001; ++i) {
        resultado = sistema->penalizacionRetroceso(mapa);
    }

    EXPECT_FLOAT_EQ(resultado.second, -5.0f);
    EXPECT_EQ(resultado.first, "Retroceso");
}


TEST_F(SistemaRecompensasTest, RecompensaAcumuladaArrancaEnCero) {
    EXPECT_FLOAT_EQ(sistema->getRecompensaAcumulada(), 0.0f);
}

TEST_F(SistemaRecompensasTest, ResetearVuelveAPonerAcumuladoEnCero) {
    sistema->resetear();
    EXPECT_FLOAT_EQ(sistema->getRecompensaAcumulada(), 0.0f);
}

TEST_F(SistemaRecompensasTest, ObtenerDesgloseIncluyeRecompensaAcumulada) {
    std::string desglose = sistema->obtenerDesglose();
    EXPECT_NE(desglose.find("Accumulated reward:"), std::string::npos);
}