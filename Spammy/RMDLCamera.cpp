/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLCamera.cpp            +++     +++         **/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 18/09/2025 18:56:40      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "RMDLCamera.hpp"

static simd::float4x4 sInvMatrixLookat( simd::float3 inEye, simd::float3 inTo, simd::float3 inUp )
{
    simd::float3 z = simd::normalize(inTo - inEye);
    simd::float3 x = simd::normalize(simd::cross(inUp, z));
    simd::float3 y = simd::cross(z, x);
    simd::float3 t = (simd::float3) { -simd::dot(x, inEye), -simd::dot(y, inEye), -simd::dot(z, inEye) };
    return ( simd::float4x4( simd::float4 { x.x, y.x, z.x, 0 },
                             simd::float4 { x.y, y.y, z.y, 0 },
                             simd::float4 { x.z, y.z, z.z, 0 },
                             simd::float4 { t.x, t.y, t.z, 1 }) );
}

static simd::float4 sPlaneNormalize( const simd::float4& inPlane )
{
    return (inPlane / simd::length(inPlane.xyz));
}

RMDLCamera::RMDLCamera() : _position{0, 0, 0}, _direction{0, 0, 1}, _up{0, 1, 0}, _viewAngle(0), _aspectRatio(1.0), _nearPlane(0.1f), _farPlane(100.0f), _width(0), _uniformsDirty(true)
{
}

RMDLCamera::~RMDLCamera()
{
}

RMDLCamera& RMDLCamera::initPerspectiveWithPosition(simd::float3 position, simd::float3 direction, simd::float3 up, float viewAngle, float aspectRatio, float nearPlane, float farPlane)
{
    _up = up;
    orthogonalizeFromNewForward(direction);
    _position = position;
    _width = 0;
    _viewAngle = viewAngle;
    _aspectRatio = aspectRatio;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _uniformsDirty = true;
    return *this;
}
    
RMDLCamera& RMDLCamera::initParallelWithPosition(simd::float3 position, simd::float3 direction, simd::float3 up, float width, float height, float nearPlane, float farPlane)
{
    _up = up;
    orthogonalizeFromNewForward(direction);
    _position = position;
    _width = width;
    _viewAngle = 0;
    _aspectRatio = width / height;
    _nearPlane = nearPlane;
    _farPlane = farPlane;
    _uniformsDirty = true;
    return *this;
}
    
bool RMDLCamera::isPerspective() const
{
    return (_viewAngle != 0.0f);
}

bool RMDLCamera::isParallel() const
{
    return (_viewAngle == 0.0f);
}

void RMDLCamera::updateUniforms()
{
    simd::float3 pos = _position + _shakeOffset;
    _uniforms.viewMatrix = sInvMatrixLookat(pos, pos + _direction, _up);
    //_uniforms.viewMatrix = sInvMatrixLookat(_position, _position + _direction, _up);
    if (_viewAngle != 0)
    {
        float va_tan = 1.0f / tan(_viewAngle * 0.5f);
        float ys = va_tan;
        float xs = ys / _aspectRatio;
        float zs = _farPlane / (_farPlane - _nearPlane);
        _uniforms.projectionMatrix = simd::float4x4((simd::float4){ xs,  0,  0, 0},
                                                    (simd::float4){  0, ys,  0, 0},
                                                    (simd::float4){  0,  0, zs, 1},
                                                    (simd::float4){  0,  0, -_nearPlane * zs , 0 } );
    }
    else
    {
        float ys = 2.0f / _width;
        float xs = ys / _aspectRatio;
        float zs = 1.0f / (_farPlane - _nearPlane);
        _uniforms.projectionMatrix = simd::float4x4((simd::float4){ xs,  0,  0, 0},
                                                    (simd::float4){  0, ys,  0, 0},
                                                    (simd::float4){  0,  0, zs, 1},
                                                    (simd::float4){  0,  0, -_nearPlane * zs , 1} );
    }
    _uniforms.viewProjectionMatrix = _uniforms.projectionMatrix * _uniforms.viewMatrix;
    _uniforms.invProjectionMatrix = simd_inverse(_uniforms.projectionMatrix);
    _uniforms.invOrientationProjectionMatrix = simd_inverse( _uniforms.projectionMatrix * sInvMatrixLookat( simd::float3{0, 0, 0}, _direction, _up ) );
    _uniforms.invViewProjectionMatrix = simd_inverse(_uniforms.viewProjectionMatrix);
    _uniforms.invViewMatrix = simd_inverse(_uniforms.viewMatrix);
    simd::float4x4 transp_vpm = simd::transpose(_uniforms.viewProjectionMatrix);
    _uniforms.frustumPlanes[0] = sPlaneNormalize(transp_vpm.columns[3] + transp_vpm.columns[0]);
    _uniforms.frustumPlanes[1] = sPlaneNormalize(transp_vpm.columns[3] - transp_vpm.columns[0]);
    _uniforms.frustumPlanes[2] = sPlaneNormalize(transp_vpm.columns[3] + transp_vpm.columns[1]);
    _uniforms.frustumPlanes[3] = sPlaneNormalize(transp_vpm.columns[3] - transp_vpm.columns[1]);
    _uniforms.frustumPlanes[4] = sPlaneNormalize(transp_vpm.columns[3] + transp_vpm.columns[2]);
    _uniforms.frustumPlanes[5] = sPlaneNormalize(transp_vpm.columns[3] - transp_vpm.columns[2]);
    _uniformsDirty = false;
}

