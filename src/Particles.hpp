/*
    KeyLight - A MIDI Piano Visualizer
    Copyright (C) 2025  OmniaX-Dev

    This file is part of KeyLight.

    KeyLight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KeyLight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KeyLight.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <ostd/BaseObject.hpp>
#include <ostd/Color.hpp>
#include <ostd/Defines.hpp>
#include <ostd/Spline.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <any>

class TextureRef : public ostd::BaseObject
{
	public:
		typedef uint32_t TextureAtlasIndex;
		typedef uint32_t ID;

	public: struct TextureInfo
	{
		TextureRef* texture { nullptr };
		TextureAtlasIndex tile { 0 };
	};

    public: struct tTexCoords
    {
        ostd::Vec2 topLeft { 0.0f, 1.0f };
        ostd::Vec2 topRight { 1.0f, 1.0f };
        ostd::Vec2 bottomRight { 1.0f, 0.0f };
        ostd::Vec2 bottomLeft { 0.0f, 0.0f };
    };
    public:
        inline TextureRef(void) { create(); }
        ~TextureRef(void);
        TextureRef& create(void);

        ID attachTexture(std::any& texture, uint32_t width, uint32_t height);
        inline std::any* getTexture(void) const { return m_texturePtr; }
        inline bool hasTexture(void) const { return m_texturePtr != nullptr; }

        inline int32_t getWidth(void) const { return m_width; }
        inline int32_t getHeight(void) const { return m_height; }
        inline bool hasTileData(void) { return m_tiles.size() > 0; }
        TextureAtlasIndex addTileInfo(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        tTexCoords getTile(TextureAtlasIndex index);

    private:
        int32_t m_width { 0 };
        int32_t m_height { 0 };
        std::vector<tTexCoords> m_tiles;
        std::any* m_texturePtr { nullptr };

        inline static uint32_t s_nextID { 1 };

    public:
        inline static constexpr int32_t ERR_IMAGE_LOAD_FAILED = OX_TEXTURE_ERR_MASK + 0x0001;
        inline static constexpr int32_t ERR_NO_DATA_STORED = OX_TEXTURE_ERR_MASK + 0x0002;

        inline static constexpr TextureAtlasIndex FullTextureCoords = 0;
        inline static constexpr ID InvalidTexture = 0;
};

class PhysicsObject
{
	public:
		PhysicsObject(void);
		void applyForce(ostd::Vec2 force, float max = 0.0f);
		void physicsUpdate(void);
		inline void setStatic(bool st) { m_static = st; }
		inline bool isStatic(void) { return m_static; }

	protected:
		inline virtual void beforeUpdate(void) {  }
		inline virtual void afterUpdate(void) {  }
		inline virtual void beforeApplyForce(ostd::Vec2 force, float max) {  }
		inline virtual void afterApplyForce(ostd::Vec2 force, float max) {  }

		inline void skipNextUpdate(bool skip = true) { m_skipUpdate = skip; }
		inline void skipNextApplyForce(bool skip = true) { m_skipApplyForce = skip; }
		inline bool willSkipNextUpdate(void) { return m_skipUpdate; }
		inline bool willSkipNextApplyForce(void) { return m_skipApplyForce; }

	public:
		ostd::Vec2 velocity;
		ostd::Vec2 position;
		ostd::Vec2 acceleration;
		ostd::Vec2 velocityDamping;
		float maxVelocity;
		float maxForce;
		float mass;

	private:
		bool m_skipUpdate;
		bool m_skipApplyForce;
		bool m_static;
};

struct tColorInterpolator
{
	ostd::Color start;
	ostd::Color end;
	float length { 60.0f };
	uint32_t currentFrame { 0 };
	ostd::Color current;
	bool done { false };
	float percent { 0.2f };

	inline tColorInterpolator(void) {  }

	inline tColorInterpolator(ostd::Color st, ostd::Color en)
	{
		start = st;
		end = en;
		length = 0.0f;
		current = start;
	}

	inline void reset(void)
	{
		currentFrame = 0;
		current = start;
		done = false;
	}

	inline void updateLength(float new_total_length)
	{
		length = percent * new_total_length;
	}

	inline void update(void)
	{
		if (done || length == 0.0f) return;
		currentFrame++;
		if (currentFrame >= length)
		{
			done = true;
			return;
		}
		float t = std::min((float)currentFrame / length, 1.0f);
		uint8_t r = (int)std::round(std::lerp(start.r, end.r, t));
		uint8_t g = (int)std::round(std::lerp(start.g, end.g, t));
		uint8_t b = (int)std::round(std::lerp(start.b, end.b, t));
		uint8_t a = (int)std::round(std::lerp(start.a, end.a, t));
		current.set(r, g, b, a);
	}
};

class ColorRamp
{
	public:
		inline void update(void)
		{
			if (done) return;
			// if (m_colors.size() == 0) return;
			m_colors[m_currentColor].update();
			if (m_colors[m_currentColor].done)
				m_currentColor++;
			if (m_currentColor >= m_colors.size())
				done = true;
		}

		inline void reset(void)
		{
			m_currentColor = 0;
			for (auto& inter : m_colors)
				inter.reset();
			done = false;
		}

		inline ostd::Color current(void)
		{
			if (m_colors.size() == 0)
				return { 0, 0, 0, 0 };
			if (done) return m_colors[m_colors.size() - 1].current;
			return m_colors[m_currentColor].current;
		}

		inline uint32_t count(void) { return m_colors.size(); }

	public:
		std::vector<tColorInterpolator> m_colors;
		uint32_t m_currentColor { 0 };
		bool done { false };
};

struct tParticleInfo
{
	float lifeSpan { 100 };

	ostd::Vec2 randomVelocity { 0.2f, 0.2f };
	float randomLifeSpan { 0.2f };
	float randomDirection { 0.2f };
	float randomSpeed { 0.2f };
	float randomAlpha { 0.2f };
	ostd::Vec2 randomSize { 0.2f, 0.2f  };
	bool randomDamping { true };

	ostd::Vec2 damping { 0.005f, 0.005f };
	TextureRef* texture { nullptr };
	TextureRef::TextureAtlasIndex tileIndex { TextureRef::FullTextureCoords };
	ostd::Color color { 120, 120, 120, 80 };
	ostd::Vec2 size { 16.0f, 16.0f };

	bool fadeIn { true };
	bool allDirections { false };

	float angle { 0.0f };
	float speed { 1.0f };

	ColorRamp colorRamp;

	inline void addColorToGradient(ostd::Color start, ostd::Color end, float percent)
	{
		colorRamp.m_colors.push_back({ start, end });
		colorRamp.m_colors[colorRamp.m_colors.size() - 1].percent = percent;
		colorRamp.m_colors[colorRamp.m_colors.size() - 1].updateLength(lifeSpan);
	}
};

class Particle : public PhysicsObject
{
	public:
		inline Particle(void) { m_ready = false; }
		void setup(tParticleInfo partInfo);
		void beforeUpdate(void) override;
		void kill(void);
		inline bool isReady(void) { return m_ready; }
		inline bool isDead(void) { return m_dead; }

	private:
		float m_alpha_dec { 0 };
		bool m_ready { false };
		bool m_dead { false };
		bool m_fade_in { true };
		float m_curr_alpha { 0.0f };
		float m_fade_in_mult { 1.0f };

	public:
		float life { 100 };
		float fullLife { 100 };
		uint8_t fulLAlpha;
		float rotationStep { 1.0f };
		ostd::Color color { 10, 110, 255 };
		ostd::Vec2 size { 30.0f, 30.0f };
		TextureRef* texture { nullptr };
		TextureRef::TextureAtlasIndex tileIndex { TextureRef::FullTextureCoords };
		float alpha { 0.0f };

		bool dead { false };
		bool fadeIn { true };

		ColorRamp colorRamp;
};

class ParticleEmitter : public ostd::BaseObject
{
	public:
		ParticleEmitter(void);
		ParticleEmitter(ostd::Rectangle emissionRect, uint32_t maxParticles = 400);
		ParticleEmitter(ostd::Vec2 position, uint32_t maxParticles = 400);
		ParticleEmitter& create(ostd::Rectangle emissionRect, uint32_t maxParticles = 400);

		void update(const ostd::Vec2& force = { 0.0f, 0.0f });
		void reset(void);

		void emit(tParticleInfo partInfo, int32_t count = 1);
		void emit(int32_t count = 1);

		void setDefaultParticleInfo(tParticleInfo info);
		inline tParticleInfo& getDefaultParticleInfo(void) { return m_defaultParticle; }
		inline void setEmissionRect(ostd::Rectangle rect) { m_emissionRect = rect; }
		inline ostd::Rectangle getEmissionRect(void) { return m_emissionRect; }
		inline void setWorkingRectangle(ostd::Rectangle rect) { m_workingRect = rect; }
		inline ostd::Rectangle getWorkingRectangle(void) { return m_workingRect; }
		inline void setMaxParticleCount(uint32_t maxParticles) { m_particleCount = maxParticles; m_particles.resize(m_particleCount); m_vertexArray.resize(m_particleCount); }
		inline uint32_t getMaxParticleCount(void) { return m_particleCount; }
		inline void useTileArray(bool u = true) { m_useTileArray = u; }
		inline bool isTileArrayUsed(void) { return m_useTileArray; }
		void addTilesToArray(const std::vector<TextureRef::TextureAtlasIndex>& array);

		inline void enablePath(bool e = true) { m_path.enable(e); }
		inline void addPathPoint(ostd::Vec2 point) { m_path.addPoint(point); }
		inline void enableEditablePath(bool e = true) { m_path.setEditable(e); if (e) m_path.connectSignals(); }

		inline sf::VertexArray& getVertexArray(void) { return m_vertexArray; }

	private:
		ostd::Vec2 getRandomEmissionPoint(void);

	private:
		tParticleInfo m_defaultParticle;
		std::vector<Particle> m_particles;
		sf::VertexArray m_vertexArray;
		uint32_t m_particleCount;
		ostd::Rectangle m_workingRect;
		ostd::Rectangle m_emissionRect;
		std::vector<TextureRef::TextureAtlasIndex> m_tileArray;
		bool m_useTileArray { false };

		ostd::Spline m_path;
		float m_currentPathValue { 0.0f };
		float m_pathStep { 15.0f };
		ostd::tSplineNode m_currentPathPoint { { 0.0f, 0.0f }, 0.0f };
};

class ParticleFactory
{
	public:
		static tParticleInfo basicFireParticle(TextureRef::TextureInfo texture = { nullptr, TextureRef::FullTextureCoords });
		static tParticleInfo basicSnowParticle(TextureRef::TextureInfo texture = { nullptr, TextureRef::FullTextureCoords });

		static ParticleEmitter basicFireEmitter(TextureRef::TextureInfo texture, ostd::Vec2 position, uint32_t pre_emit_cycles = 0);
		static ParticleEmitter basicSnowEmitter(TextureRef::TextureInfo texture, ostd::Vec2 windowSize, uint32_t pre_emit_cycles = 0);

		static void createColorGradient(tParticleInfo& partInfo, const ostd::Color& startColor, uint8_t nColors);

	private:
		static void __pre_emit(ParticleEmitter& emitter, uint32_t pre_emit_cycles, ostd::Vec2 rand_force_range);
};
