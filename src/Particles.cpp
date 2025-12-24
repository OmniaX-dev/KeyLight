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

#include "Particles.hpp"
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <ostd/Defines.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Utils.hpp>
#include <ostd/Random.hpp>
#include <ostd/Logger.hpp>
#include "Common.hpp"


// ============================================== TextureRef ==============================================
TextureRef::~TextureRef(void)
{
	setID(InvalidTexture);
	invalidate();
}

TextureRef& TextureRef::create(void)
{
	if (isValid()) return *this;
	setID(TextureRef::s_nextID++);
	setTypeName("TextureRef");
	validate();
	return *this;
}

TextureRef::ID TextureRef::attachTexture(std::any& texture, uint32_t width, uint32_t height)
{
	if (m_texturePtr == nullptr)
	{
		m_texturePtr = &texture;
		m_width = width;
		m_height = height;
	}
	return getID();
}

TextureRef::TextureAtlasIndex TextureRef::addTileInfo(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	if (isInvalid() || x + w > m_width || y + h > m_height)
	{
		OX_WARN("Texture::addTileInfo(...): Invalid texture coordinates.");
		return TextureRef::FullTextureCoords;
	}
	if (!hasTileData())
		m_tiles.push_back(tTexCoords());
	ostd::Vec2 bottomLeft = { (float)x / (float)m_width, 1.0f - ((float)(y + h) / (float)m_height) };
	ostd::Vec2 bottomRight = { (float)(x + w) / (float)m_width, 1.0f - ((float)(y + h) / (float)m_height) };
	ostd::Vec2 topLeft = { (float)x / (float)m_width, 1.0f - ((float)y / (float)m_height) };
	ostd::Vec2 topRight = { (float)(x + w) / (float)m_width, 1.0f - ((float)y / (float)m_height) };
	tTexCoords texc;
	texc.bottomLeft = bottomLeft;
	texc.bottomRight = bottomRight;
	texc.topLeft = topLeft;
	texc.topRight = topRight;
	m_tiles.push_back(texc);
	return m_tiles.size() - 1;
}

TextureRef::tTexCoords TextureRef::getTile(TextureAtlasIndex index)
{
	if (!hasTileData() || index >= m_tiles.size())
	{
		//OX_WARN("ox::Texture::getTile(...): Unable to retrieve tile.");
		return TextureRef::tTexCoords();
	}
	return m_tiles[index];
}
// =====================================================================================================







// =========================================== PhysicsObject ==========================================
PhysicsObject::PhysicsObject(void)
{
	velocity = { 0, 0 };
	position = { 0, 0 };
	acceleration = { 0, 0 };
	velocityDamping = { 0, 0 };
	maxVelocity = 1.0f;
	maxForce = 2.0f;
	mass = 1.0f;
	m_skipApplyForce = false;
	m_skipUpdate = false;
	m_static = false;
}

void PhysicsObject::applyForce(ostd::Vec2 force, float max)
{
	force *= Common::deltaTime;
	max *= Common::deltaTime;
	beforeApplyForce(force, max);
	if (willSkipNextApplyForce())
	{
		skipNextApplyForce(false);
		return;
	}
	if (!isStatic())
	{
		if (max == 0.0f) max = (maxForce * Common::deltaTime);
		force.divm(mass);
		force.limit(max);
		acceleration.addm(force);
	}
	skipNextApplyForce(false);
	afterApplyForce(force, max);
}

void PhysicsObject::physicsUpdate(void)
{
	beforeUpdate();
	if (willSkipNextUpdate())
	{
		skipNextUpdate(false);
		return;
	}
	velocity.addm(acceleration);
	velocity.limit(maxVelocity);
	position.addm(velocity);
	acceleration.mulm(0);
	velocity.x *= 1.0f - MIN(velocityDamping.x, 0.9999f);
	velocity.y *= 1.0f - MIN(velocityDamping.y, 0.9999f);
	skipNextUpdate(false);
	afterUpdate();
}
// =====================================================================================================







