extern "C"{
	#include "../../lib/tinyPTC/src/tinyptc.h"
}
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include "rendersystem.hpp"
#include "../man/entitymanager.hpp"
#include "../util/gamecontext.hpp"

namespace ECS
{
	// ============================================================
	// Bitmap Font: 5x7 pixel characters
	//   Bit 0 = top row, Bit 6 = bottom row
	// ============================================================
	static unsigned char kFontData(char c, int col) {
		if(col < 0 || col >= 5) return 0;
		switch(c) {
			case ' ': return 0x00;
			case '0': { static const unsigned char d[5] = {0x3E,0x51,0x49,0x45,0x3E}; return d[col]; }
			case '1': { static const unsigned char d[5] = {0x00,0x42,0x7F,0x40,0x00}; return d[col]; }
			case '2': { static const unsigned char d[5] = {0x42,0x61,0x51,0x49,0x46}; return d[col]; }
			case '3': { static const unsigned char d[5] = {0x21,0x41,0x45,0x4B,0x31}; return d[col]; }
			case '4': { static const unsigned char d[5] = {0x18,0x14,0x12,0x7F,0x10}; return d[col]; }
			case '5': { static const unsigned char d[5] = {0x27,0x45,0x45,0x45,0x39}; return d[col]; }
			case '6': { static const unsigned char d[5] = {0x3C,0x4A,0x49,0x49,0x30}; return d[col]; }
			case '7': { static const unsigned char d[5] = {0x01,0x71,0x09,0x05,0x03}; return d[col]; }
			case '8': { static const unsigned char d[5] = {0x36,0x49,0x49,0x49,0x36}; return d[col]; }
			case '9': { static const unsigned char d[5] = {0x06,0x49,0x49,0x29,0x1E}; return d[col]; }
			case 'A': { static const unsigned char d[5] = {0x7C,0x12,0x11,0x12,0x7C}; return d[col]; }
			case 'B': { static const unsigned char d[5] = {0x7F,0x49,0x49,0x49,0x36}; return d[col]; }
			case 'C': { static const unsigned char d[5] = {0x3E,0x41,0x41,0x41,0x22}; return d[col]; }
			case 'D': { static const unsigned char d[5] = {0x7F,0x41,0x41,0x22,0x1C}; return d[col]; }
			case 'E': { static const unsigned char d[5] = {0x7F,0x49,0x49,0x49,0x41}; return d[col]; }
			case 'F': { static const unsigned char d[5] = {0x7F,0x09,0x09,0x09,0x01}; return d[col]; }
			case 'G': { static const unsigned char d[5] = {0x3E,0x41,0x49,0x49,0x7A}; return d[col]; }
			case 'H': { static const unsigned char d[5] = {0x7F,0x08,0x08,0x08,0x7F}; return d[col]; }
			case 'I': { static const unsigned char d[5] = {0x00,0x41,0x7F,0x40,0x00}; return d[col]; }
			case 'J': { static const unsigned char d[5] = {0x20,0x40,0x41,0x3F,0x01}; return d[col]; }
			case 'K': { static const unsigned char d[5] = {0x7F,0x08,0x14,0x22,0x41}; return d[col]; }
			case 'L': { static const unsigned char d[5] = {0x7F,0x40,0x40,0x40,0x40}; return d[col]; }
			case 'M': { static const unsigned char d[5] = {0x7F,0x02,0x0C,0x02,0x7F}; return d[col]; }
			case 'N': { static const unsigned char d[5] = {0x7F,0x04,0x08,0x10,0x7F}; return d[col]; }
			case 'O': { static const unsigned char d[5] = {0x3E,0x41,0x41,0x41,0x3E}; return d[col]; }
			case 'P': { static const unsigned char d[5] = {0x7F,0x09,0x09,0x09,0x06}; return d[col]; }
			case 'Q': { static const unsigned char d[5] = {0x3E,0x41,0x51,0x21,0x5E}; return d[col]; }
			case 'R': { static const unsigned char d[5] = {0x7F,0x09,0x19,0x29,0x46}; return d[col]; }
			case 'S': { static const unsigned char d[5] = {0x46,0x49,0x49,0x49,0x31}; return d[col]; }
			case 'T': { static const unsigned char d[5] = {0x01,0x01,0x7F,0x01,0x01}; return d[col]; }
			case 'U': { static const unsigned char d[5] = {0x3F,0x40,0x40,0x40,0x3F}; return d[col]; }
			case 'V': { static const unsigned char d[5] = {0x1F,0x20,0x40,0x20,0x1F}; return d[col]; }
			case 'W': { static const unsigned char d[5] = {0x3F,0x40,0x30,0x40,0x3F}; return d[col]; }
			case 'X': { static const unsigned char d[5] = {0x63,0x14,0x08,0x14,0x63}; return d[col]; }
			case 'Y': { static const unsigned char d[5] = {0x07,0x08,0x70,0x08,0x07}; return d[col]; }
			case 'Z': { static const unsigned char d[5] = {0x61,0x51,0x49,0x45,0x43}; return d[col]; }
			case ':': { static const unsigned char d[5] = {0x00,0x36,0x36,0x00,0x00}; return d[col]; }
			case '!': { static const unsigned char d[5] = {0x00,0x00,0x5F,0x00,0x00}; return d[col]; }
			case '.': { static const unsigned char d[5] = {0x00,0x60,0x60,0x00,0x00}; return d[col]; }
			case '<': { static const unsigned char d[5] = {0x00,0x41,0x22,0x14,0x08}; return d[col]; }
			case '\'': { static const unsigned char d[5] = {0x00,0x02,0x03,0x00,0x00}; return d[col]; }
			case '(': { static const unsigned char d[5] = {0x00,0x1C,0x22,0x41,0x00}; return d[col]; }
			case ')': { static const unsigned char d[5] = {0x00,0x41,0x22,0x1C,0x00}; return d[col]; }
			default:  return 0x00;
		}
	}

