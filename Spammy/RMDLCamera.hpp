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

# import "RMDLMainRenderer_shared.h"

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
    simd::float4x4  ProjectionMatrix();
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
    
    float _yaw = 0.0f;                  // Rotation horizontale (radians)
    float _pitch = 0.0f;                // Rotation verticale (radians)
    float _pitchMin = -M_PI_2 + 0.1f;   // Limite basse (-89°)
    float _pitchMax = M_PI_2 - 0.1f;    // Limite haute (+89°)
    
    simd::float3 _orbitTarget = {0, 0, 0};
    float _orbitDistance = 10.0f;
    bool _orbitMode = false;
};

#endif /* RMDLCAMERA_HPP */