simd::float3 RMDLCamera::left() const
{
    return (simd_cross(_direction, _up));
}

simd::float3 RMDLCamera::right() const
{
    return (-left());
}

simd::float3 RMDLCamera::down() const
{
    return (-_up);
}

simd::float3 RMDLCamera::forward() const
{
    return (_direction);
}

simd::float3 RMDLCamera::backward() const
{
    return (-_direction);
}

float RMDLCamera::nearPlane() const
{
    return (_nearPlane);
}

float RMDLCamera::farPlane() const
{
    return (_farPlane);
}

float RMDLCamera::aspectRatio() const
{
    return (_aspectRatio);
}

float RMDLCamera::viewAngle() const
{
    return (_viewAngle);
}

float RMDLCamera::width() const
{
    return (_width);
}

simd::float3 RMDLCamera::up() const
{
    return (_up);
}

simd::float3 RMDLCamera::position() const
{
    return (_position);
}

simd::float3 RMDLCamera::direction() const
{
    return (_direction);
}
    
RMDLCameraUniforms RMDLCamera::uniforms()
{
    if (_uniformsDirty)
        RMDLCamera::updateUniforms();
    return (_uniforms);
}
    
void RMDLCamera::setNearPlane(float newNearPlane)
{
    _nearPlane = newNearPlane;
    _uniformsDirty = true;
}

void RMDLCamera::setFarPlane(float newFarPlane)
{
    _farPlane = newFarPlane;
    _uniformsDirty = true;
}

void RMDLCamera::setAspectRatio(float newAspectRatio)
{
    _aspectRatio = newAspectRatio;
    _uniformsDirty = true;
}

void RMDLCamera::setViewAngle(float newAngle)
{
    _width = 0;
    _viewAngle = newAngle;
    _uniformsDirty = true;
}

void RMDLCamera::setWidth(float newWidth)
{
    _viewAngle = 0;
    _width = newWidth;
    _uniformsDirty = true;
}

void RMDLCamera::setPosition( simd::float3 newPosition )
{
    _position = newPosition;
    _uniformsDirty = true;
}

void RMDLCamera::setUp( simd::float3 newUp )
{
    orthogonalizeFromNewUp(newUp);
    _uniformsDirty = true;
}

void RMDLCamera::setDirection( simd::float3 newDirection )
{
    orthogonalizeFromNewForward(newDirection);
    _uniformsDirty = true;
}

simd::float4x4 RMDLCamera::ViewMatrix()
{
    return (uniforms().viewMatrix);
}

simd::float4x4 RMDLCamera::getProjectionMatrix()
{
    return (uniforms().projectionMatrix);
}

simd::float4x4 RMDLCamera::ViewProjectionMatrix()
{
    return (uniforms().viewProjectionMatrix);
}

simd::float4x4 RMDLCamera::InvOrientationProjectionMatrix()
{
    return (uniforms().invOrientationProjectionMatrix);
}

