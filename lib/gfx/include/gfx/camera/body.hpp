#pragma once

#include <gfx/definitions.hpp>

#include <SFML/Graphics.hpp>

namespace Gfx {

class CameraBody {
  public:
    struct Environment {
        Real gravity; //gravity in m/s^2
        Real density; //fluid density in kg/m^3
    };

    struct Properties {
        Properties(Real density, Real radius, Real dragCoefficient)
          : density(density)
            , radius(radius)
            , dragCoefficient(dragCoefficient)
            , volume((4. / 3.) * M_PI * radius * radius * radius)
            , projectedArea(M_PI * radius * radius)
            , mass(volume * density) {}

        Real density; //kg/m^3
        Real radius; //m
        Real dragCoefficient; //drag coefficient, unitless

        Real volume; //m^3
        Real projectedArea; //m^2
        Real mass; //kg
    };

    bool Tick(Real dt, Point forceSince);

    void SetEnvironment(Environment v) { this->environment = v; }

    void SetProperties(Properties v) { this->properties = v; }

    Point velocity{{0}};

    Point position{{0}};

    /**
     * Input new orientation in [yaw, pitch, roll]
     * Not a dumb setter, will create a rotation matrix as well
     * @param orient The new orientation
     */
    void Orientation(Point orient) {
        orientation = orient;
        rotationMatrix = Math::Matrix<Real, 3, 3>::Rotation(orientation[0], orientation[1], orientation[2]);
    }

    /**
     * Effectively does Orientation(Orientation() + orient);
     * @param orient Orientation offset
     */
    void Orient(Point orient) { return Orientation(Orientation() + orient); }

    [[nodiscard]] Point Orientation() const { return orientation; }

    [[nodiscard]] Math::Matrix<Real, 3, 3> Rotation() const { return rotationMatrix; }

  private:
    std::pair<Real, Real> muSK{0., 0.06};

    Point orientation{{0, 0, 0}};
    Math::Matrix<Real, 3, 3> rotationMatrix{{1, 0, 0, 0, 1, 0, 0, 0, 1}};

    Environment environment{
      .gravity = 9.80665,
      .density = 1.225,
    };

    Properties properties{7874, .05, 0.47,};


    [[nodiscard]] bool HaveVelocity() const;

    [[nodiscard]] bool ClampVelocity();

    void ProcessForce(Real dt, Point forceSince);

    void ProcessFriction(Real dt);

    void ProcessDrag(Real dt);
};

class CameraBodySFML : public CameraBody {
  public:
    bool TickSFML(const sf::Window &window, Real dt, Real force, const std::array<sf::Keyboard::Key, 10> &keybindings = {
      sf::Keyboard::W,
      sf::Keyboard::A,
      sf::Keyboard::S,
      sf::Keyboard::D,
      sf::Keyboard::LShift,
      sf::Keyboard::Space,
      sf::Keyboard::Up,
      sf::Keyboard::Down,
      sf::Keyboard::Left,
      sf::Keyboard::Right,
    });

  private:
};

}
