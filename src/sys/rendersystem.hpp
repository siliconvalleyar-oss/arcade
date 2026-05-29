#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include "../util/typealiases.hpp"
#include "../cmp/entity.hpp"

namespace ECS{

	struct EntityManager_t;
	struct GameContext_t;

	// Simple 5x7 bitmap font for HUD rendering
	struct BitmapFont_t {
		static void drawChar(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
		                     char c, uint32_t color);
		static void drawString(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
		                       const char* str, uint32_t color);
		static void drawInt(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
		                    int32_t value, uint32_t color);
	};

	// ============================================================
	// Scene Background (loaded from PNG)
	// ============================================================
	struct SceneBackground_t {
		void load(const std::string& filename, uint32_t scrW, uint32_t scrH);
		void draw(uint32_t* fb, uint32_t scrW, uint32_t scrH) const;

		std::vector<uint32_t> pixels;
		uint32_t imgW{0}, imgH{0};
	};

	// ============================================================
	// Procedural Parallax Background (used when no scene BG loaded)
	// ============================================================
	struct ParallaxBG_t {
		struct Star { int32_t x, y; uint8_t brightness; };
		struct Cloud { int32_t x, y, w, h; };

		void generate(uint32_t scrW, uint32_t scrH);
		void draw(uint32_t* fb, uint32_t scrW, uint32_t scrH,
		          int32_t scrollX) const;

	private:
		void drawStar(uint32_t* fb, uint32_t scrW, uint32_t scrH,
		              int32_t sx, int32_t sy, uint8_t brightness,
		              int32_t scroll) const;
		void drawCloud(uint32_t* fb, uint32_t scrW, uint32_t scrH,
		               const Cloud& cloud, int32_t scroll) const;

		std::vector<Star>  m_farStars;
		std::vector<Star>  m_midStars;
		std::vector<Star>  m_nearStars;
		std::vector<Cloud> m_clouds;
		uint32_t m_scrW{0}, m_scrH{0};
	};

	// ============================================================
	// Particle System
	// ============================================================
	struct Particle_t {
		float x, y;
		float vx, vy;
		float lifetime;
		float maxLifetime;
		uint32_t color;
		int size;
	};

	struct ParticleSystem_t {
		void emit(int32_t x, int32_t y, int count, uint32_t color,
		          float speedMin, float speedMax, float lifetime,
		          int size = 2);
		void update();
		void draw(uint32_t* fb, uint32_t scrW, uint32_t scrH) const;
		void clear();

		std::vector<Particle_t> particles;
	};

	// ============================================================
	// Maze Tile Sprites (pre-loaded)
	// ============================================================
	struct MazeTileSprites_t {
		void load();
		const std::vector<uint32_t>& getTile(uint32_t tileType) const;

		std::vector<uint32_t> wall;
		std::vector<uint32_t> floor;
		std::vector<uint32_t> door;
		std::vector<uint32_t> goal;
		std::vector<uint32_t> spike;
	std::vector<uint32_t> platform;
	std::vector<uint32_t> coin;
	uint32_t tileSize{64};
	};

	struct RenderSystem_t
	{
		explicit RenderSystem_t(uint32_t, uint32_t);
		~RenderSystem_t();

		// Frame lifecycle
		void beginFrame(int32_t playerVx, int32_t camX = 0, int32_t camY = 0);
		void updateParticles();
		void drawParticles() const;
		void emitParticles(int32_t x, int32_t y, int count, uint32_t color,
		                   float speedMin, float speedMax, float lifetime,
		                   int size = 2);
		void clearParticles();
		bool endFrame();
		uint32_t* getFramebuffer() const;

		void drawAllEntities(const Vect_t<Entity_t>&) const;

		// Camera control
		void setCamera(int32_t cx, int32_t cy) { m_camX = cx; m_camY = cy; }

		// Scene background
		void loadSceneBG(const std::string& filename);
		void setUseSceneBG(bool use) { m_useSceneBG = use; }

		// Maze rendering
		void loadMazeTiles();
		void drawMaze(const uint8_t* maze, uint32_t mazeW, uint32_t mazeH,
		              uint32_t tileSize, int32_t camX, int32_t camY) const;

		// HUD
	void drawHUD(uint32_t score, uint32_t lives, bool invincible) const;
		void drawGameOver() const;
		void drawLevelClear() const;
		void drawSceneName(const char* name) const;
		void drawBossHPBar(int32_t hp, int32_t maxHP) const;
		void drawTitle() const;
		void drawVictory(uint32_t score, uint32_t coins) const;

		uint32_t width()  const { return m_w; }
		uint32_t height() const { return m_h; }

	private:
		const uint32_t m_w { 0 }, m_h { 0 };
		std::unique_ptr<uint32_t[]> m_framebuffer{ nullptr };
		ParallaxBG_t m_parallaxBG;
		SceneBackground_t m_sceneBG;
		ParticleSystem_t m_particles;
		MazeTileSprites_t m_mazeTiles;

		int32_t m_scrollOffset { 0 };
		int32_t m_camX { 0 }, m_camY { 0 };
		bool m_useSceneBG { false };
		bool m_mazeTilesLoaded { false };
	};

}//end namespace ECS
