#ifndef EVENTOS_EMERALD_H
#define EVENTOS_EMERALD_H

#include <vector>
#include <string>
#include <cstdint>
#include "MemoriaJuego.h"

namespace EventosEmerald {
    
    inline std::vector<EventoHistoria> obtenerEventosHistoriaPrincipal() {
        return {
            {"Recibir Pokédex", "Oak te entrega el Pokédex", 0x800, 20.0f},
            {"Elegir starter", "Recibir primer Pokémon", 0x860, 50.0f},
            {"Derrotar a May/Brendan (Ruta 103)", "Primera batalla rival", 0x92, 15.0f},
            
            // Rustboro City
            {"Rescatar Devon Goods", "Recuperar paquete de Devon", 0x1E6, 25.0f},
            {"Entregar Devon Goods", "Devolver paquete a Devon Corp", 0x1E7, 15.0f},
            {"Obtener PokéNav", "Recibir PokéNav de Devon", 0x1E8, 20.0f},
            {"Derrotar Team Aqua/Magma (Rusturf)", "Primera batalla contra equipo villano", 0x20C, 30.0f},
            
            // Dewford Town
            {"Entregar carta a Steven", "Llevar carta de Devon a Steven", 0x227, 25.0f},
            {"Recibir HM05 Flash", "Obtener Flash en Granite Cave", 0x228, 15.0f},
            
            // Slateport City
            {"Detener a Team Aqua/Magma (Museo)", "Proteger las Devon Goods", 0x234, 40.0f},
            {"Recibir Devon Scope", "Obtener Devon Scope", 0x235, 20.0f},
            
            // Mauville City
            {"Ayudar a Wally", "Capturar Ralts con Wally", 0x65, 25.0f},
            {"Derrotar a Wally (Ruta 110)", "Batalla contra Wally", 0x93, 20.0f},
            
            // Mt. Chimney
            {"Detener a Team Aqua/Magma (Mt. Chimney)", "Batalla en la cima del volcán", 0x237, 50.0f},
            {"Recibir Meteorite", "Obtener Meteorite", 0x238, 15.0f},
            
            // Lavaridge Town
            {"Devolver Meteorite", "Entregar Meteorite en Fallarbor", 0x239, 20.0f},
            
            // Petalburg City
            {"Desafiar a Norman", "Enfrentar a tu padre (5ta medalla)", 0x80B, 75.0f},
            
            // Fortree City
            {"Encontrar a Steven (Ruta 120)", "Hablar con Steven sobre clima", 0x2B0, 20.0f},
            {"Recibir Devon Scope", "Usar Devon Scope en Kecleon", 0x2B1, 25.0f},
            {"Derrotar a Winona", "Obtener 6ta medalla", 0x80C, 100.0f},
            
            // Lilycove City
            {"Infiltrarse en Aqua/Magma Hideout", "Entrar a escondite villano", 0x2E0, 40.0f},
            {"Derrotar líder Aqua/Magma", "Batalla contra Maxie/Archie", 0x2E1, 60.0f},
            {"Obtener Master Ball", "Recibir Master Ball", 0x2E2, 30.0f},
            
            // Space Center
            {"Detener robo en Space Center", "Proteger el cohete", 0x300, 45.0f},
            
            // Mossdeep City
            {"Ayudar a Steven vs Team Aqua/Magma", "Batalla doble en Space Center", 0x310, 55.0f},
            
            // Seafloor Cavern
            {"Llegar a Seafloor Cavern", "Descubrir la caverna submarina", 0x320, 40.0f},
            {"Despertar a Kyogre/Groudon", "Presenciar el despertar legendario", 0x321, 70.0f},
            
            // Sootopolis City
            {"Presenciar batalla Kyogre vs Groudon", "Evento climático catastrófico", 0x330, 50.0f},
            {"Despertar a Rayquaza", "Subir a Sky Pillar", 0x331, 80.0f},
            {"Calmar a los legendarios", "Rayquaza detiene la batalla", 0x332, 100.0f},
            {"Derrotar a Wallace", "Obtener 8va medalla", 0x80E, 120.0f},
            
            // Victory Road & Elite Four
            {"Llegar a Ever Grande City", "Alcanzar la Liga Pokémon", 0x840, 50.0f},
            {"Completar Victory Road", "Atravesar Victory Road", 0x841, 60.0f},
            {"Derrotar a Sidney", "Vencer al primer Elite Four", 0x82E, 150.0f},
            {"Derrotar a Phoebe", "Vencer al segundo Elite Four", 0x82F, 150.0f},
            {"Derrotar a Glacia", "Vencer al tercer Elite Four", 0x830, 150.0f},
            {"Derrotar a Drake", "Vencer al cuarto Elite Four", 0x831, 150.0f},
            {"Derrotar a Steven", "¡Convertirse en Campeón!", 0x832, 500.0f},
            
            // Post-game
            {"Registrarse en Battle Frontier", "Acceder a Battle Frontier", 0x850, 100.0f},
            {"Primera victoria en Battle Tower", "Ganar streak en Battle Tower", 0x851, 50.0f},
        };
    }
    
    
    inline std::vector<EventoHistoria> obtenerEventosSecundarios() {
        return {
            // MOs 
            {"Obtener HM01 Cut", "Recibir Cut en Rustboro", 0x1E9, 25.0f},
            {"Obtener HM03 Surf", "Recibir Surf de Wally's father", 0x240, 40.0f},
            {"Obtener HM04 Strength", "Recibir Strength", 0x241, 25.0f},
            {"Obtener HM06 Rock Smash", "Recibir Rock Smash", 0x1EA, 20.0f},
            {"Obtener HM07 Waterfall", "Recibir Waterfall", 0x333, 35.0f},
            {"Obtener HM08 Dive", "Recibir Dive", 0x242, 40.0f},
            
            {"Intercambiar por Seedot/Ralts", "Primer intercambio in-game", 0x400, 15.0f},
            
            {"Encontrar a Regirock", "Acceder a cámara de Regirock", 0x8B8, 60.0f},
            {"Encontrar a Regice", "Acceder a cámara de Regice", 0x8B9, 60.0f},
            {"Encontrar a Registeel", "Acceder a cámara de Registeel", 0x8BA, 60.0f},
            {"Capturar Regirock", "Capturar a Regirock", 0x8BB, 80.0f},
            {"Capturar Regice", "Capturar a Regice", 0x8BC, 80.0f},
            {"Capturar Registeel", "Capturar a Registeel", 0x8BD, 80.0f},
            
            {"Encontrar a Latias/Latios", "Primer encuentro con Eon Pokémon", 0x8C0, 70.0f},
            {"Capturar Latias/Latios", "Capturar Eon Pokémon", 0x8C1, 100.0f},
            
            {"Encontrar a Rayquaza", "Subir a Sky Pillar", 0x8C5, 90.0f},
            {"Capturar Rayquaza", "Capturar a Rayquaza", 0x8C6, 120.0f},
            
            {"Ayudar al Weather Institute", "Rescatar Weather Institute", 0x2A0, 35.0f},
            {"Rescatar a Captain Stern", "Proteger a Stern en museo", 0x236, 30.0f},
            {"Ayudar a Lanette", "Mejorar el PC System", 0x2C0, 20.0f},
            
            {"Primer concurso Pokémon", "Participar en un concurso", 0x900, 15.0f},
            {"Ganar concurso Normal Rank", "Ganar concurso Normal", 0x901, 20.0f},
            {"Ganar concurso Super Rank", "Ganar concurso Super", 0x902, 30.0f},
            {"Ganar concurso Hyper Rank", "Ganar concurso Hyper", 0x903, 40.0f},
            {"Ganar concurso Master Rank", "Ganar concurso Master", 0x904, 60.0f},
            
            {"Primera visita a Safari Zone", "Entrar a Safari Zone", 0x500, 15.0f},
            
            {"Crear Secret Base", "Establecer tu base secreta", 0x600, 20.0f},
            {"Decorar Secret Base", "Añadir decoraciones", 0x601, 10.0f},
            
            {"Conocer al Berry Master", "Hablar con Berry Master", 0x700, 10.0f},
            {"Plantar primera Berry", "Cultivar tu primera baya", 0x701, 10.0f},
        };
    }
    
