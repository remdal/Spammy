/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                        +       +          */
/*      File: RMDLCamera.hpp            +++     +++         **/
/*                                        +       +          */
/*      By: Laboitederemdal      **        +       +        **/
/*                                       +           +       */
/*      Created: 18/09/2025 13:36:07      + + + + + +   * ****/
/*                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RMDLCAMERA_HPP
# define RMDLCAMERA_HPP

# include <simd/simd.h>
# include <functional>

# import "RMDLMainRenderer_shared.h"

enum class RMDLCameraEase {
    Linear,
    SmoothStep,
    SmootherStep,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInOutBack,
};

struct RMDLCameraSnapshot
{
    simd::float3    position;
    simd::float3    direction;
    simd::float3    up;
    float           viewAngle;
    float           nearPlane;
    float           farPlane;
    simd::float3    orbitTarget;
    float           orbitDistance;
    float           yaw;
    float           pitch;
};

class RMDLCamera
{
public:
    RMDLCamera();
    ~RMDLCamera();

    RMDLCamera&     initPerspectiveWithPosition( simd::float3 position, simd::float3 direction, simd::float3 up, float viewAngle, float aspectRatio, float nearPlane, float farPlane );
    RMDLCamera&     initParallelWithPosition( simd::float3 position, simd::float3 direction, simd::float3 up, float width, float height, float nearPlane, float farPlane );
    RMDLCameraUniforms  uniforms();
    void            updateUniforms();
    bool            isPerspective() const;
    bool            isParallel() const;
    simd::float3    left() const;
    simd::float3    right() const;
    simd::float3    down() const;
    simd::float3    forward() const;
    simd::float3    backward() const;
    float           nearPlane() const;
    float           farPlane() const;
    float           aspectRatio() const;
    float           viewAngle() const;
    float           width() const;
    simd::float3    up() const;
    simd::float3    position() const;
    simd::float3    direction() const;
    void            setNearPlane(float newNearPlane);
    void            setFarPlane(float newFarPlane);
    void            setAspectRatio(float newAspectRatio);
    void            setViewAngle(float newAngle);
    void            setWidth(float newWidth);
    void            setPosition(simd::float3 newPosition);
    void            setUp(simd::float3 newUp);
    void            setDirection(simd::float3 newDirection);
    simd::float4x4  ViewMatrix();
    simd::float4x4  getProjectionMatrix();
    simd::float4x4  ViewProjectionMatrix();
    simd::float4x4  InvOrientationProjectionMatrix();
    simd::float4x4  InvViewProjectionMatrix();
    simd::float4x4  InvProjectionMatrix();
    simd::float4x4  InvViewMatrix();
    void            rotateOnAxis( simd::float3 inAxis, float inRadians );
    
    void rotateYawPitch(float deltaYaw, float deltaPitch);
    void setYaw(float yaw);
    void setPitch(float pitch);
    void setPitchLimits(float minPitch, float maxPitch);
    float yaw() const { return _yaw; }
    float pitch() const { return _pitch; }
    
    void setOrbitMode(bool enabled);
    void setOrbitTarget(simd::float3 target);
    void setOrbitDistance(float distance);
    void orbit(float deltaYaw, float deltaPitch);
    void zoom(float delta);
    bool isOrbitMode() const { return _orbitMode; }
    simd::float3 orbitTarget() const { return _orbitTarget; }
    float orbitDistance() const { return _orbitDistance; }
    
    void followTarget(simd::float3 target, float distance, float height, float smoothing, float dt);
    void lookAt(simd::float3 target);
    
    void updateOrbitPosition();
    void updateDirectionFromYawPitch();
    void rotateOnAxisOrbit(float yawDelta, float pitchDelta);
    
    RMDLCameraSnapshot snapshot() const;
    void applySnapshot(const RMDLCameraSnapshot& snap);
    
    void transitionTo(const RMDLCameraSnapshot& target, float duration,
                      RMDLCameraEase ease = RMDLCameraEase::SmoothStep,
                      std::function<void()> onComplete = std::function<void()>());
    
    void transitionTo(const RMDLCamera& other, float duration,
                      RMDLCameraEase ease = RMDLCameraEase::SmoothStep,
                      std::function<void()> onComplete = std::function<void()>());
    
    void updateTransition(float delta);
    
    bool isTransitioning() const { return _transitionActive; }
    void cancelTransition() { _transitionActive = false; }
    
    // Shake
    void applyShake(float intensity, float duration, float frequency = 25.0f);
private:
    float           _nearPlane;
    float           _farPlane;
    float           _aspectRatio;
    float           _viewAngle;
    float           _width;
    simd::float3    _up;
    simd::float3    _position;
    simd::float3    _direction;
    void            orthogonalizeFromNewUp( simd::float3 newUp );
    void            orthogonalizeFromNewForward( simd::float3 newForward );
    bool            _uniformsDirty;
    RMDLCameraUniforms  _uniforms;
    
    float _yaw = 0.0f;
    float _pitch = 0.0f;
    float _pitchMin = -M_PI_2 + 0.1f;   // Limite basse (-89°)
    float _pitchMax = M_PI_2 - 0.1f;    // Limite haute (+89°)
    
    simd::float3 _orbitTarget = {0, 0, 0};
    float _orbitDistance = 10.0f;
    bool _orbitMode = false;

    float applyEase(float t, RMDLCameraEase ease) const;
    simd::float3 slerpDirection(simd::float3 a, simd::float3 b, float t) const;
    
    bool _transitionActive = false;
    RMDLCameraSnapshot _transitionFrom;
    RMDLCameraSnapshot _transitionTo;
    float _transitionDuration = 0.0f;
    float _transitionElapsed = 0.0f;
    RMDLCameraEase _transitionEase = RMDLCameraEase::SmoothStep;
    std::function<void()> _transitionOnComplete = nullptr;
    
    float _shakeIntensity = 0.0f;
    float _shakeDuration = 0.0f;
    float _shakeElapsed = 0.0f;
    float _shakeFrequency = 25.0f;
    simd::float3 _shakeOffset = {0, 0, 0};
};

#endif /* RMDLCAMERA_HPP */
