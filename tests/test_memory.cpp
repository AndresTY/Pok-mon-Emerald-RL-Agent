
#include <gtest/gtest.h>
#include "MemoriaJuego.h"

// ── position tests ────────────────────────────────


TEST(MemoriaJuego, PosicionNuevaSeDetectaLaPrimeraVez) {
    MemoriaJuego memoria;
    EXPECT_TRUE(memoria.hayNuevaPosicion(10, 20, 1));
}

TEST(MemoriaJuego, PosicionRegistradaYaNoEsNueva) {
    MemoriaJuego memoria;
    memoria.registrarPosicion(10, 20, 1);
    EXPECT_FALSE(memoria.hayNuevaPosicion(10, 20, 1));
}

TEST(MemoriaJuego, MismaCoordenadaEnMapaDistintoEsNueva) {
    MemoriaJuego memoria;
    memoria.registrarPosicion(10, 20, 1);
    // Mismo x,y pero mapaID distinto -> debe considerarse una posición nueva
    EXPECT_TRUE(memoria.hayNuevaPosicion(10, 20, 2));
}

TEST(MemoriaJuego, ContadorDePosicionesVisitadasCrece) {
    MemoriaJuego memoria;
    EXPECT_EQ(memoria.getTotalPosicionesVisitadas(), 0);
    memoria.registrarPosicion(0, 0, 0);
    memoria.registrarPosicion(1, 1, 0);
    memoria.registrarPosicion(1, 1, 0); // duplicado, no debe sumar de nuevo
    EXPECT_EQ(memoria.getTotalPosicionesVisitadas(), 2);
}

// ── Map tests ───────────────────────────────

TEST(MemoriaJuego, PrimerMapaCuentaComoCambio) {
    MemoriaJuego memoria; // mapaAnterior arranca con mapaID = 0
    InfoMapa actual{5, 0, 100, 200, "Rustboro City"};
    EXPECT_TRUE(memoria.hayCambioDeMapa(actual));
}

TEST(MemoriaJuego, MismoMapaNoCuentaComoCambioTrasActualizar) {
    MemoriaJuego memoria;
    InfoMapa actual{5, 0, 100, 200, "Rustboro City"};
    memoria.actualizarHistorial(actual);
    EXPECT_FALSE(memoria.hayCambioDeMapa(actual));
}

TEST(MemoriaJuego, CambiarDeMapaVuelveADetectarse) {
    MemoriaJuego memoria;
    InfoMapa mapaA{5, 0, 0, 0, "Rustboro City"};
    InfoMapa mapaB{6, 0, 0, 0, "Dewford Town"};
    memoria.actualizarHistorial(mapaA);
    EXPECT_TRUE(memoria.hayCambioDeMapa(mapaB));
}

// ── Hash de frames ──────────────────────────

TEST(MemoriaJuego, FrameNuevoAntesDeRegistrarlo) {
    MemoriaJuego memoria;
    std::vector<uint32_t> buffer(64, 0xAABBCCDD);
    FrameHash hash = memoria.calcularHashFrame(buffer);
    EXPECT_TRUE(memoria.esFrameNuevo(hash));
}

TEST(MemoriaJuego, FrameYaNoEsNuevoTrasRegistrarlo) {
    MemoriaJuego memoria;
    std::vector<uint32_t> buffer(64, 0xAABBCCDD);
    FrameHash hash = memoria.calcularHashFrame(buffer);
    memoria.registrarFrame(hash);
    EXPECT_FALSE(memoria.esFrameNuevo(hash));
    EXPECT_EQ(memoria.getTotalFramesConocidos(), 1);
}

TEST(MemoriaJuego, BuffersDistintosProducenHashesDistintos) {
    MemoriaJuego memoria;
    std::vector<uint32_t> bufferA(64, 0x11111111);
    std::vector<uint32_t> bufferB(64, 0x22222222);
    FrameHash hashA = memoria.calcularHashFrame(bufferA);
    FrameHash hashB = memoria.calcularHashFrame(bufferB);
    EXPECT_FALSE(hashA == hashB);
}

TEST(MemoriaJuego, BufferVacioNoRevientaElHash) {
    MemoriaJuego memoria;
    std::vector<uint32_t> vacio;
    EXPECT_NO_THROW({
        FrameHash hash = memoria.calcularHashFrame(vacio);
        (void)hash;
    });
}


TEST(MemoriaJuego, DialogoEsNuevoLaPrimeraVez) {
    MemoriaJuego memoria;
    EXPECT_TRUE(memoria.esDialogoNuevo(0xDEADBEEF));
}

TEST(MemoriaJuego, DialogoRegistradoYaNoEsNuevo) {
    MemoriaJuego memoria;
    memoria.registrarDialogo(0xDEADBEEF);
    EXPECT_FALSE(memoria.esDialogoNuevo(0xDEADBEEF));
}


TEST(MemoriaJuego, ProgresoInicialEsCero) {
    MemoriaJuego memoria;
    EXPECT_FLOAT_EQ(memoria.calcularProgreso(), 0.0f);
}

TEST(MemoriaJuego, ProgresoAumentaConMedallas) {
    MemoriaJuego memoria;
    memoria.actualizarEstadoAnterior(/*dinero=*/0, /*medallas=*/4, {}, /*enBatalla=*/false);
    EXPECT_NEAR(memoria.calcularProgreso(), 0.3f, 1e-4f);
}

TEST(MemoriaJuego, ActualizarEstadoAnteriorPersisteValores) {
    MemoriaJuego memoria;
    std::vector<InfoPokemon> equipo = {{1, 50, 60, 5, 0}};
    memoria.actualizarEstadoAnterior(/*dinero=*/1500, /*medallas=*/2, equipo, /*enBatalla=*/true);

    EXPECT_EQ(memoria.getDineroAnterior(), 1500);
    EXPECT_EQ(memoria.getMedallasAnteriores(), 2);
    EXPECT_TRUE(memoria.getBatallaAnterior());
    ASSERT_EQ(memoria.getEquipoAnterior().size(), 1u);
    EXPECT_EQ(memoria.getEquipoAnterior()[0].level, 5);
}

TEST(MemoriaJuego, RegistrarPokemonCapturadoIncrementaContador) {
    MemoriaJuego memoria;
    EXPECT_EQ(memoria.getTotalPokemonCapturados(), 0);
    memoria.registrarPokemonCapturado();
    memoria.registrarPokemonCapturado();
    EXPECT_EQ(memoria.getTotalPokemonCapturados(), 2);
}

TEST(MemoriaJuego, RegistrarBatallaGanadaIncrementaContador) {
    MemoriaJuego memoria;
    EXPECT_EQ(memoria.getTotalBatallasGanadas(), 0);
    memoria.registrarBatallaGanada();
    EXPECT_EQ(memoria.getTotalBatallasGanadas(), 1);
}


TEST(MemoriaJuego, HaCompletadoEventoSiempreFalseHoy) {
    MemoriaJuego memoria;
    EXPECT_FALSE(memoria.haCompletadoEvento(0x82E)); // Sidney
    EXPECT_FALSE(memoria.haCompletadoEvento(0x832)); // Campeón
}