    inline std::vector<EventoHistoria> obtenerEventosPersonajes() {
        return {
            // Rival
            {"Batalla vs Rival (Ruta 103)", "Primera batalla", 0x92, 15.0f},
            {"Batalla vs Rival (Ruta 110)", "Segunda batalla", 0x93, 20.0f},
            {"Batalla vs Rival (Ruta 119)", "Tercera batalla", 0x94, 25.0f},
            {"Batalla vs Rival (Lilycove)", "Cuarta batalla", 0x95, 30.0f},
            
            // Wally
            {"Ayudar a Wally (Petalburg)", "Enseñar a capturar", 0x65, 20.0f},
            {"Batalla vs Wally (Ruta 110)", "Primera batalla vs Wally", 0x93, 20.0f},
            {"Batalla vs Wally (Victory Road)", "Batalla final vs Wally", 0x96, 50.0f},
            
            // Steven
            {"Conocer a Steven (Granite Cave)", "Primer encuentro", 0x220, 15.0f},
            {"Recibir carta de Steven", "Steven te pide ayuda", 0x221, 10.0f},
            {"Entregar carta a Steven", "Completar encargo", 0x227, 20.0f},
            {"Steven te da Beldum", "Recibir Beldum de regalo", 0x870, 80.0f},
            
            // Team Aqua/Magma
            {"Primer encuentro con Maxie/Archie", "Conocer al líder villano", 0x200, 20.0f},
            {"Batalla vs Admin Aqua/Magma", "Enfrentar a administrador", 0x201, 35.0f},
            {"Batalla final vs Maxie/Archie", "Batalla climática", 0x2E1, 70.0f},
            
            // Gym Leaders - Out Gym
            {"Conocer a Roxanne", "Hablar con Roxanne fuera del gym", 0x7F0, 5.0f},
            {"Conocer a Brawly", "Hablar con Brawly fuera del gym", 0x7F1, 5.0f},
            {"Conocer a Wattson", "Hablar con Wattson fuera del gym", 0x7F2, 5.0f},
            {"Conocer a Flannery", "Hablar con Flannery fuera del gym", 0x7F3, 5.0f},
            {"Conocer a Norman", "Tu padre es líder de gym", 0x7F4, 10.0f},
            {"Conocer a Winona", "Hablar con Winona fuera del gym", 0x7F5, 5.0f},
            {"Conocer a Tate & Liza", "Hablar con los gemelos", 0x7F6, 5.0f},
            {"Conocer a Wallace", "Hablar con Wallace fuera del gym", 0x7F7, 5.0f},
        };
    }
    