simd::float4x4 RMDLCamera::InvViewProjectionMatrix()
{
    return (uniforms().invViewProjectionMatrix);
}

simd::float4x4 RMDLCamera::InvProjectionMatrix()
{
    return (uniforms().invProjectionMatrix);
}

simd::float4x4 RMDLCamera::InvViewMatrix()
{
    return (uniforms().invViewMatrix);
}
    
void RMDLCamera::rotateOnAxis( simd::float3 inAxis, float inRadians )
{
    simd::float3 axis = simd::normalize(inAxis);
    float ct = cosf(inRadians);
    float st = sinf(inRadians);
    float ci = 1 - ct;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    simd::float3x3 mat( (simd::float3){ ct + x * x * ci, y * x * ci + z * st, z * x * ci - y * st } ,
                        (simd::float3){ x * y * ci - z * st, ct + y * y * ci, z * y * ci + x * st } ,
                        (simd::float3){ x * z * ci + y * st, y * z * ci - x * st, ct + z * z * ci } );
    _direction = mat * _direction;
    _up = mat * _up;
    _uniformsDirty = true;
}

void RMDLCamera::orthogonalizeFromNewUp( simd::float3 newUp )
{
    _up = simd::normalize(newUp);
    simd::float3 right = simd::normalize( simd::cross(_direction, _up) );
    _direction = simd::cross(_up, right);
}
    
void RMDLCamera::orthogonalizeFromNewForward( simd::float3 newForward )
{
    _direction = simd::normalize(newForward);
    simd::float3 right = simd::normalize(simd::cross(_direction, _up));
    _up = simd::cross(right, _direction);
}

void RMDLCamera::rotateYawPitch(float deltaYaw, float deltaPitch)
{
    _yaw += deltaYaw;
    _pitch += deltaPitch;
    
    // Wrap yaw entre -PI et PI
    while (_yaw > M_PI) _yaw -= 2.0f * M_PI;
    while (_yaw < -M_PI) _yaw += 2.0f * M_PI;
    
    _pitch = simd::clamp(_pitch, _pitchMin, _pitchMax);
    
    if (_orbitMode) {
        updateOrbitPosition();
    } else {
        updateDirectionFromYawPitch();
    }
}

void RMDLCamera::setYaw(float yaw)
{
    _yaw = yaw;
    if (_orbitMode) updateOrbitPosition();
    else updateDirectionFromYawPitch();
}

void RMDLCamera::setPitch(float pitch)
{
    _pitch = simd::clamp(pitch, _pitchMin, _pitchMax);
    if (_orbitMode) updateOrbitPosition();
    else updateDirectionFromYawPitch();
}

void RMDLCamera::setPitchLimits(float minPitch, float maxPitch)
{
    _pitchMin = minPitch;
    _pitchMax = maxPitch;
    _pitch = simd::clamp(_pitch, _pitchMin, _pitchMax);
}

void RMDLCamera::updateDirectionFromYawPitch()
{
    // Calculer direction depuis yaw/pitch
    float cosP = cosf(_pitch);
    _direction = simd::float3{
        cosP * sinf(_yaw),
        sinf(_pitch),
        cosP * cosf(_yaw)
    };
    _direction = simd::normalize(_direction);
    
    // Recalculer up pour rester orthogonal
    simd::float3 worldUp = {0, 1, 0};
    simd::float3 right = simd::normalize(simd::cross(_direction, worldUp));
    _up = simd::cross(right, _direction);
    
    _uniformsDirty = true;
}

void RMDLCamera::setOrbitMode(bool enabled)
{
    _orbitMode = enabled;
    if (enabled) {
        // Calculer yaw/pitch depuis position actuelle
        simd::float3 toTarget = _orbitTarget - _position;
        _orbitDistance = simd::length(toTarget);
        if (_orbitDistance > 0.001f) {
            simd::float3 dir = toTarget / _orbitDistance;
            _yaw = atan2f(dir.x, dir.z);
            _pitch = -asinf(simd::clamp(dir.y, -1.0f, 1.0f));
        }
    }
}

void RMDLCamera::setOrbitTarget(simd::float3 target)
{
    _orbitTarget = target;
    if (_orbitMode) updateOrbitPosition();
}

void RMDLCamera::setOrbitDistance(float distance)
{
    _orbitDistance = simd::max(0.1f, distance);
    if (_orbitMode) updateOrbitPosition();
}