	void BitmapFont_t::drawChar(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
	                            char c, uint32_t color)
	{
		if(x < -5 || x >= 8000 || y < -7 || y >= 8000) return;
		for(int32_t col = 0; col < 5; ++col) {
			int32_t px = x + col;
			if(px < 0) continue;
			unsigned char bits = kFontData(c, col);
			for(int32_t row = 0; row < 7; ++row) {
				int32_t py = y + row;
				if(py < 0) continue;
				// Bit 0 = top row, Bit 6 = bottom row
				if(bits & (1 << row)) {
					fb[py * fbW + px] = color;
				}
			}
		}
	}

	void BitmapFont_t::drawString(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
	                              const char* str, uint32_t color)
	{
		int32_t cx = x;
		while(*str) {
			if(*str == '\n') { cx = x; y += 9; }
			else { drawChar(fb, fbW, cx, y, *str, color); cx += 6; }
			str++;
		}
	}

	void BitmapFont_t::drawInt(uint32_t* fb, uint32_t fbW, int32_t x, int32_t y,
	                           int32_t value, uint32_t color)
	{
		char buf[32];
		int len = 0;
		if(value < 0) { buf[len++] = '-'; value = -value; }
		if(value == 0) { buf[len++] = '0'; }
		else {
			char digits[16];
			int dlen = 0;
			while(value > 0) { digits[dlen++] = '0' + (value % 10); value /= 10; }
			for(int i = dlen - 1; i >= 0; --i) buf[len++] = digits[i];
		}
		buf[len] = '\0';
		drawString(fb, fbW, x, y, buf, color);
	}

	// ============================================================
	// Parallax Background implementation
	// ============================================================
	void ParallaxBG_t::generate(uint32_t scrW, uint32_t scrH)
	{
		m_scrW = scrW;
		m_scrH = scrH;

		m_farStars.clear();
		m_midStars.clear();
		m_nearStars.clear();
		m_clouds.clear();

		for(int i = 0; i < 120; ++i) {
			m_farStars.push_back({
				static_cast<int32_t>(std::rand() % (scrW * 3)),
				static_cast<int32_t>(std::rand() % scrH),
				static_cast<uint8_t>(80 + std::rand() % 60)
			});
		}
		for(int i = 0; i < 80; ++i) {
			m_midStars.push_back({
				static_cast<int32_t>(std::rand() % (scrW * 2)),
				static_cast<int32_t>(std::rand() % scrH),
				static_cast<uint8_t>(140 + std::rand() % 80)
			});
		}
		for(int i = 0; i < 40; ++i) {
			m_nearStars.push_back({
				static_cast<int32_t>(std::rand() % (scrW * 3 / 2)),
				static_cast<int32_t>(std::rand() % scrH),
				static_cast<uint8_t>(200 + std::rand() % 56)
			});
		}
		for(int i = 0; i < 8; ++i) {
			m_clouds.push_back({
				static_cast<int32_t>(std::rand() % (scrW * 2)),
				static_cast<int32_t>(20 + std::rand() % (scrH / 3)),
				static_cast<int32_t>(40 + std::rand() % 80),
				static_cast<int32_t>(12 + std::rand() % 20)
			});
		}
	}