    inline std::vector<EventoHistoria> obtenerEventosPokedex() {
        return {
            {"Pokédex: 10 capturados", "Capturar 10 Pokémon diferentes", 0xFFFF, 20.0f},
            {"Pokédex: 25 capturados", "Capturar 25 Pokémon diferentes", 0xFFFF, 40.0f},
            {"Pokédex: 50 capturados", "Capturar 50 Pokémon diferentes", 0xFFFF, 70.0f},
            {"Pokédex: 100 capturados", "Capturar 100 Pokémon diferentes", 0xFFFF, 120.0f},
            {"Pokédex: 150 capturados", "Capturar 150 Pokémon diferentes", 0xFFFF, 180.0f},
            {"Pokédex: 200 capturados", "Capturar 200 Pokémon diferentes", 0xFFFF, 250.0f},
            {"Pokédex Hoenn completo", "Completar Pokédex regional (202)", 0x865, 400.0f},
            {"Pokédex Nacional completo", "Completar Pokédex Nacional (386)", 0x866, 1000.0f},
        };
    }
    
    inline std::vector<EventoHistoria> obtenerTodosLosEventos() {
        std::vector<EventoHistoria> todos;
        
        auto principales = obtenerEventosHistoriaPrincipal();
        auto secundarios = obtenerEventosSecundarios();
        auto personajes = obtenerEventosPersonajes();
        auto pokedex = obtenerEventosPokedex();
        
        todos.insert(todos.end(), principales.begin(), principales.end());
        todos.insert(todos.end(), secundarios.begin(), secundarios.end());
        todos.insert(todos.end(), personajes.begin(), personajes.end());
        todos.insert(todos.end(), pokedex.begin(), pokedex.end());
        
        return todos;
    }
}

#endif // EVENTOS_EMERALD_H
