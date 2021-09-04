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

    bool Tick(Real dt, Point forceSince) {
        ProcessForce(dt, forceSince);
        ProcessFriction(dt);
        ProcessDrag(dt);

        if (!ClampVelocity()) return false;
        position += velocity * dt;
        return true;
    }

    void SetEnvironment(Environment v) { this->environment = v; }

    void SetProperties(Properties v) { this->properties = v; }

    Point velocity{0};

    Point position{0};

    /**
     * Input new orientation in [yaw, pitch, roll]
     * Not a dumb setter, will make a rotation matrix aswell
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

    Point orientation{0, 0, 1};
    Math::Matrix<Real, 3, 3> rotationMatrix{};

    Environment environment{
      .gravity = 9.80665,
      .density = 1.225,
    };

    Properties properties{7874, .05, 0.47,};

    [[nodiscard]] bool ClampVelocity() {
        if (!HaveVelocity()) {
            velocity = {0};
            return false;
        }

        return true;
    }

    [[nodiscard]] bool HaveVelocity() const {
        return !std::all_of(velocity.data, velocity.data + velocity.size(), [](auto v) {
            return std::abs(v) <= 0.01;
        });
    }

    void ProcessForce(Real dt, Point forceSince) {
        //f = ma, a = f/m, v = ft/m
        velocity += forceSince * dt / properties.mass;
    }

    void ProcessFriction(Real dt) {
        //f = mgk, v = ft/m, v = mgkt/m = gkt
        Real maxVDelta = environment.gravity * muSK.second * dt;

        if (!ClampVelocity()) { return; }

        Point applyDirection = -Math::Ops::Vector::Normalized(velocity);
        applyDirection *= std::min(velocity.Magnitude(), maxVDelta);

        velocity += applyDirection;
    }

    void ProcessDrag(Real dt) {
        if (!HaveVelocity()) return;

        //Fd = 0.5 * rho * v^2 * cd * A
        auto absV = velocity.Magnitude();

        auto force = 0.5
                     * environment.density
                     * absV * absV
                     * properties.dragCoefficient
                     * properties.projectedArea;

        velocity -= Math::Ops::Vector::Normalized(velocity) * (force * dt / properties.mass);
    }
};

class CameraBodySFML : public CameraBody {
  public:
    bool TickSFML(Real dt, Real force, const std::array<sf::Keyboard::Key, 10> &keybindings = {
      sf::Keyboard::W,
      sf::Keyboard::A,
      sf::Keyboard::S,
      sf::Keyboard::D,
      sf::Keyboard::LShift,
      sf::Keyboard::Space,
      sf::Keyboard::Up,
      sf::Keyboard::Left,
      sf::Keyboard::Down,
      sf::Keyboard::Right,
    }) {
        Point inputVector{0};

        inputVector[2] += sf::Keyboard::isKeyPressed(keybindings[0]);
        inputVector[0] -= sf::Keyboard::isKeyPressed(keybindings[1]);
        inputVector[2] -= sf::Keyboard::isKeyPressed(keybindings[2]);
        inputVector[0] += sf::Keyboard::isKeyPressed(keybindings[3]);
        inputVector[1] -= sf::Keyboard::isKeyPressed(keybindings[4]);
        inputVector[1] += sf::Keyboard::isKeyPressed(keybindings[5]);

        if (!(inputVector[0] == 0 && inputVector[1] == 0 && inputVector[2] == 0)) {
            inputVector = Math::Ops::Vector::Normalized(inputVector) * force;
        }

        return CameraBody::Tick(dt, inputVector);
    }

  private:
};

}
