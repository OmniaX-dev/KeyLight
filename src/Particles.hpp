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

class Transform2D
{
    private:
        float m_rotation { 0.0f };
        ostd::Vec2 m_translation { 0.0f, 0.0f };
        ostd::Vec2 m_scale { 1.0f, 1.0f };
        bool m_centeredOrigin { false };
        ostd::Vec2 m_baseSize { 0.0f, 0.0f };

        bool m_applied { true };
        glm::mat4 m_matrix;
        std::vector<ostd::Vec2> m_vertices { {}, {}, {}, {} };

    public:
        inline Transform2D& resetRotation(float newValue = 0.0f) { m_applied = (m_rotation == newValue && m_applied); m_rotation = newValue; return *this; }
        inline Transform2D& resetTranslation(ostd::Vec2 newValue = { 0.0f, 0.0f }) { m_applied = (m_translation == newValue && m_applied); m_translation = newValue; return *this; }
        inline Transform2D& resetScale(ostd::Vec2 newValue = { 1.0f, 1.0f }) { m_applied = (m_scale == newValue && m_applied); m_scale = newValue; return *this; }
        inline Transform2D& reset(float _rotation = 0.0f, ostd::Vec2 _translation = { 0.0f, 0.0f }, ostd::Vec2 _scale = { 1.0f,1.0f }) { resetRotation(_rotation); resetTranslation(_translation); return resetScale(_scale); }

        inline Transform2D& rotate(float value) { m_applied = (value == 0.0f && m_applied); m_rotation += value; return *this; }
        inline Transform2D& translate(ostd::Vec2 value) { m_applied = (value == ostd::Vec2(0.0f, 0.0f) && m_applied); m_translation += value; return *this; }
        inline Transform2D& scale(ostd::Vec2 value) { m_applied = (value == ostd::Vec2(0.0f, 0.0f) && m_applied); m_scale += value; return *this; }

        inline float getRotation(void) const { return m_rotation; }
        inline ostd::Vec2 getTranslation(void) const { return m_translation; }
        inline ostd::Vec2 getScale(void) const { return m_scale; }
        inline bool isOriginCentered(void) const { return m_centeredOrigin; }
        inline bool isApplied(void) const { return m_applied; }
        inline glm::mat4 getMatrix(void) const { return m_matrix; }
        inline std::vector<ostd::Vec2> getVertices(void) const { return m_vertices; }
        inline std::vector<ostd::Vec2>& getVerticesRef(void) { return m_vertices; }
        inline ostd::Vec2 getBaseSize(void) const { return m_baseSize; }

        inline Transform2D& setOriginCentered(bool b = true) { m_applied = (m_centeredOrigin == b && m_applied); m_centeredOrigin = b; return *this; }
        inline Transform2D& setBaseSize(ostd::Vec2 baseSize) { m_applied = (m_baseSize == baseSize && m_applied); m_baseSize = baseSize; return *this; }

        inline Transform2D& apply(void)
        {
            if (m_applied) return *this;

            glm::vec4 npos(0.0f, 0.0f, 0.0f, 1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), { m_translation.x, m_translation.y, 0.0f });
            model = glm::rotate(model, DEG_TO_RAD(m_rotation), { 0.0f, 0.0f, 1.0f });
            model = glm::scale(model, { m_scale.x, m_scale.y, 1.0f });