// ============================================= Particle ==============================================
void Particle::setup(tParticleInfo partInfo)
{
	maxVelocity = 5.0f;
	velocity = { 0, 0 };
	acceleration = { 0, 0 };
	velocityDamping = { 0, 0 };

	float angle = partInfo.angle;
	if (partInfo.allDirections)
		angle = ostd::Random::getf32(0.0f, 360.0f);
	float dirVar = angle * partInfo.randomDirection;
	angle += ostd::Random::getf32(-dirVar, dirVar);

	float speedVar = partInfo.speed * partInfo.randomSpeed;
	float speed = partInfo.speed + ostd::Random::getf32(-speedVar, speedVar);

	float rad = DEG_TO_RAD(angle);
	velocity = { speed * std::cos(rad), -speed * std::sin(rad) };
	ostd::Vec2 velVar { velocity.x * partInfo.randomVelocity.x, velocity.y * partInfo.randomVelocity.y };
	velocity.x += ostd::Random::getf32(-velVar.x, velVar.x);
	velocity.y += ostd::Random::getf32(-velVar.y, velVar.y);

	float lifeVar = partInfo.lifeSpan * partInfo.randomLifeSpan;
	life = partInfo.lifeSpan;
	life += ostd::Random::getf32(-lifeVar, lifeVar);

	color = partInfo.color;
	float alphaVar = color.a * partInfo.randomAlpha;
	color.a += ostd::Random::geti8(-(int8_t)alphaVar, (int8_t)alphaVar);
	alpha = 0.0f;
	m_curr_alpha = 0.0f;

	size = partInfo.size;
	ostd::Vec2 sizeVar { size.x * partInfo.randomSize.x, size.y * partInfo.randomSize.y };
	size.x += ostd::Random::getf32(-sizeVar.x, sizeVar.x);
	size.y += ostd::Random::getf32(-sizeVar.y, sizeVar.y);

	if (partInfo.randomDamping)
	{
		velocityDamping.x += ostd::Random::getf32(0, partInfo.damping.x);
		velocityDamping.y += ostd::Random::getf32(0, partInfo.damping.y);
	}

	texture = partInfo.texture;
	tileIndex = partInfo.tileIndex;

	m_ready = true;
	m_dead = false;

	m_fade_in = true;
	fadeIn = partInfo.fadeIn;
	if (fadeIn)
	{
		alpha = color.a;
		m_alpha_dec = color.a / life;
		color.a = 0;
		life /= 2.0f;
	}
	else
	{
		m_alpha_dec = color.a / life;
		alpha = color.a;
	}

	fullLife = life;
	fulLAlpha = color.a;
	m_fade_in_mult = (partInfo.lifeSpan / 100.0f);
	colorRamp = partInfo.colorRamp;

	// velocity *= Common::deltaTime;
}

void Particle::beforeUpdate(void)
{
	if (isDead()) return;
	if (colorRamp.count() > 0)
	{
		color.r = colorRamp.current().r;
		color.g = colorRamp.current().g;
		color.b = colorRamp.current().b;
		colorRamp.update();
	}
	if (m_fade_in && fadeIn)
	{
		m_curr_alpha += (m_alpha_dec * m_fade_in_mult);
		color.a = m_curr_alpha;
		if (m_curr_alpha >= alpha)
		{
			m_fade_in = false;
			color.a = alpha;
		}
		return;
	}
	alpha -= m_alpha_dec * 2.0f;
	life--;
	if (alpha <= 0 || life <= 0)
	{
		color.a = 0;
		kill();
	}
	else color.a = std::round(alpha);
}

void Particle::kill(void)
{
	 m_dead = true;
}
// =====================================================================================================







// ========================================= ParticleEmitter ===========================================
ParticleEmitter::ParticleEmitter(void)
{
	invalidate();
}

ParticleEmitter::ParticleEmitter(ostd::Rectangle emissionRect, uint32_t maxParticles)
{
	create(emissionRect, maxParticles);
}

ParticleEmitter::ParticleEmitter(ostd::Vec2 position, uint32_t maxParticles)
{
	create({ position, 1, 1 }, maxParticles);
}

ParticleEmitter& ParticleEmitter::create(ostd::Rectangle emissionRect, uint32_t maxParticles)
{
	setEmissionRect(emissionRect);
	m_particles.resize(maxParticles);
	for (auto& part : m_particles)
		part.kill();
	m_particleCount = maxParticles;
	m_currentPathValue = 0.0f;

	m_vertexArray.resize(maxParticles * 6); // 6 because it is for two triangles per particles
	m_vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);

	enablePath(false);

	setTypeName("ox::ParticleEmitter");
	validate();
	return *this;
}