	void ParallaxBG_t::drawStar(uint32_t* fb, uint32_t scrW, uint32_t scrH,
	                            int32_t sx, int32_t sy, uint8_t brightness,
	                            int32_t scroll) const
	{
		int32_t px = (sx + scroll) % (scrW * 3);
		if(px < 0) px += (scrW * 3);
		if(px < 0 || px >= static_cast<int32_t>(scrW)) return;
		if(sy < 0 || sy >= static_cast<int32_t>(scrH)) return;

		uint32_t color = (static_cast<uint32_t>(brightness) << 16)
		               | (static_cast<uint32_t>(brightness) << 8)
		               | static_cast<uint32_t>(brightness);
		fb[sy * scrW + px] = color;
	}

	void ParallaxBG_t::drawCloud(uint32_t* fb, uint32_t scrW, uint32_t scrH,
	                             const Cloud& cloud, int32_t scroll) const
	{
		int32_t baseX = (cloud.x + scroll) % (scrW * 2);
		if(baseX < 0) baseX += (scrW * 2);
		if(baseX > static_cast<int32_t>(scrW)) return;

		const int blobCount = 4;
		const int32_t blobs[4][3] = {
			{ cloud.w / 4, cloud.h / 2, cloud.w / 3 },
			{ cloud.w * 3 / 8, cloud.h / 3, cloud.w / 3 },
			{ cloud.w / 2, cloud.h / 2, cloud.w / 3 },
			{ cloud.w * 3 / 4, cloud.h / 2, cloud.w / 4 }
		};

		for(int b = 0; b < blobCount; ++b) {
			int32_t cx = baseX + blobs[b][0];
			int32_t cy = cloud.y + blobs[b][1];
			int32_t r  = blobs[b][2];

			int32_t x1 = std::max(0, cx - r);
			int32_t y1 = std::max(0, cy - r);
			int32_t x2 = std::min(static_cast<int32_t>(scrW), cx + r);
			int32_t y2 = std::min(static_cast<int32_t>(scrH), cy + r);

			for(int32_t py = y1; py < y2; ++py) {
				for(int32_t px = x1; px < x2; ++px) {
					int32_t dx = px - cx;
					int32_t dy = py - cy;
					if(dx * dx + dy * dy < r * r) {
						uint32_t bg = fb[py * scrW + px];
						uint8_t blend = 60 + (std::abs(dx) * 40 / r)
						               + (std::abs(dy) * 40 / r) / 2;
						if(blend > 120) blend = 120;

						uint8_t r_bg = (bg >> 16) & 0xFF;
						uint8_t g_bg = (bg >> 8) & 0xFF;
						uint8_t b_bg = bg & 0xFF;

						uint8_t r_out = r_bg + ((220 - r_bg) * blend >> 7);
						uint8_t g_out = g_bg + ((200 - g_bg) * blend >> 7);
						uint8_t b_out = b_bg + ((180 - b_bg) * blend >> 7);

						fb[py * scrW + px] = (static_cast<uint32_t>(r_out) << 16)
						                   | (static_cast<uint32_t>(g_out) << 8)
						                   | static_cast<uint32_t>(b_out);
					}
				}
			}
		}
	}

	void ParallaxBG_t::draw(uint32_t* fb, uint32_t scrW, uint32_t scrH,
	                        int32_t scrollX) const
	{
		int32_t farScroll = scrollX / 5;
		for(const auto& star : m_farStars) {
			drawStar(fb, scrW, scrH, star.x, star.y, star.brightness, farScroll);
		}
		int32_t midScroll = scrollX / 2;
		for(const auto& star : m_midStars) {
			drawStar(fb, scrW, scrH, star.x, star.y, star.brightness, midScroll);
		}
		int32_t cloudScroll = scrollX * 3 / 10;
		for(const auto& cloud : m_clouds) {
			drawCloud(fb, scrW, scrH, cloud, cloudScroll);
		}
		for(const auto& star : m_nearStars) {
			drawStar(fb, scrW, scrH, star.x, star.y, star.brightness, scrollX);
		}
	}

	// ============================================================
	// Particle System implementation
	// ============================================================
	void ParticleSystem_t::emit(int32_t x, int32_t y, int count, uint32_t color,
	                            float speedMin, float speedMax, float lifetime,
	                            int size)
	{
		for(int i = 0; i < count; ++i) {
			float angle = static_cast<float>(std::rand()) / RAND_MAX * 6.283185f;
			float speed = speedMin + (static_cast<float>(std::rand()) / RAND_MAX)
			                       * (speedMax - speedMin);
			uint8_t r = (color >> 16) & 0xFF;
			uint8_t g = (color >> 8) & 0xFF;
			uint8_t b = color & 0xFF;
			r = static_cast<uint8_t>(std::max(0, std::min(255,
			    static_cast<int>(r) + (std::rand() % 60 - 30))));
			g = static_cast<uint8_t>(std::max(0, std::min(255,
			    static_cast<int>(g) + (std::rand() % 60 - 30))));
			b = static_cast<uint8_t>(std::max(0, std::min(255,
			    static_cast<int>(b) + (std::rand() % 60 - 30))));
			uint32_t variedColor = (static_cast<uint32_t>(r) << 16)
			                     | (static_cast<uint32_t>(g) << 8)
			                     | static_cast<uint32_t>(b);

			particles.push_back({
				static_cast<float>(x),
				static_cast<float>(y),
				std::cos(angle) * speed,
				std::sin(angle) * speed,
				lifetime,
				lifetime,
				variedColor,
				size + (std::rand() % 2)
			});
		}
	}

