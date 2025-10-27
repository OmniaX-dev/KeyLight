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
#include <SFML/Graphics.hpp>
#include <cmath>
#include <array>

class RoundedRectangleShape : public sf::Shape {
public:
    RoundedRectangleShape(sf::Vector2f size = {0.f, 0.f},
                          float radiusTopLeft = 0.f,
                          float radiusTopRight = 0.f,
                          float radiusBottomRight = 0.f,
                          float radiusBottomLeft = 0.f,
                          std::size_t cornerPointCount = 8)
    {
        setSize(size);
        setCornerRadii(radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
        setCornerPointCount(cornerPointCount);
    }

    void setSize(sf::Vector2f size) {
        m_size = size;
        update();
    }

    sf::Vector2f getSize() const { return m_size; }

    void setCornerRadii(float topLeft, float topRight, float bottomRight, float bottomLeft) {
        m_radii[0] = topLeft;
        m_radii[1] = topRight;
        m_radii[2] = bottomRight;
        m_radii[3] = bottomLeft;
        update();
    }

    void setCornerPointCount(std::size_t count) {
        m_cornerPointCount = std::max<std::size_t>(2, count); // at least 2 points per corner
        update();
    }

    std::size_t getPointCount() const override {
        return m_cornerPointCount * 4;
    }

    sf::Vector2f getPoint(std::size_t index) const override {
        static constexpr float pi = 3.141592654f;

        std::size_t corner = index / m_cornerPointCount;
        float radius = m_radii[corner];
        float angleStep = (pi / 2.f) / static_cast<float>(m_cornerPointCount - 1);
        float angle = (index % m_cornerPointCount) * angleStep;

        sf::Vector2f center;
        switch (corner) {
            case 0: // Top-left
                center = { radius, radius };
                angle += pi; // 180°
                break;
            case 1: // Top-right
                center = { m_size.x - radius, radius };
                angle += 1.5f * pi; // 270°
                break;
            case 2: // Bottom-right
                center = { m_size.x - radius, m_size.y - radius };
                angle += 0.f; // 0°
                break;
            case 3: // Bottom-left
                center = { radius, m_size.y - radius };
                angle += pi / 2.f; // 90°
                break;
        }

        return {
            center.x + std::cos(angle) * radius,
            center.y + std::sin(angle) * radius
        };
    }

private:
    sf::Vector2f m_size;
    std::array<float, 4> m_radii {0.f, 0.f, 0.f, 0.f}; // TL, TR, BR, BL
    std::size_t m_cornerPointCount = 8;
};
