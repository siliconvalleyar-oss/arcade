
mkdir -p obj/ bin/ 
# Ubuntu/Debian
sudo apt-get install build-essential
sudo apt-get install libncurses-dev
sudo apt-get install libx11-dev
sudo apt-get install libxext-dev
sudo apt-get install cmake
# 🎮 RetroMan

Un motor de juegos retro escrito en C++17 con sistemas ECS (Entity Component System), renderizado por software y soporte para sprites PNG.

## ✨ Características

- 🎯 **Arquitectura ECS** - Entity Component System para máxima flexibilidad
- 🖼️ **Renderizado por software** - Usando tinyPTC para gráficos rápidos
- 📦 **Soporte PNG** - Carga de sprites desde archivos PNG (picoPNG)
- 💥 **Sistema de físicas** - Movimiento y colisiones básicas
- 🎨 **Gráficos 2D** - Ventana de 640x360 píxeles
- 🔧 **C++17 moderno** - Usando lo último del estándar

## 📋 Requisitos

### Dependencias del sistema

```bash
# Ubuntu/Debian
sudo apt-get install build-essential
sudo apt-get install libncurses-dev
sudo apt-get install libx11-dev
sudo apt-get install libxext-dev
sudo apt-get install cmake