	void ParticleSystem_t::update()
	{
		for(auto& p : particles) {
			p.x += p.vx;
			p.y += p.vy;
			p.vy += 0.15f;
			p.vx *= 0.97f;
			p.vy *= 0.97f;
			p.lifetime -= 1.0f;
		}
		auto it = std::remove_if(particles.begin(), particles.end(),
		    [](const Particle_t& p) { return p.lifetime <= 0.0f; });
		particles.erase(it, particles.end());
	}

	void ParticleSystem_t::draw(uint32_t* fb, uint32_t scrW, uint32_t scrH) const
	{
		for(const auto& p : particles) {
			if(p.lifetime <= 0.0f) continue;
			int32_t px = static_cast<int32_t>(p.x);
			int32_t py = static_cast<int32_t>(p.y);

			float lifeFrac = p.lifetime / p.maxLifetime;
			uint8_t alpha = static_cast<uint8_t>(lifeFrac * 255.0f);
			if(alpha < 10) continue;

			int half = p.size / 2;
			for(int dy = -half; dy <= half; ++dy) {
				for(int dx = -half; dx <= half; ++dx) {
					int32_t sx = px + dx;
					int32_t sy = py + dy;
					if(sx < 0 || sx >= static_cast<int32_t>(scrW)) continue;
					if(sy < 0 || sy >= static_cast<int32_t>(scrH)) continue;

					uint32_t bg = fb[sy * scrW + sx];
					uint8_t r_bg = (bg >> 16) & 0xFF;
					uint8_t g_bg = (bg >> 8) & 0xFF;
					uint8_t b_bg = bg & 0xFF;

					uint8_t r_p = (p.color >> 16) & 0xFF;
					uint8_t g_p = (p.color >> 8) & 0xFF;
					uint8_t b_p = p.color & 0xFF;

					uint32_t a = alpha;
					uint32_t invA = 255 - a;
					uint8_t r = (r_p * a + r_bg * invA) / 255;
					uint8_t g = (g_p * a + g_bg * invA) / 255;
					uint8_t b = (b_p * a + b_bg * invA) / 255;

					fb[sy * scrW + sx] = (static_cast<uint32_t>(r) << 16)
					                   | (static_cast<uint32_t>(g) << 8)
					                   | static_cast<uint32_t>(b);
				}
			}
		}
	}

	void ParticleSystem_t::clear()
	{
		particles.clear();
	}

	// ============================================================
	// RenderSystem implementation
	// ============================================================
	RenderSystem_t::RenderSystem_t(uint32_t w, uint32_t h)
	: m_w{w}, m_h{h}
	, m_framebuffer{std::make_unique<uint32_t[]>(w * h)}
	{
		ptc_open("RetroMania Arcade", static_cast<int>(w), static_cast<int>(h));
		m_parallaxBG.generate(w, h);
	}

	RenderSystem_t::~RenderSystem_t() {
		ptc_close();
	}

	uint32_t* RenderSystem_t::getFramebuffer() const {
		return m_framebuffer.get();
	}

	void RenderSystem_t::beginFrame(int32_t playerVx, int32_t camX, int32_t camY)
	{
		m_camX = camX;
		m_camY = camY;
		auto* fb = m_framebuffer.get();

		// Clear to sky blue background
		uint32_t skyColor = 0x0087CEEB; // Sky blue
		for(uint32_t i = 0; i < m_w * m_h; ++i) {
			fb[i] = skyColor;
		}

		// Draw scene background if loaded, otherwise parallax
		if(m_useSceneBG && m_sceneBG.imgW > 0) {
			m_sceneBG.draw(fb, m_w, m_h);
		} else {
			m_parallaxBG.draw(fb, m_w, m_h, m_scrollOffset);
		}
	}

	void RenderSystem_t::updateParticles() {
		m_particles.update();
	}

	void RenderSystem_t::drawParticles() const {
		m_particles.draw(m_framebuffer.get(), m_w, m_h);
	}

	void RenderSystem_t::emitParticles(int32_t x, int32_t y, int count, uint32_t color,
	                                   float speedMin, float speedMax, float lifetime,
	                                   int size) {
		m_particles.emit(x, y, count, color, speedMin, speedMax, lifetime, size);
	}

