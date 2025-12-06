#!/usr/bin/env python3
"""
Script para consolidar múltiples modelos entrenados en uno solo.
Promedia los pesos de todos los modelos encontrados.
"""

import torch
import glob
import os
import sys
from collections import OrderedDict

def consolidar_modelos(patron_modelos="pokemon_dqn_model_*.pt", 
                       modelo_salida="pokemon_dqn_model_consolidated.pt"):
    """
    Consolida múltiples modelos en uno solo promediando sus pesos.
    
    Args:
        patron_modelos: Patrón para encontrar los modelos (ej: "pokemon_dqn_model_*.pt")
        modelo_salida: Nombre del archivo de salida
    """
    
    # Buscar todos los modelos
    archivos_modelo = sorted(glob.glob(patron_modelos))
    
    if len(archivos_modelo) == 0:
        print(f"❌ Error: No se encontraron modelos con el patrón '{patron_modelos}'")
        print(f"   Asegúrate de estar en el directorio 'build/' donde están los modelos")
        return False
    
    print(f"\n{'='*60}")
    print(f"  Consolidación de Modelos RL")
    print(f"{'='*60}\n")
    print(f"📁 Modelos encontrados: {len(archivos_modelo)}")
    for i, archivo in enumerate(archivos_modelo):
        print(f"   {i}: {archivo}")
    print()
    
    # Cargar el primer modelo para obtener la estructura
    print("📥 Cargando modelos...")
    try:
        modelo_base = torch.load(archivos_modelo[0])
        print(f"   ✓ Modelo base cargado: {archivos_modelo[0]}")
    except Exception as e:
        print(f"❌ Error cargando modelo base: {e}")
        return False
    
    # Inicializar diccionario para acumular pesos
    pesos_acumulados = OrderedDict()
    for key in modelo_base.keys():
        pesos_acumulados[key] = modelo_base[key].clone().float()
    
    # Cargar y acumular el resto de modelos
    modelos_cargados = 1
    for i, archivo in enumerate(archivos_modelo[1:], 1):
        try:
            modelo = torch.load(archivo)
            for key in modelo.keys():
                if key in pesos_acumulados:
                    pesos_acumulados[key] += modelo[key].float()
            modelos_cargados += 1
            print(f"   ✓ Modelo {i+1}/{len(archivos_modelo)} acumulado: {archivo}")
        except Exception as e:
            print(f"   ⚠ Error cargando {archivo}: {e}")
            continue
    
    if modelos_cargados == 0:
        print("❌ Error: No se pudo cargar ningún modelo")
        return False
    
    # Promediar los pesos
    print(f"\n🧮 Promediando pesos de {modelos_cargados} modelos...")
    modelo_consolidado = OrderedDict()
    for key in pesos_acumulados.keys():
        modelo_consolidado[key] = pesos_acumulados[key] / modelos_cargados
    
    # Guardar modelo consolidado
    print(f"💾 Guardando modelo consolidado...")
    try:
        torch.save(modelo_consolidado, modelo_salida)
        print(f"   ✓ Modelo guardado: {modelo_salida}")
    except Exception as e:
        print(f"❌ Error guardando modelo: {e}")
        return False
    
    # Mostrar estadísticas
    print(f"\n{'='*60}")
    print(f"  Resumen")
    print(f"{'='*60}")
    print(f"Modelos consolidados: {modelos_cargados}")
    print(f"Archivo de salida: {modelo_salida}")
    
    # Calcular tamaño del archivo
    tamaño_mb = os.path.getsize(modelo_salida) / (1024 * 1024)
    print(f"Tamaño del modelo: {tamaño_mb:.2f} MB")
    
    # Contar parámetros
    total_params = sum(p.numel() for p in modelo_consolidado.values())
    print(f"Parámetros totales: {total_params:,}")
    print(f"{'='*60}\n")
    
    print("✅ Consolidación completada exitosamente")
    print(f"\n💡 Para usar el modelo consolidado:")
    print(f"   1. Renombra o mueve tus modelos originales")
    print(f"   2. Copia el consolidado: cp {modelo_salida} pokemon_dqn_model_0.pt")
    print(f"   3. O usa directamente: torch::load(model, '{modelo_salida}')\n")
    
    return True