void RMDLCamera::orbit(float deltaYaw, float deltaPitch)
{
    if (!_orbitMode) {
        setOrbitMode(true);
    }
    rotateYawPitch(deltaYaw, deltaPitch);
}

void RMDLCamera::zoom(float delta)
{
    _orbitDistance = simd::max(0.5f, _orbitDistance - delta);
    if (_orbitMode) updateOrbitPosition();
}

void RMDLCamera::updateOrbitPosition()
{
    // Position sur sphère autour de la cible
    float cosP = cosf(_pitch);
    simd::float3 offset = {
        cosP * sinf(_yaw) * _orbitDistance,
        sinf(_pitch) * _orbitDistance,
        cosP * cosf(_yaw) * _orbitDistance
    };
    
    _position = _orbitTarget + offset;
    _direction = simd::normalize(_orbitTarget - _position);
    
    // Recalculer up
    simd::float3 worldUp = {0, 1, 0};
    simd::float3 right = simd::normalize(simd::cross(_direction, worldUp));
    _up = simd::cross(right, _direction);
    
    _uniformsDirty = true;
}

void RMDLCamera::followTarget(simd::float3 target, float distance, float height, float smoothing, float dt)
{
    // Position idéale derrière la cible
    simd::float3 idealPos = target - _direction * distance + simd::float3{0, height, 0};
    
    // Interpolation smooth
    float t = 1.0f - powf(smoothing, dt);
    _position = _position + (idealPos - _position) * t;
    
    // Regarder la cible
    lookAt(target);
}

void RMDLCamera::lookAt(simd::float3 target)
{
    simd::float3 toTarget = target - _position;
    float dist = simd::length(toTarget);
    
    if (dist < 0.001f) return;
    
    _direction = toTarget / dist;
    
    // Calculer yaw/pitch depuis direction
    _yaw = atan2f(_direction.x, _direction.z);
    _pitch = asinf(simd::clamp(_direction.y, -1.0f, 1.0f));
    
    // Orthogonaliser up
    simd::float3 worldUp = {0, 1, 0};
    simd::float3 right = simd::normalize(simd::cross(_direction, worldUp));
    _up = simd::cross(right, _direction);
    
    _uniformsDirty = true;
}

// Surcharge pour rotation avec deux floats (plus pratique)
void RMDLCamera::rotateOnAxisOrbit(float yawDelta, float pitchDelta)
{
    rotateYawPitch(yawDelta, pitchDelta);
}

RMDLCameraSnapshot RMDLCamera::snapshot() const
{
    return { _position, _direction, _up, _viewAngle, _nearPlane, _farPlane, _orbitTarget, _orbitDistance, _yaw, _pitch };
}

void RMDLCamera::applySnapshot(const RMDLCameraSnapshot& snap)
{
    _position = snap.position;
    _direction = snap.direction;
    _up = snap.up;
    _viewAngle = snap.viewAngle;
    _nearPlane = snap.nearPlane;
    _farPlane = snap.farPlane;
    _orbitTarget = snap.orbitTarget;
    _orbitDistance = snap.orbitDistance;
    _yaw = snap.yaw;
    _pitch = snap.pitch;
    _uniformsDirty = true;
}

void RMDLCamera::transitionTo(const RMDLCameraSnapshot& target, float duration, RMDLCameraEase ease, std::function<void()> onComplete)
{
    _transitionFrom = snapshot();
    _transitionTo = target;
    _transitionDuration = simd::max(0.001f, duration);
    _transitionElapsed = 0.0f;
    _transitionEase = ease;
    _transitionActive = true;
    _transitionOnComplete = onComplete;
}

void RMDLCamera::transitionTo(const RMDLCamera& other, float duration, RMDLCameraEase ease, std::function<void()> onComplete)
{
    transitionTo(other.snapshot(), duration, ease, onComplete);
}