	void RenderSystem_t::clearParticles() {
		m_particles.clear();
	}

	bool RenderSystem_t::endFrame() {
		ptc_update(m_framebuffer.get());
		return !ptc_process_events();
	}

	void RenderSystem_t::drawAllEntities(const Vect_t<Entity_t>& entities) const
	{
		auto* fb = m_framebuffer.get();
		for(const auto& e : entities) {
			if(e.phy == nullptr || e.sprite.empty()) continue;

			int32_t screenX = e.phy->x - m_camX;
			int32_t screenY = e.phy->y - m_camY;

			int32_t startX = std::max(0, screenX);
			int32_t startY = std::max(0, screenY);
			int32_t endX   = std::min(static_cast<int32_t>(m_w),
			                  screenX + static_cast<int32_t>(e.w));
			int32_t endY   = std::min(static_cast<int32_t>(m_h),
			                  screenY + static_cast<int32_t>(e.h));

			if(startX >= endX || startY >= endY) continue;

			int32_t srcOffsetX = startX - screenX;
			int32_t srcOffsetY = startY - screenY;

			for(int32_t py = startY; py < endY; ++py) {
				uint32_t* dst = fb + py * m_w + startX;
				const uint32_t* src = e.sprite.data()
					+ static_cast<size_t>(srcOffsetY * e.w)
					+ static_cast<size_t>(srcOffsetX);
				for(int32_t px = startX; px < endX; ++px) {
					uint32_t pixel = *src;
					if(pixel & 0xFF000000) {
						*dst = pixel & 0x00FFFFFF;
					}
					dst++;
					src++;
				}
			}
		}
	}

	void RenderSystem_t::drawHUD(uint32_t score, uint32_t lives, bool invincible) const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w;

		// Top bar — semi-transparent dark strip
		uint32_t barColor = invincible ? 0x00332211 : 0x00111111;
		for(uint32_t x = 0; x < w; ++x) {
			for(int32_t y = 0; y < 18; ++y) {
				fb[y * w + x] = barColor;
			}
		}

		BitmapFont_t::drawString(fb, w, 10, 4, "SCORE", 0x00CCCCCC);
		BitmapFont_t::drawInt(fb, w, 80, 4, static_cast<int32_t>(score), 0x00FFFFFF);

		BitmapFont_t::drawString(fb, w, w - 180, 4, "LIVES", 0x00CCCCCC);
		for(uint32_t i = 0; i < lives; ++i) {
			BitmapFont_t::drawChar(fb, w, w - 110 + static_cast<int32_t>(i) * 22, 4, '<', 0x00FF4444);
			BitmapFont_t::drawChar(fb, w, w - 105 + static_cast<int32_t>(i) * 22, 4, '3', 0x00FF4444);
		}