void ParticleEmitter::update(const ostd::Vec2& force)
{
	if (isInvalid()) return;
	sf::VertexArray a;
	if (m_path.isEnabled() && m_path.exists())
	{
		m_currentPathValue += m_pathStep;
		if (m_currentPathValue >= m_path.getTotalLength())
			m_currentPathValue -= m_path.getTotalLength();
		float fOffset = m_path.getNormalisedOffset(m_currentPathValue);
		m_currentPathPoint = m_path.getPoint(fOffset);
		setEmissionRect({ m_currentPathPoint.position, 10.0f, 10.0f });
	}
	int32_t activeParticleCount = 0;
	m_vertexArray.clear();
	for (uint32_t i = 0, k = 0; i < m_particleCount; i++)
	{
		auto& part = m_particles[i];
		if (part.isDead()) continue;
		if (m_useTileArray && m_tileArray.size() > 0)
		{
			float tile = (float)m_tileArray.size() / part.fullLife;
			tile *= part.life;
			uint32_t i = m_tileArray.size() - (uint32_t)std::round(tile);
			if (i < 0) i = 0;
			else if (i >= m_tileArray.size()) i = m_tileArray.size() - 1;
			part.tileIndex = m_tileArray[i];
		}
		if (m_workingRect.w != 0 && m_workingRect.h != 0)
		{
			if (part.position.x + part.size.x < m_workingRect.x ||
				part.position.y + part.size.y < m_workingRect.y ||
				part.position.x > m_workingRect.x + m_workingRect.w ||
				part.position.y > m_workingRect.y + m_workingRect.h)
			{
				part.kill();
				continue;
			}
		}
		activeParticleCount++;
		part.applyForce(force);
		part.physicsUpdate();

		size_t v = k * 6;

        for (int j = 0; j < 6; ++j)
        {
        	m_vertexArray.append(sf::Vertex());
        	m_vertexArray[v + j].color = sf_color(part.color);
        }

        m_vertexArray[v + 0].position = { part.position.x, part.position.y };
        m_vertexArray[v + 1].position = { part.position.x + part.size.x, part.position.y };
        m_vertexArray[v + 2].position = { part.position.x, part.position.y + part.size.y };
        m_vertexArray[v + 3].position = { part.position.x + part.size.x, part.position.y };
        m_vertexArray[v + 4].position = { part.position.x + part.size.x, part.position.y + part.size.y };
        m_vertexArray[v + 5].position = { part.position.x, part.position.y + part.size.y };

        TextureRef::tTexCoords uv;
       	if (part.texture != nullptr)
        	uv = part.texture->getTile(part.tileIndex);

        m_vertexArray[v + 0].texCoords = { uv.topLeft.x,     uv.topLeft.y     };     // TL
        m_vertexArray[v + 1].texCoords = { uv.topRight.x,    uv.topRight.y    };    // TR
        m_vertexArray[v + 2].texCoords = { uv.bottomLeft.x,  uv.bottomLeft.y  };  // BL
        m_vertexArray[v + 3].texCoords = { uv.topRight.x,    uv.topRight.y    };    // TR
        m_vertexArray[v + 4].texCoords = { uv.bottomRight.x, uv.bottomRight.y }; // BR
        m_vertexArray[v + 5].texCoords = { uv.bottomLeft.x,  uv.bottomLeft.y  };  // BL

        k++;
	}
}

void ParticleEmitter::emit(tParticleInfo partInfo, int32_t count)
{
	if (isInvalid()) return;
	if (count <= 0) return;
	uint32_t index = 0;
	// partInfo.angle = 90;
	for (auto& part : m_particles)
	{
		if (part.isDead())
		{
			part.position = getEmissionRect().getPosition() + getRandomEmissionPoint();
			part.setup(partInfo);
			count--;
			if (count <= 0) return;
		}
		index++;
	}
}

void ParticleEmitter::emit(int32_t count)
{
	if (isInvalid()) return;
	emit(m_defaultParticle, count);
}

void ParticleEmitter::setDefaultParticleInfo(tParticleInfo info)
{
	m_defaultParticle = info;
}

void ParticleEmitter::addTilesToArray(const std::vector<TextureRef::TextureAtlasIndex>& array)
{
	for (const auto& tile : array)
		m_tileArray.push_back(tile);
}

ostd::Vec2 ParticleEmitter::getRandomEmissionPoint(void)
{
	return ostd::Random::getVec2({ 0, getEmissionRect().w }, { 0, getEmissionRect().h });
}
// =====================================================================================================







// ======================================== basicFireParticle ==========================================
tParticleInfo ParticleFactory::basicFireParticle(TextureRef::TextureInfo texture)
{
	tParticleInfo info;
	info.texture = texture.texture;
	info.tileIndex = texture.tile;
	info.speed = 2.0f;
	info.randomVelocity = { 0.2f, 0.5f };
	info.randomDirection = 0.14f;
	info.randomAlpha = 0.25;
	info.randomSize = { 0.8f, 0.8f };
	info.size = { 40.0f, 40.0f };
	info.lifeSpan = 120.0f;
	info.color = { 255, 255, 255, 255 };
	info.addColorToGradient({ 161, 1, 0 },   { 218, 31, 5 },  0.08f);  // Bright core → orange
	info.addColorToGradient({ 218, 31, 5 },   { 243, 60, 4 },  0.45f);  // Orange → deep orange
	info.addColorToGradient({ 243, 60, 4 },   { 254, 101, 13 },  0.25f);  // Deep orange → red
	info.addColorToGradient({ 254, 101, 13 },   { 255, 193, 31 },  0.2f);  // Red → dark red
	info.addColorToGradient({ 255, 193, 31 },   { 255, 247, 93 },  0.02f);  // Dark red → ember
	info.angle = 90.0f;
	return info;
}