void RMDLCamera::updateTransition(float dt)
{
    if (_shakeDuration > 0.0f)
    {
        _shakeElapsed += dt;
        if (_shakeElapsed < _shakeDuration)
        {
            float decay = 1.0f - (_shakeElapsed / _shakeDuration);
            float phase = _shakeElapsed * _shakeFrequency;
            _shakeOffset = {
                sinf(phase) * _shakeIntensity * decay,
                cosf(phase * 1.3f) * _shakeIntensity * decay * 0.7f,
                sinf(phase * 0.7f) * _shakeIntensity * decay * 0.3f
            };
            _uniformsDirty = true;
        }
        else
        {
            _shakeDuration = 0.0f;
            _shakeOffset = {0, 0, 0};
            _uniformsDirty = true;
        }
    }
    
    if (!_transitionActive) return;
    
    _transitionElapsed += dt;
    float t = simd::min(1.0f, _transitionElapsed / _transitionDuration);
    float easedT = applyEase(t, _transitionEase);
    
    // Lerp position
    _position = _transitionFrom.position + easedT * (_transitionTo.position - _transitionFrom.position);
    
    // Slerp direction
    _direction = slerpDirection(_transitionFrom.direction, _transitionTo.direction, easedT);
    
    // Slerp up
    _up = slerpDirection(_transitionFrom.up, _transitionTo.up, easedT);
    
    // Lerp scalaires
    _viewAngle = _transitionFrom.viewAngle + easedT * (_transitionTo.viewAngle - _transitionFrom.viewAngle);
    _nearPlane = _transitionFrom.nearPlane + easedT * (_transitionTo.nearPlane - _transitionFrom.nearPlane);
    _farPlane = _transitionFrom.farPlane + easedT * (_transitionTo.farPlane - _transitionFrom.farPlane);
    
    // Orbit params
    _orbitTarget = _transitionFrom.orbitTarget + easedT * (_transitionTo.orbitTarget - _transitionFrom.orbitTarget);
    _orbitDistance = _transitionFrom.orbitDistance + easedT * (_transitionTo.orbitDistance - _transitionFrom.orbitDistance);
    _yaw = _transitionFrom.yaw + easedT * (_transitionTo.yaw - _transitionFrom.yaw);
    _pitch = _transitionFrom.pitch + easedT * (_transitionTo.pitch - _transitionFrom.pitch);
    
    _uniformsDirty = true;
    
    if (t >= 1.0f)
    {
        _transitionActive = false;
        applySnapshot(_transitionTo);
        if (_transitionOnComplete) _transitionOnComplete();
    }
}

float RMDLCamera::applyEase(float t, RMDLCameraEase ease) const
{
    switch (ease)
    {
        case RMDLCameraEase::Linear:
            return t;
        case RMDLCameraEase::SmoothStep:
            return t * t * (3.0f - 2.0f * t);
        case RMDLCameraEase::SmootherStep:
            return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        case RMDLCameraEase::EaseInQuad:
            return t * t;
        case RMDLCameraEase::EaseOutQuad:
            return t * (2.0f - t);
        case RMDLCameraEase::EaseInOutQuad:
            return (t < 0.5f) ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        case RMDLCameraEase::EaseInCubic:
            return t * t * t;
        case RMDLCameraEase::EaseOutCubic:
        {
            float f = t - 1.0f;
            return f * f * f + 1.0f;
        }
        case RMDLCameraEase::EaseInOutCubic:
            return (t < 0.5f) ? 4.0f * t * t * t
                             : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        case RMDLCameraEase::EaseInOutBack:
        {
            const float c1 = 1.70158f;
            const float c2 = c1 * 1.525f;
            return (t < 0.5f)
                    ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
                    : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
        }
    }
    return t;
}

simd::float3 RMDLCamera::slerpDirection(simd::float3 a, simd::float3 b, float t) const
{
    a = simd::normalize(a);
    b = simd::normalize(b);
    float dot = simd::dot(a, b);
    
    // Si presque parallèles, lerp simple
    if (dot > 0.9995f)
        return simd::normalize(a + t * (b - a));
    
    dot = simd::clamp(dot, -1.0f, 1.0f);
    float theta = acosf(dot) * t;
    simd::float3 relative = simd::normalize(b - a * dot);
    return a * cosf(theta) + relative * sinf(theta);
}

void RMDLCamera::applyShake(float intensity, float duration, float frequency)
{
    _shakeIntensity = intensity;
    _shakeDuration = duration;
    _shakeElapsed = 0.0f;
    _shakeFrequency = frequency;
}