		if(invincible) {
			BitmapFont_t::drawString(fb, w, w / 2 - 60, 4, "INVINCIBLE", 0x00FFAA00);
		}
	}

	void RenderSystem_t::drawGameOver() const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w, h = m_h;

		for(uint32_t i = 0; i < w * h; ++i) {
			uint32_t pixel = fb[i];
			uint32_t r = (pixel >> 16) & 0xFF;
			uint32_t g = (pixel >> 8) & 0xFF;
			uint32_t b = pixel & 0xFF;
			r = r / 3; g = g / 3; b = b / 3;
			fb[i] = (r << 16) | (g << 8) | b;
		}

		int32_t cx = static_cast<int32_t>(w) / 2;
		int32_t cy = static_cast<int32_t>(h) / 2 - 30;
		BitmapFont_t::drawString(fb, w, cx - 50, cy, "GAME OVER", 0x00FF4444);
		BitmapFont_t::drawString(fb, w, cx - 65, cy + 18, "PRESS R TO RESTART", 0x00AAAAAA);
		BitmapFont_t::drawString(fb, w, cx - 55, cy + 36, "PRESS ESC TO QUIT", 0x00666666);
	}

	// ============================================================
	// Scene Background
	// ============================================================
	void SceneBackground_t::load(const std::string& filename, uint32_t scrW, uint32_t scrH)
	{
		Entity_t tmp(filename);
		if(tmp.w == 0 || tmp.h == 0) {
			pixels.clear();
			imgW = imgH = 0;
			return;
		}
		pixels = std::move(tmp.sprite);
		imgW = tmp.w;
		imgH = tmp.h;
	}

	void SceneBackground_t::draw(uint32_t* fb, uint32_t scrW, uint32_t scrH) const
	{
		if(pixels.empty() || imgW == 0 || imgH == 0) return;

		// If image matches screen size, blit directly (fast path)
		if(imgW == scrW && imgH == scrH) {
			for(uint32_t i = 0; i < scrW * scrH; ++i) {
				uint32_t pixel = pixels[i];
				if(pixel & 0xFF000000) {
					fb[i] = pixel & 0x00FFFFFF;
				}
			}
			return;
		}

		// Otherwise scale/tile
		for(uint32_t sy = 0; sy < scrH; ++sy) {
			uint32_t srcY = (imgH > scrH) ? (sy * imgH / scrH) : (sy % imgH);
			if(srcY >= imgH) continue;
			for(uint32_t sx = 0; sx < scrW; ++sx) {
				uint32_t srcX = (imgW > scrW) ? (sx * imgW / scrW) : (sx % imgW);
				if(srcX >= imgW) continue;
				uint32_t pixel = pixels[srcY * imgW + srcX];
				if(pixel & 0xFF000000) {
					fb[sy * scrW + sx] = pixel & 0x00FFFFFF;
				}
			}
		}
	}

	// ============================================================
	// Maze Tile Sprites
	// ============================================================
	void MazeTileSprites_t::load()
	{
		tileSize = 64;
		auto loadTile = [&](const std::string& path, std::vector<uint32_t>& out) {
			Entity_t tmp(path);
			if(tmp.w > 0 && tmp.h > 0 && tmp.sprite.size() >= tileSize * tileSize) {
				out = std::move(tmp.sprite);
				return true;
			}
			return false;
		};

		if(!loadTile("assets/tile_wall.png", wall)) {
			wall.resize(tileSize * tileSize, 0xFF4A463C);
		}
		if(!loadTile("assets/tile_floor.png", floor)) {
			floor.resize(tileSize * tileSize, 0xFF6E5F46);
		}
		if(!loadTile("assets/tile_door.png", door)) {
			door.resize(tileSize * tileSize, 0xFF785028);
		}
		if(!loadTile("assets/tile_goal.png", goal)) {
			goal.resize(tileSize * tileSize, 0xFF3C1464);
		}
		if(!loadTile("assets/tile_spike.png", spike)) {
			spike.resize(tileSize * tileSize, 0xFF644628);
		}
		if(!loadTile("assets/tile_platform.png", platform)) {
			platform.resize(tileSize * tileSize, 0xFF4A6E3C);
		}
		// Coin tile (tile type 6) — try loading item_coin.png, fallback to yellow
		if(!loadTile("assets/item_coin.png", coin)) {
			coin.resize(tileSize * tileSize, 0xFFFFAA00);
		}
	}

	const std::vector<uint32_t>& MazeTileSprites_t::getTile(uint32_t tileType) const
	{
		switch(tileType) {
			case 1: return wall;
			case 2: return door;
			case 3: return goal;
			case 4: return spike;
			case 5: return platform;
			case 6: return coin;
			default: return floor;
		}
	}

	// ============================================================
	// RenderSystem: Scene & Maze methods
	// ============================================================
	void RenderSystem_t::loadSceneBG(const std::string& filename)
	{
		m_sceneBG.load(filename, m_w, m_h);
	}

	void RenderSystem_t::loadMazeTiles()
	{
		m_mazeTiles.load();
		m_mazeTilesLoaded = true;
	}

	void RenderSystem_t::drawMaze(const uint8_t* maze, uint32_t mazeW, uint32_t mazeH,
	                              uint32_t tileSize, int32_t camX, int32_t camY) const
	{
		if(!m_mazeTilesLoaded || maze == nullptr) return;
		auto* fb = m_framebuffer.get();

		int32_t firstTileX = camX / static_cast<int32_t>(tileSize);
		int32_t firstTileY = camY / static_cast<int32_t>(tileSize);
		int32_t lastTileX = (camX + static_cast<int32_t>(m_w) + static_cast<int32_t>(tileSize) - 1) / static_cast<int32_t>(tileSize);
		int32_t lastTileY = (camY + static_cast<int32_t>(m_h) + static_cast<int32_t>(tileSize) - 1) / static_cast<int32_t>(tileSize);

		if(firstTileX < 0) firstTileX = 0;
		if(firstTileY < 0) firstTileY = 0;
		if(lastTileX > static_cast<int32_t>(mazeW)) lastTileX = static_cast<int32_t>(mazeW);
		if(lastTileY > static_cast<int32_t>(mazeH)) lastTileY = static_cast<int32_t>(mazeH);

		for(int32 ty = firstTileY; ty < lastTileY; ++ty) {
			for(int32 tx = firstTileX; tx < lastTileX; ++tx) {
				uint8_t tileType = maze[ty * mazeW + tx];
				if(tileType == 0) continue;

				const auto& sprite = m_mazeTiles.getTile(tileType);
				if(sprite.empty()) continue;

				int32_t screenX = tx * static_cast<int32_t>(tileSize) - camX;
				int32_t screenY = ty * static_cast<int32_t>(tileSize) - camY;

				int32_t drawX = std::max(0, screenX);
				int32_t drawY = std::max(0, screenY);
				int32_t drawEndX = std::min(static_cast<int32_t>(m_w),
				                    screenX + static_cast<int32_t>(tileSize));
				int32_t drawEndY = std::min(static_cast<int32_t>(m_h),
				                    screenY + static_cast<int32_t>(tileSize));

				if(drawX >= drawEndX || drawY >= drawEndY) continue;

				int32_t srcOffX = drawX - screenX;
				int32_t srcOffY = drawY - screenY;

				for(int32_t py = drawY; py < drawEndY; ++py) {
					uint32_t* dst = fb + py * m_w + drawX;
					const uint32_t* src = sprite.data()
						+ static_cast<size_t>(srcOffY * tileSize)
						+ static_cast<size_t>(srcOffX);
					for(int32_t px = drawX; px < drawEndX; ++px) {
						uint32_t pixel = *src;
						if(pixel & 0xFF000000) {
							*dst = pixel & 0x00FFFFFF;
						}
						dst++;
						src++;
					}
				}
			}
		}
	}

	void RenderSystem_t::drawLevelClear() const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w, h = m_h;

		for(uint32_t i = 0; i < w * h; ++i) {
			uint32_t pixel = fb[i];
			uint32_t r = (pixel >> 16) & 0xFF;
			uint32_t g = (pixel >> 8) & 0xFF;
			uint32_t b = pixel & 0xFF;
			r = r * 2 / 3 + 20;
			g = g * 2 / 3 + 40;
			b = b * 2 / 3 + 20;
			fb[i] = (r << 16) | (g << 8) | b;
		}

		int32_t cx = static_cast<int32_t>(w) / 2;
		int32_t cy = static_cast<int32_t>(h) / 2 - 20;
		BitmapFont_t::drawString(fb, w, cx - 50, cy - 10, "LEVEL CLEAR!", 0x0044FF44);
		BitmapFont_t::drawString(fb, w, cx - 80, cy + 14, "PRESS SPACE TO CONTINUE", 0x00CCCCCC);
	}

	void RenderSystem_t::drawSceneName(const char* name) const
	{
		if(!name) return;
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w;

		int32_t len = 0;
		const char* p = name;
		while(*p) { len++; p++; }
		int32_t textW = len * 6;
		int32_t startX = (static_cast<int32_t>(w) - textW) / 2;

		for(uint32_t bx = static_cast<uint32_t>(startX - 10);
		    bx < static_cast<uint32_t>(startX + textW + 10) && bx < w; ++bx) {
			fb[23 * w + bx] = 0x00333344;
			fb[24 * w + bx] = 0x00333344;
			fb[25 * w + bx] = 0x00333344;
			fb[26 * w + bx] = 0x00333344;
		}
		BitmapFont_t::drawString(fb, w, startX, 24, name, 0x00FFDD88);
	}

	void RenderSystem_t::drawBossHPBar(int32_t hp, int32_t maxHP) const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w;
		constexpr int32_t barW = 300;
		constexpr int32_t barH = 16;
		constexpr int32_t barX = 250;
		constexpr int32_t barY = 42;

		for(int32_t y = barY; y < barY + barH; ++y) {
			for(int32_t x = barX; x < barX + barW; ++x) {
				if(x < 0 || x >= static_cast<int32_t>(w)) continue;
				fb[y * w + x] = 0x00333333;
			}
		}
		for(int32_t x = barX - 1; x <= barX + barW; ++x) {
			if(x < 0 || x >= static_cast<int32_t>(w)) continue;
			fb[(barY - 1) * w + x] = 0x00666666;
			fb[(barY + barH) * w + x] = 0x00666666;
		}
		for(int32_t y = barY; y < barY + barH; ++y) {
			if(barX - 1 >= 0) fb[y * w + (barX - 1)] = 0x00666666;
			if(barX + barW < static_cast<int32_t>(w)) fb[y * w + (barX + barW)] = 0x00666666;
		}

		if(hp <= 0) return;
		int32_t fillW = (hp * barW) / maxHP;
		if(fillW < 0) fillW = 0;
		if(fillW > barW) fillW = barW;

		uint32_t hpColor;
		float hpFrac = static_cast<float>(hp) / static_cast<float>(maxHP);
		if(hpFrac > 0.5f) {
			hpColor = 0x00FF4444;
		} else if(hpFrac > 0.25f) {
			hpColor = 0x00FF8800;
		} else {
			hpColor = 0x00FF2222;
			if(rand() % 4 == 0) hpColor = 0x00FFAA00;
		}

		for(int32_t y = barY + 2; y < barY + barH - 2; ++y) {
			for(int32_t x = barX + 2; x < barX + fillW - 2; ++x) {
				if(x < 0 || x >= static_cast<int32_t>(w)) continue;
				fb[y * w + x] = hpColor;
			}
		}
		BitmapFont_t::drawString(fb, w, barX, barY - 14, "BOSS", 0x00FF6666);
	}

	// ============================================================
	// Title Screen
	// ============================================================
	void RenderSystem_t::drawTitle() const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w, h = m_h;

		// Dark overlay
		for(uint32_t i = 0; i < w * h; ++i) {
			uint32_t pixel = fb[i];
			uint32_t r = (pixel >> 16) & 0xFF;
			uint32_t g = (pixel >> 8) & 0xFF;
			uint32_t b = pixel & 0xFF;
			r = r / 2; g = g / 2; b = b / 2;
			fb[i] = (r << 16) | (g << 8) | b;
		}

		int32_t cx = static_cast<int32_t>(w) / 2;
		int32_t cy = static_cast<int32_t>(h) / 2 - 50;

		// Title
		BitmapFont_t::drawString(fb, w, cx - 70, cy, "NINJA PLATFORMER", 0x00FFDD00);

		// Controls
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 25,
			"[A / LEFT ARROW]  MOVE LEFT", 0x00CCCCCC);
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 35,
			"[D / RIGHT ARROW] MOVE RIGHT", 0x00CCCCCC);
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 45,
			"[SPACE / W / UP]  JUMP", 0x00CCCCCC);
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 55,
			"[F]               ATTACK", 0x00CCCCCC);
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 65,
			"[R]               RESTART", 0x00CCCCCC);
		BitmapFont_t::drawString(fb, w, cx - 100, cy + 75,
			"[ESC]             QUIT", 0x00CCCCCC);

		BitmapFont_t::drawString(fb, w, cx - 95, cy + 95,
			"STOMP ENEMIES - COLLECT COINS - REACH THE FLAG", 0x00AAAAAA);

		// Pulse
		if((std::rand() % 30) < 20) {
			BitmapFont_t::drawString(fb, w, cx - 65, cy + 115,
				"PRESS SPACE TO START", 0x00FFAA00);
		}

		BitmapFont_t::drawString(fb, w, cx - 40, h - 30,
			"PRESS ESC TO QUIT", 0x00666666);
	}

	// ============================================================
	// Victory Screen
	// ============================================================
	void RenderSystem_t::drawVictory(uint32_t score, uint32_t coins) const
	{
		auto* fb = m_framebuffer.get();
		uint32_t w = m_w, h = m_h;

		for(uint32_t i = 0; i < w * h; ++i) {
			uint32_t pixel = fb[i];
			uint32_t r = (pixel >> 16) & 0xFF;
			uint32_t g = (pixel >> 8) & 0xFF;
			uint32_t b = pixel & 0xFF;
			r = r / 3 + 30;
			g = g / 3 + 20;
			b = b / 4;
			fb[i] = (r << 16) | (g << 8) | b;
		}

		int32_t cx = static_cast<int32_t>(w) / 2;
		int32_t cy = static_cast<int32_t>(h) / 2 - 50;

		BitmapFont_t::drawString(fb, w, cx - 70, cy, "YOU BEAT THE GAME!", 0x00FFDD00);
		BitmapFont_t::drawString(fb, w, cx - 45, cy + 20, "CONGRATULATIONS!", 0x00CCCCCC);

		char buf[64];
		int len = 0;
		const char* prefix = "FINAL SCORE: ";
		while(*prefix) buf[len++] = *prefix++;
		if(score == 0) { buf[len++] = '0'; }
		else {
			char digits[16];
			int dlen = 0;
			uint32_t s = score;
			while(s > 0) { digits[dlen++] = '0' + (s % 10); s /= 10; }
			for(int i = dlen - 1; i >= 0; --i) buf[len++] = digits[i];
		}
		buf[len] = '\0';
		BitmapFont_t::drawString(fb, w, cx - 50, cy + 50, buf, 0x00FFFFFF);

		len = 0;
		prefix = "COINS: ";
		while(*prefix) buf[len++] = *prefix++;
		if(coins == 0) { buf[len++] = '0'; }
		else {
			char digits[16];
			int dlen = 0;
			uint32_t c = coins;
			while(c > 0) { digits[dlen++] = '0' + (c % 10); c /= 10; }
			for(int i = dlen - 1; i >= 0; --i) buf[len++] = digits[i];
		}
		buf[len] = '\0';
		BitmapFont_t::drawString(fb, w, cx - 30, cy + 70, buf, 0x00FFD700);

		BitmapFont_t::drawString(fb, w, cx - 70, cy + 100,
			"PRESS R TO PLAY AGAIN", 0x00AAAAAA);
	}
}//namespace ECS