            m_vertices.clear();
            if (m_centeredOrigin)
            {
                npos = { -(m_baseSize.x / 2.0f), -(m_baseSize.y / 2.0f), 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { m_baseSize.x / 2.0f, -(m_baseSize.y / 2.0f), 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { m_baseSize.x / 2.0f, m_baseSize.y / 2.0f, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { -(m_baseSize.x / 2.0f), m_baseSize.y / 2.0f, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
            }
            else
            {
                npos = { 0.0f, 0.0f, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { m_baseSize.x, 0.0f, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { m_baseSize.x, m_baseSize.y, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
                npos = { 0.0f, m_baseSize.y, 0.0f, 1.0f };
                npos = model * npos;
                m_vertices.push_back({ npos.x, npos.y });
            }

            m_applied = true;
            return *this;
        }
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
		float t = (float)currentFrame / (length - 1);
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

class TransformableObject : public ostd::BaseObject
{
	public:
		TransformableObject(ostd::Rectangle rect = { 0.0f, 0.0f, 16.0f, 16.0f });
		inline virtual ~TransformableObject(void) {  }

		TransformableObject& rotate(float angle);
		TransformableObject& translate(ostd::Vec2 translation);
		TransformableObject& scale(ostd::Vec2 scale);

		inline virtual void draw(void) {  }
		inline virtual void update(const ostd::Vec2& force = { 0.0f, 0.0f }) {  }

		inline ostd::Color getTintColor(void) const { return m_tintColor; }
		inline void setTintColor(ostd::Color color) { m_tintColor = color; }
		inline const Transform2D getTransform(void) const { return m_transform; }
		inline ostd::Rectangle getBounds(void) const { return m_bounds; }
		inline ostd::Rectangle getBaseRect(void) const { return m_rect; }
		inline void originCentered(bool b = true) { m_transform.setOriginCentered(b); }
		inline bool isOriginCentered(void) { return m_transform.isOriginCentered(); }
		inline bool isVisible(void) { return m_visible; }
		inline const std::vector<ostd::Vec2>& getVertices(void) { return m_vertices; }
		inline void setBaseRect(ostd::Rectangle r) { m_rect = r; update_transform(); }

	protected:
		void update_transform(void);
		inline virtual void setVisible(bool v) { m_visible = v; }

	private:
		std::vector<ostd::Vec2> m_vertices;
		ostd::Color m_tintColor;
		Transform2D m_transform;
		ostd::Rectangle m_rect;
		ostd::Rectangle m_bounds;
		bool m_visible;
};

struct tParticleInfo
{
	float lifeSpan { 100 };

	ostd::Vec2 randomVelocity { 0.2f, 0.2f };
	float randomLifeSpan { 0.2f };
	float randomDirection { 0.2f };
	float randomSpeed { 0.2f };
	float randomAlpha { 0.2f };
	float randomRotation { 0.2f };
	ostd::Vec2 randomSize { 0.2f, 0.2f  };
	bool randomDamping { true };

	ostd::Vec2 damping { 0.005f, 0.005f };
	TextureRef* texture { nullptr };
	// TextureRef::ID texture { TextureRef::InvalidTexture };
	TextureRef::TextureAtlasIndex tileIndex { TextureRef::FullTextureCoords };
	ostd::Color color { 120, 120, 120, 80 };
	ostd::Vec2 size { 16.0f, 16.0f };
	float rotationStep { 1.0f };

	bool fadeIn { true };
	bool allDirectionos { false };
	bool square { true };

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
		void afterUpdate(void) override;
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
		// TextureRef::ID texture { TextureRef::InvalidTexture };
		TextureRef::TextureAtlasIndex tileIndex { TextureRef::FullTextureCoords };
		Transform2D transform;
		float alpha { 0.0f };

		bool dead { false };
		bool fadeIn { true };

		ColorRamp colorRamp;
};

class ParticleEmitter : public TransformableObject
{
	public:
		ParticleEmitter(void);
		ParticleEmitter(ostd::Rectangle emissionRect, uint32_t maxParticles = 400);
		ParticleEmitter(ostd::Vec2 position, uint32_t maxParticles = 400);
		ParticleEmitter& create(ostd::Rectangle emissionRect, uint32_t maxParticles = 400);

		void update(const ostd::Vec2& force = { 0.0f, 0.0f }) override;

		void emit(tParticleInfo partInfo, int32_t count = 1);
		void emit(int32_t count = 1);

		void setDefaultParticleInfo(tParticleInfo info);
		inline tParticleInfo& getDefaultParticleInfo(void) { return m_defaultParticle; }
		inline void setEmissionRect(ostd::Rectangle rect) { setBaseRect(rect); }
		inline ostd::Rectangle getEmissionRect(void) { return getBaseRect(); }
		inline void setWorkingRectangle(ostd::Rectangle rect) { m_workingRect = rect; }
		inline ostd::Rectangle getWorkingRectangle(void) { return m_workingRect; }
		inline bool isParticleTransformEnabled(void) { return m_useParticleTransform; }
		inline void useParticleTransform(bool p = true) { m_useParticleTransform = p; }
		inline void setMaxParticleCount(uint32_t maxParticles) { m_particleCount = maxParticles; m_particles.resize(m_particleCount); }
		inline uint32_t getMaxParticleCount(void) { return m_particleCount; }
		inline void useTileArray(bool u = true) { m_useTileArray = u; }
		inline bool isTileArrayUsed(void) { return m_useTileArray; }
		void addTilesToArray(const std::vector<TextureRef::TextureAtlasIndex>& array);

		// inline void setPath(Spline& path) { m_path = path; m_path.enable(); }
		inline void enablePath(bool e = true) { m_path.enable(e); }
		inline void addPathPoint(ostd::Vec2 point) { m_path.addPoint(point); }
		inline void enableEditablePath(bool e = true) { m_path.setEditable(e); if (e) m_path.connectSignals(); }

		// inline ParticleVertexArray& getParticleVertexArray(void) { return m_particleVertexArray; }
		inline sf::VertexArray& getVertexArray(void) { return m_vertexArray; }

	private:
		ostd::Vec2 getRandomEmissionPoint(void);

	private:
		tParticleInfo m_defaultParticle;
		std::vector<Particle> m_particles;
		uint32_t m_particleCount;
		ostd::Rectangle m_workingRect;
		std::vector<TextureRef::TextureAtlasIndex> m_tileArray;

		ostd::Spline m_path;
		float m_currentPathValue { 0.0f };
		float m_pathStep { 15.0f };
		ostd::tSplineNode m_currentPathPoint { { 0.0f, 0.0f }, 0.0f };
		bool m_useParticleTransform { true };
		bool m_useTileArray { false };

		// ParticleVertexArray m_particleVertexArray;
		sf::VertexArray m_vertexArray;
};

class ParticleFactory
{
	public:
		static tParticleInfo basicFireParticle(TextureRef::TextureInfo texture = { nullptr, TextureRef::FullTextureCoords });
		static tParticleInfo basicSnowParticle(TextureRef::TextureInfo texture = { nullptr, TextureRef::FullTextureCoords });

		static ParticleEmitter basicFireEmitter(TextureRef::TextureInfo texture, ostd::Vec2 position, uint32_t pre_emit_cycles = 0);
		static ParticleEmitter basicSnowEmitter(TextureRef::TextureInfo texture, ostd::Vec2 windowSize, uint32_t pre_emit_cycles = 0);

	private:
		static void __pre_emit(ParticleEmitter& emitter, uint32_t pre_emit_cycles, ostd::Vec2 rand_force_range);
};
