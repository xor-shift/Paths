#include <gfx/camera/body.hpp>

namespace Gfx {

bool CameraBody::Tick(Real dt, Point forceSince) {
    ProcessForce(dt, forceSince);
    ProcessFriction(dt);
    ProcessDrag(dt);

    if (!ClampVelocity()) return false;
    position += velocity * dt;
    return true;
}

bool CameraBody::HaveVelocity() const {
    return std::any_of(velocity.data(), velocity.data() + velocity.size(), [](auto v) {
        return std::abs(v) >= 0.01;
    });
}

bool CameraBody::ClampVelocity() {
    if (!HaveVelocity()) {
        velocity = {{0}};
        return false;
    }

    return true;
}

void CameraBody::ProcessForce(Real dt, Point forceSince) {
    //f = ma, a = f/m, v = ft/m
    velocity += Point{forceSince * dt / properties.mass};
}

void CameraBody::ProcessFriction(Real dt) {
    if (!ClampVelocity()) { return; }

    //f = mgk, v = ft/m, v = mgkt/m = gkt
    const Real maxVDelta = environment.gravity * muSK.second * dt;

    const Point applyDirection = -Math::Normalized(velocity) * std::min(Math::Magnitude(velocity), maxVDelta);

    velocity += applyDirection;
}

void CameraBody::ProcessDrag(Real dt) {
    if (!HaveVelocity()) return;

    //Fd = 0.5 * rho * v^2 * cd * A
    const auto absV = Math::Magnitude(velocity);

    auto force = 0.5
                 * environment.density
                 * absV * absV
                 * properties.dragCoefficient
                 * properties.projectedArea;

    velocity -= Math::Normalized(velocity) * (force * dt / properties.mass);
}

bool CameraBodySFML::TickSFML(const sf::Window &window, Real dt, Real force, const std::array<sf::Keyboard::Key, 10> &keybindings) {
    if (window.hasFocus()) {
        Point inputVector{{0}};

        inputVector[2] += sf::Keyboard::isKeyPressed(keybindings[0]);
        inputVector[0] -= sf::Keyboard::isKeyPressed(keybindings[1]);
        inputVector[2] -= sf::Keyboard::isKeyPressed(keybindings[2]);
        inputVector[0] += sf::Keyboard::isKeyPressed(keybindings[3]);
        inputVector[1] -= sf::Keyboard::isKeyPressed(keybindings[4]);
        inputVector[1] += sf::Keyboard::isKeyPressed(keybindings[5]);

        static constexpr Real rotDegPS = M_PI * 0.2;
        const Real rotDeg = rotDegPS * dt;

        Point orient{{0}};
        orient[1] -= rotDeg * sf::Keyboard::isKeyPressed(keybindings[6]);
        orient[1] += rotDeg * sf::Keyboard::isKeyPressed(keybindings[7]);
        orient[0] -= rotDeg * sf::Keyboard::isKeyPressed(keybindings[8]);
        orient[0] += rotDeg * sf::Keyboard::isKeyPressed(keybindings[9]);
        bool changedOrientation = (orient[0] != 0 || orient[1] != 0 || orient[2] != 0);
        if (changedOrientation) {
            Orient(orient);
        }

        if (!(inputVector[0] == 0 && inputVector[1] == 0 && inputVector[2] == 0)) {
            inputVector = (Math::Normalized(inputVector) * force) * Rotation();
        }

        return CameraBody::Tick(dt, inputVector) || changedOrientation;
    } else {
        return CameraBody::Tick(dt, {{0}});
    }
}

}
