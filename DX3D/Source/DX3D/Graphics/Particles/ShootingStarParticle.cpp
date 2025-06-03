#include <DX3D/Graphics/Particles/ShootingStarParticle.h>
#include <cmath>
#include <algorithm>


using namespace dx3d;

StarTrailParticle::StarTrailParticle()
{
    // Trail particle defaults
    m_maxLife = 1.0f;
    m_fadeSpeed = 2.0f;

    // Star colors (bright white/yellow to dim blue)
    m_startColor = Vec4(1.0f, 1.0f, 0.8f, 1.0f);  // Bright yellow-white
    m_endColor = Vec4(0.4f, 0.6f, 1.0f, 0.0f);    // Dim blue, transparent

    m_startSize = 0.06f;
    m_endSize = 0.01f;
}

void StarTrailParticle::update(float deltaTime)
{
    if (!m_active || m_life <= 0.0f)
        return;

    // Base particle update
    Particle::update(deltaTime);

    // Trail-specific behavior
    float lifeRatio = getLifeRatio();

    // Add sparkle effect
    m_sparkle = std::sin(m_life * 20.0f) * 0.5f + 0.5f;

    // Adjust brightness based on trail position and sparkle
    float brightness = lifeRatio * (0.7f + 0.3f * m_sparkle);
    m_color.w = m_startColor.w * brightness;

    // Slightly curve the trail
    float curveFactor = (1.0f - lifeRatio) * 0.1f;
    m_velocity.y -= curveFactor * deltaTime;
}

void StarTrailParticle::reset(const Vec2& position, const Vec2& velocity)
{
    Particle::reset(position, velocity);
    m_sparkle = 1.0f;
}

ShootingStarEmitter::ShootingStarEmitter(size_t maxParticles)
    : ParticleEmitter(maxParticles)
{
}

void ShootingStarEmitter::update(float deltaTime)
{
    ParticleEmitter::update(deltaTime);

    // Update the main star
    if (m_starActive)
    {
        m_starLife -= deltaTime;
        if (m_starLife <= 0.0f)
        {
            m_starActive = false;
            return;
        }

        // Move the star
        m_starPosition += m_starVelocity * deltaTime;

        // Emit trail particles
        m_trailTimer += deltaTime;
        while (m_trailTimer >= m_trailEmissionRate)
        {
            m_trailTimer -= m_trailEmissionRate;
            emitTrail();
        }
    }
}

void ShootingStarEmitter::launchStar(const Vec2& startPos, const Vec2& direction, float speed)
{
    m_starPosition = startPos;
    m_starVelocity = direction.normalized() * speed;
    m_starActive = true;
    m_starLife = m_starMaxLife;
    m_currentTrailIndex = 0;
    m_trailTimer = 0.0f;

    // Clear existing particles for a fresh start
    m_particles.clear();
}

void ShootingStarEmitter::emitTrail()
{
    if (!m_starActive)
        return;

    // Emit multiple particles for the trail
    int particlesPerEmission = 3;

    for (int i = 0; i < particlesPerEmission; ++i)
    {
        if (m_particles.size() >= m_maxParticles)
            break;

        auto particle = createParticle();
        if (particle)
        {
            initializeParticle(particle.get(), m_starPosition);

            // Set trail index for staggered fade
            StarTrailParticle* trailParticle = static_cast<StarTrailParticle*>(particle.get());
            trailParticle->setTrailIndex(m_currentTrailIndex++);

            m_particles.push_back(std::move(particle));
        }
    }
}

std::unique_ptr<Particle> ShootingStarEmitter::createParticle()
{
    return std::make_unique<StarTrailParticle>();
}

void ShootingStarEmitter::initializeParticle(Particle* particle, const Vec2& emitPosition)
{
    StarTrailParticle* trailParticle = static_cast<StarTrailParticle*>(particle);

    // Slight offset for particle spread
    Vec2 offset = randomDirection() * randomFloat(0.0f, 0.02f);
    Vec2 pos = emitPosition + offset;

    // Trail particles have reduced velocity from the main star
    Vec2 vel = m_starVelocity * randomFloat(0.05f, 0.15f);

    // Add some perpendicular spread
    Vec2 perpendicular(-m_starVelocity.y, m_starVelocity.x);
    perpendicular = perpendicular.normalized() * randomFloat(-0.1f, 0.1f);
    vel += perpendicular;

    trailParticle->reset(pos, vel);

    // Set trail properties using setters
    float lifeFactor = 1.0f - (m_starLife / m_starMaxLife);
    float maxLife = randomFloat(0.5f, 1.2f) * (1.0f - lifeFactor * 0.5f);
    trailParticle->setLifetime(maxLife);

    // Size based on position in trail
    float sizeFactor = 1.0f - (static_cast<float>(trailParticle->getTrailIndex() % 10) / 10.0f);
    float startSize = m_starSize * randomFloat(0.6f, 1.0f) * sizeFactor;
    trailParticle->setSizeRange(startSize, startSize * 0.1f);

    // Slight rotation
    trailParticle->setRotationSpeed(randomFloat(-1.0f, 1.0f));
}