tParticleInfo ParticleFactory::basicSnowParticle(TextureRef::TextureInfo texture)
{
	tParticleInfo info;
	info.texture = texture.texture;
	info.tileIndex = texture.tile;
	info.speed = 3.0f;
	info.color = { 132, 165, 216 };
	info.randomVelocity.x = 0.0f;
	info.randomDirection = 0.0f;
	info.size = { 16.0f, 16.0f };
	info.randomSize = { 0.3f, 0.3f };
	info.lifeSpan = 100000;
	info.randomDamping = true;
	info.randomLifeSpan = 0.0f;
	info.damping = { 0.08f, 0.0f };
	info.randomAlpha = 0.1f;
	info.angle = -90.0f;
	return info;
}

ParticleEmitter ParticleFactory::basicFireEmitter(TextureRef::TextureInfo texture, ostd::Vec2 position, uint32_t pre_emit_cycles)
{
	ParticleEmitter emitter(ostd::Rectangle(position, 10.0f, 10.0f), 1000);
	emitter.setDefaultParticleInfo(ParticleFactory::basicFireParticle(texture));
	ParticleFactory::__pre_emit(emitter, pre_emit_cycles, { 0.0f, 0.002f });
	return emitter;
}

ParticleEmitter ParticleFactory::basicSnowEmitter(TextureRef::TextureInfo texture, ostd::Vec2 windowSize, uint32_t pre_emit_cycles)
{
	ParticleEmitter emitter(ostd::Rectangle(0, 0, windowSize.x, 1.0f), 2000);
	emitter.setDefaultParticleInfo(ParticleFactory::basicSnowParticle(texture));
	emitter.setWorkingRectangle({ 0.0f, 0.0f, windowSize });
	ParticleFactory::__pre_emit(emitter, pre_emit_cycles, { 0.002, 0.09 });
	return emitter;
}

void ParticleFactory::__pre_emit(ParticleEmitter& emitter, uint32_t pre_emit_cycles, ostd::Vec2 rand_force_range)
{
	uint32_t current = 0;
	ostd::Vec2 wind { 0.0f, 0.0f };
	for (uint32_t i = 0; i < pre_emit_cycles; i++)
	{
		emitter.emit(ostd::Random::geti32(1, 2));
		if (current++ > 30)
		{
			current = 0;
			wind.x = ostd::Random::getf32(rand_force_range.x, rand_force_range.y);
		}
		emitter.update(wind);
	}
}

void ParticleFactory::createColorGradient(tParticleInfo& partInfo, const ostd::Color& startColor, uint8_t nColors)
{
	if (nColors == 0) return;

    partInfo.colorRamp.reset();
    partInfo.colorRamp.m_colors.clear();

    if (nColors == 1)
    {
        partInfo.addColorToGradient(startColor, startColor, 1.0f);
        return;
    }

    const float percentPerStep = 1.0f / static_cast<float>(nColors - 1);

    // Convert to HSV
    float h, s, v;
    Common::RGBtoHSV(startColor.r / 255.0f, startColor.g / 255.0f, startColor.b / 255.0f, h, s, v);

    for (uint8_t i = 0; i < nColors - 1; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(nColors - 1);
        float t_next = static_cast<float>(i + 1) / static_cast<float>(nColors - 1);

        // Brightness: start high (light), peak higher, end slightly lower (soft glow)
        float brightness     = 0.85f + 0.15f * std::sin(t * 3.14159f);         // 0.85 → 1.0 → 0.85
        float brightnessNext = 0.85f + 0.15f * std::sin(t_next * 3.14159f);

        // Saturation: slightly increase then decrease for warmth
        float saturation     = s * (0.7f + 0.3f * std::sin(t * 3.14159f));
        float saturationNext = s * (0.7f + 0.3f * std::sin(t_next * 3.14159f));

        // Clamp
        brightness = std::min(brightness, 1.0f);
        brightnessNext = std::min(brightnessNext, 1.0f);
        saturation = std::min(saturation, 1.0f);
        saturationNext = std::min(saturationNext, 1.0f);

        float r1, g1, b1, r2, g2, b2;
        Common::HSVtoRGB(h, saturation,     brightness,     r1, g1, b1);
        Common::HSVtoRGB(h, saturationNext, brightnessNext, r2, g2, b2);

        ostd::Color current (static_cast<uint8_t>(r1 * 255), static_cast<uint8_t>(g1 * 255),
                             static_cast<uint8_t>(b1 * 255), 255);
        ostd::Color next    (static_cast<uint8_t>(r2 * 255), static_cast<uint8_t>(g2 * 255),
                             static_cast<uint8_t>(b2 * 255), 255);

        partInfo.addColorToGradient(current, next, percentPerStep);
    }
        // std::cout << current << "\n" << next << "\n\n";
    // exit(0);
}
// =====================================================================================================