def consolidar_con_pesos(archivos_pesos, modelo_salida="pokemon_dqn_model_weighted.pt"):
    """
    Consolida modelos usando pesos específicos (por rendimiento).
    
    Args:
        archivos_pesos: Lista de tuplas (archivo, peso) ej: [("model_0.pt", 0.5), ("model_1.pt", 0.3)]
        modelo_salida: Nombre del archivo de salida
    """
    print(f"\n{'='*60}")
    print(f"  Consolidación Ponderada de Modelos")
    print(f"{'='*60}\n")
    
    # Verificar que los pesos sumen 1.0
    suma_pesos = sum(peso for _, peso in archivos_pesos)
    if abs(suma_pesos - 1.0) > 0.001:
        print(f"⚠ Advertencia: Los pesos suman {suma_pesos:.3f}, normalizando...")
        archivos_pesos = [(archivo, peso/suma_pesos) for archivo, peso in archivos_pesos]
    
    print("📊 Pesos asignados:")
    for archivo, peso in archivos_pesos:
        print(f"   {archivo}: {peso:.3f} ({peso*100:.1f}%)")
    print()
    
    # Cargar primer modelo
    try:
        modelo_base = torch.load(archivos_pesos[0][0])
        peso_base = archivos_pesos[0][1]
    except Exception as e:
        print(f"❌ Error cargando modelo base: {e}")
        return False
    
    # Inicializar con el primer modelo ponderado
    modelo_consolidado = OrderedDict()
    for key in modelo_base.keys():
        modelo_consolidado[key] = modelo_base[key].clone().float() * peso_base
    
    print("📥 Cargando y ponderando modelos...")
    # Acumular resto de modelos con sus pesos
    for archivo, peso in archivos_pesos[1:]:
        try:
            modelo = torch.load(archivo)
            for key in modelo.keys():
                if key in modelo_consolidado:
                    modelo_consolidado[key] += modelo[key].float() * peso
            print(f"   ✓ {archivo} (peso: {peso:.3f})")
        except Exception as e:
            print(f"   ⚠ Error con {archivo}: {e}")
            continue
    
    # Guardar
    print(f"\n💾 Guardando modelo consolidado ponderado...")
    try:
        torch.save(modelo_consolidado, modelo_salida)
        print(f"   ✓ Guardado: {modelo_salida}")
    except Exception as e:
        print(f"❌ Error guardando: {e}")
        return False
    
    print("\n✅ Consolidación ponderada completada")
    return True


def mostrar_info_modelos(patron="pokemon_dqn_model_*.pt"):
    """Muestra información sobre los modelos disponibles"""
    archivos = sorted(glob.glob(patron))
    
    if len(archivos) == 0:
        print(f"❌ No se encontraron modelos con patrón: {patron}")
        return
    
    print(f"\n{'='*60}")
    print(f"  Información de Modelos Disponibles")
    print(f"{'='*60}\n")
    
    for archivo in archivos:
        try:
            modelo = torch.load(archivo)
            tamaño_mb = os.path.getsize(archivo) / (1024 * 1024)
            params = sum(p.numel() for p in modelo.values())
            
            print(f"📄 {archivo}")
            print(f"   Tamaño: {tamaño_mb:.2f} MB")
            print(f"   Parámetros: {params:,}")
            
            # Intentar cargar estadísticas asociadas
            stats_file = archivo.replace("pokemon_dqn_model_", "training_stats_").replace(".pt", ".txt")
            if os.path.exists(stats_file):
                with open(stats_file, 'r') as f:
                    lines = f.readlines()
                    for line in lines:
                        if "Episodio:" in line or "Recompensa total:" in line:
                            print(f"   {line.strip()}")
            print()
        except Exception as e:
            print(f"⚠ Error leyendo {archivo}: {e}\n")


def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Consolida múltiples modelos de Pokémon RL en uno solo',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Ejemplos de uso:
  
  # Consolidar todos los modelos (promedio simple)
  python3 consolidate_models.py
  
  # Especificar patrón y salida
  python3 consolidate_models.py -p "model_*.pt" -o consolidated.pt
  
  # Ver información de modelos
  python3 consolidate_models.py --info
  
  # Consolidación ponderada (mejores modelos tienen más peso)
  python3 consolidate_models.py --weighted \\
      pokemon_dqn_model_0.pt:0.5 \\
      pokemon_dqn_model_1.pt:0.3 \\
      pokemon_dqn_model_2.pt:0.2
        """
    )
    
    parser.add_argument('-p', '--pattern', 
                       default='pokemon_dqn_model_*.pt',
                       help='Patrón para buscar modelos (default: pokemon_dqn_model_*.pt)')
    
    parser.add_argument('-o', '--output',
                       default='pokemon_dqn_model_consolidated.pt',
                       help='Nombre del archivo de salida (default: pokemon_dqn_model_consolidated.pt)')
    
    parser.add_argument('--info',
                       action='store_true',
                       help='Mostrar información de modelos disponibles')
    
    parser.add_argument('--weighted',
                       nargs='+',
                       metavar='MODEL:WEIGHT',
                       help='Consolidación ponderada. Ej: model_0.pt:0.5 model_1.pt:0.5')
    
    args = parser.parse_args()
    
    # Mostrar info
    if args.info:
        mostrar_info_modelos(args.pattern)
        return
    
    # Consolidación ponderada
    if args.weighted:
        archivos_pesos = []
        for item in args.weighted:
            try:
                archivo, peso = item.split(':')
                archivos_pesos.append((archivo, float(peso)))
            except ValueError:
                print(f"❌ Error: Formato inválido '{item}'. Usa: archivo.pt:0.5")
                return
        
        consolidar_con_pesos(archivos_pesos, args.output)
        return
    
    # Consolidación normal (promedio)
    consolidar_modelos(args.pattern, args.output)


if __name__ == "__main__":
    main()
