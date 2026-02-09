//
//  Renderer.m
//  Spammy
//
//  Created by Rémy on 12/12/2025.
//

#include "RMDLRendererSpammy.hpp"

#import "GameViewController.h"

@interface GameWindow : NSWindow

@property (strong) RMDLGameApplicationLoupy* gameCoordinator;
- (InputState)pollInputState;

@end

// List the keys in use within this sample
// The enum value is the NSEvent key code
using Controls = uint8_t;
enum : Controls
{
    // Keycodes that control translation
    controlsForward     = 0x0d, // W key
    controlsBackward    = 0x01, // S key
    controlsStrafeUp    = 0x31, // Spacebar
    controlsStrafeDown  = 0x08, // C key
    controlsStrafeLeft  = 0x00, // A key
    controlsStrafeRight = 0x02, // D key

    // Keycodes that control rotation
    controlsRollLeft    = 0x0c, // Q key
    controlsRollRight   = 0x0e, // E key
    controlsTurnLeft    = 0x7b, // Left arrow
    controlsTurnRight   = 0x7c, // Right arrow
    controlsTurnUp      = 0x7e, // Up arrow
    controlsTurnDown    = 0x7d, // Down arrow

    // The brush size
    controlsIncBrush    = 0x1E, // Right bracket
    controlsDecBrush    = 0x21, // Left bracket

    // Additional virtual keys
    controlsFast        = 0x80,
    controlsSlow        = 0x81,
    
    inventory           = 0x0f  // R Key
};

@implementation GameWindow
{
    NSMutableSet<NSNumber*>*    _pressedKeys;
    simd::float2                _mouseDrag;
    BOOL                        _mouseCaptured;
}

- (instancetype)initWithContentRect:(NSRect)rect styleMask:(NSWindowStyleMask)mask backing:(NSBackingStoreType)backing defer:(BOOL)flag screen:(NSScreen*)screen
{
    self = [super initWithContentRect:rect styleMask:mask backing:backing defer:flag screen:screen];
    if (self)
    {
        _pressedKeys = [NSMutableSet new];
        _mouseDrag = {0, 0};
        _mouseCaptured = NO;
    }
    return self;
}

- (BOOL)acceptsFirstResponder { return YES; }

- (BOOL)acceptsFirstMouse:(NSEvent *)event { return YES; }

- (BOOL)canBecomeKeyWindow { return YES; }

- (BOOL)canBecomeMainWindow { return YES; }

- (void)keyDown:(NSEvent *)event
{
    if (!event.ARepeat)
    {
        [_pressedKeys addObject:@(event.keyCode)];
//        [self handleActionKey:event.keyCode pressed:YES];
        switch (event.keyCode)
        {
            case 0x31: [self.gameCoordinator jump]; break; // Espace
            case inventory: [self.gameCoordinator inventory : true]; break; // R
            case 0x03: [self.gameCoordinator setInventoryBest]; break; // F
            case 0x0B: // B
                [self.gameCoordinator toggleVehicleBuildMode];
                break;
            case 0x22: [self.gameCoordinator setInventory]; break; // i
            case 0x30: [self.gameCoordinator toggleVehicleMode]; break; // tab
            
            case 0x12: [self.gameCoordinator selectVehicleSlot:0]; break;
            case 0x13: [self.gameCoordinator selectVehicleSlot:1]; break;
            case 0x14: [self.gameCoordinator selectVehicleSlot:2]; break;
            case 0x15: [self.gameCoordinator selectVehicleSlot:3]; break;
            case 0x17: [self.gameCoordinator selectVehicleSlot:4]; break;
            case 0x16: [self.gameCoordinator selectVehicleSlot:5]; break;
            case 0x1A: [self.gameCoordinator selectVehicleSlot:6]; break;
            case 0x1C: [self.gameCoordinator selectVehicleSlot:7]; break;
            case 0x19: [self.gameCoordinator selectVehicleSlot:8]; break;
            case 0x1D: [self.gameCoordinator selectVehicleSlot:9]; break;
                
                
            case 0x0D: [self.gameCoordinator fabPanelKey:0x0D]; break; // W
            case 0x00: [self.gameCoordinator fabPanelKey:0x00]; break; // A
            case 0x01: [self.gameCoordinator fabPanelKey:0x01]; break; // S
            case 0x02: [self.gameCoordinator fabPanelKey:0x02]; break; // D
            case 0x0C: [self.gameCoordinator fabPanelKey:0x0C]; break; // Q
            case 0x06: [self.gameCoordinator fabPanelKey:0x06]; break; // Z
            case 0x0E: [self.gameCoordinator fabPanelKey:0x0E]; break; // E
        }
    }
    NSString* chars = [event charactersIgnoringModifiers];
    if (chars.length == 0) return;
    if (chars.length > 0 && [chars characterAtIndex:0] == 'y')
        [self.gameCoordinator playSoundTestY];
}

- (void)keyUp:(NSEvent*)event
{
    [_pressedKeys removeObject:@(event.keyCode)];
//    [self handleActionKey:event.keyCode pressed:NO];
    switch (event.keyCode)
    {
        case 0x0F: [self.gameCoordinator inventory : NO]; break;
    }
}

- (void)flagsChanged:(NSEvent*)event
{
    if (event.modifierFlags & NSEventModifierFlagShift)
        [_pressedKeys addObject:@(0x80)];
    else
        [_pressedKeys removeObject:@(0x80)];

    if (event.modifierFlags & NSEventModifierFlagControl)
        [_pressedKeys addObject:@(0x81)];
    else
        [_pressedKeys removeObject:@(0x81)];
}

- (void)handleActionKey:(uint16_t)keyCode pressed:(BOOL)pressed
{
    if (!pressed) return;
}

- (void)mouseUp:(NSEvent *)event
{
    [[NSCursor crosshairCursor] set];
    [self.gameCoordinator vehicleMouseUp];
}

- (void)mouseDown:(NSEvent *)event
{
    [[NSCursor closedHandCursor] set];
    [self.gameCoordinator vehicleMouseDown:NO];
//    if (!_mouseCaptured)
//        [self captureMouse];
////    else
////        [self.gameCoordinator triggerPrimaryAction];
}

- (void)rightMouseDown:(NSEvent *)event
{
    [self.gameCoordinator vehicleMouseDown:YES];
//    [self.gameCoordinator triggerSecondaryAction];
}

- (void)mouseMoved:(NSEvent *)event
{
    if (_mouseCaptured)
    {
        _mouseDrag.x += event.deltaX;
        _mouseDrag.y += event.deltaY;
    }
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
    float sensitivity = 0.009f;

    float deltaX = [event deltaX] * sensitivity;
    float deltaY = [event deltaY] * sensitivity;

    [self.gameCoordinator rotateCameraYaw : deltaX Pitch : deltaY];
    
    [self.gameCoordinator fabPanelDrag:deltaX deltaY:deltaY];
}

- (void)scrollWheel:(NSEvent *)event
{
    float delta = [event scrollingDeltaY];
    if ([event hasPreciseScrollingDeltas]) delta *= 0.1f;
    [self.gameCoordinator handleScroll:delta / 4];
    
    [self.gameCoordinator fabPanelScroll:delta / 4];
}

//- (void)mouseEntered:(NSEvent *)event
//{
//}

//- (void)captureMouse
//{
//    _mouseCaptured = YES;
//    CGAssociateMouseAndMouseCursorPosition(YES);
////    [NSCursor hide];
//}
//
//- (void)releaseMouse
//{
//    _mouseCaptured = NO;
//    CGAssociateMouseAndMouseCursorPosition(NO);
////    [NSCursor unhide];
//}
//
//- (void)cancelOperation:(id)sender
//{
//    [self releaseMouse];
//}

- (InputState)pollInputState
{
    InputState state;
    
    if ([_pressedKeys containsObject:@(0x06)]) state.moveDirection.z -= 1;
    if ([_pressedKeys containsObject:@(0x01)]) state.moveDirection.z += 1;
    if ([_pressedKeys containsObject:@(0x0C)]) state.moveDirection.x -= 1;
    if ([_pressedKeys containsObject:@(0x02)]) state.moveDirection.x += 1;
    if ([_pressedKeys containsObject:@(0x00)]) state.moveDirection.y += 1;
    if ([_pressedKeys containsObject:@(0x0E)]) state.moveDirection.y -= 1;
    if ([_pressedKeys containsObject:@(0x00)]) { /* A */ }
    if ([_pressedKeys containsObject:@(0x06)]) { /* Z */ }
    if ([_pressedKeys containsObject:@(0x0E)]) { /* E */ }
    if ([_pressedKeys containsObject:@(0x0F)]) { /* R */ }
    if ([_pressedKeys containsObject:@(0x11)]) { /* T */ }
    if ([_pressedKeys containsObject:@(0x10)]) { /* Y */ }
    if ([_pressedKeys containsObject:@(0x20)]) { /* U */ }
    if ([_pressedKeys containsObject:@(0x22)]) { /* I */ }
    if ([_pressedKeys containsObject:@(0x1F)]) { /* O */ }
    if ([_pressedKeys containsObject:@(0x23)]) { /* P */ }
    if ([_pressedKeys containsObject:@(0x0C)]) { /* Q */ }
    if ([_pressedKeys containsObject:@(0x01)]) { /* S */ }
    if ([_pressedKeys containsObject:@(0x02)]) { /* D */ }
    if ([_pressedKeys containsObject:@(0x03)]) { /* F */ }
    if ([_pressedKeys containsObject:@(0x05)]) { /* G */ }
    if ([_pressedKeys containsObject:@(0x04)]) { /* H */ }
    if ([_pressedKeys containsObject:@(0x26)]) { /* J */ }
    if ([_pressedKeys containsObject:@(0x28)]) { /* K */ }
    if ([_pressedKeys containsObject:@(0x25)]) { /* L */ }
    if ([_pressedKeys containsObject:@(0x29)]) { /* M */ }
    if ([_pressedKeys containsObject:@(0x0D)]) { /* W */ }
    if ([_pressedKeys containsObject:@(0x07)]) { /* X */ }
    if ([_pressedKeys containsObject:@(0x08)]) { /* C */ }
    if ([_pressedKeys containsObject:@(0x09)]) { /* V */ }
    if ([_pressedKeys containsObject:@(0x0B)]) { /* B */ }
    if ([_pressedKeys containsObject:@(0x2D)]) { /* N */ }
    if ([_pressedKeys containsObject:@(0x38)]) { /* Shift gch nok ->0x80*/ }
    if ([_pressedKeys containsObject:@(0x3C)]) { /* Shift drt */ }
    if ([_pressedKeys containsObject:@(0x3B)]) { /* Ctrl gauche nok ->0x81*/ }
    if ([_pressedKeys containsObject:@(0x3E)]) { /* Ctrl droite */ }
    if ([_pressedKeys containsObject:@(0x3A)]) { /* Option gch */ }
    if ([_pressedKeys containsObject:@(0x3D)]) { /* Option drt */ }
    if ([_pressedKeys containsObject:@(0x37)]) { /* Command gch */ }
    if ([_pressedKeys containsObject:@(0x36)]) { /* Command drt */ }
    if ([_pressedKeys containsObject:@(0x31)]) { /*   */ }
    if ([_pressedKeys containsObject:@(0x7E)]) { /* Haut */ }
    if ([_pressedKeys containsObject:@(0x7D)]) { /* Bas */ }
    if ([_pressedKeys containsObject:@(0x7B)]) { /* Gch */ }
    if ([_pressedKeys containsObject:@(0x7C)]) { /* Drt */ }
    if ([_pressedKeys containsObject:@(0x30)]) { /* Tab */ }
    if ([_pressedKeys containsObject:@(0x39)]) { /* Lock */ }
    // 0x24 Return <- !!
    // 0x4C Enter (pavé numérique)
    // 0x35 Escape
    // 0x33 Delete (←)
    // 0x75 Forward Delete (→)

    float len = simd::length(state.moveDirection);
    if (len > 1.0f)
        state.moveDirection /= len;
    
    if ([_pressedKeys containsObject:@(0x80)])
        state.speedMultiplier = 6.0f; // Shift aze
    if ([_pressedKeys containsObject:@(0x81)])
        state.speedMultiplier = 0.2f; // Ctrl aze
    
    state.lookDelta = _mouseDrag;
    _mouseDrag = {0, 0};
    
    return state;
}

@end

@implementation RMDLGameApplicationLoupy
{
    GameWindow*                         _window;
    std::unique_ptr<GameCoordinator>    _pGameCoordinator;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return (YES);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [self createWindow];
    [self createView];
    [self evaluateCommandLine];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    _pGameCoordinator.reset();
}

- (void)evaluateCommandLine
{
    NSArray<NSString *>* args = [[NSProcessInfo processInfo] arguments];
    BOOL exitAfterOneFrame = [args containsObject:@"--auto-close"];
    if (exitAfterOneFrame)
    {
        NSLog(@"Automatically terminating in 8 seconds...");
        [[NSApplication sharedApplication] performSelector:@selector(terminate:) withObject:self afterDelay:8];
    }
}

- (void)createWindow
{
    NSWindowStyleMask mask = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    NSRect contentRect = NSMakeRect(0, 0, 1280, 720);
    NSScreen* screen = [NSScreen mainScreen];
    contentRect.origin.x = (screen.frame.size.width / 2) - (contentRect.size.width / 2);
    contentRect.origin.y = (screen.frame.size.height / 2) - (contentRect.size.height / 2);
    _window = [[GameWindow alloc] initWithContentRect:contentRect styleMask:mask backing:NSBackingStoreBuffered defer:NO screen:screen];
    _window.releasedWhenClosed = NO;
    _window.minSize = NSMakeSize(640, 360);
    _window.gameCoordinator = self;
    NSString* title = [NSString stringWithFormat:@"Black Hole Metal4 x C++ (%@ @ %ldHz, EDR max: %.2f)", screen.localizedName, (long)screen.maximumFramesPerSecond, screen.maximumExtendedDynamicRangeColorComponentValue];
    _window.title = title;
    [_window makeKeyAndOrderFront:nil];
    [_window setIsVisible:YES];
    [_window makeMainWindow];
}

- (void)createView
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    _mtkView = [[MTKView alloc] initWithFrame:_window.contentLayoutRect device:device];
    _mtkView.preferredFramesPerSecond = 120;
    _mtkView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    _mtkView.colorPixelFormat = MTLPixelFormatRGBA16Float;
    _mtkView.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    _mtkView.framebufferOnly = YES;
    _mtkView.paused = NO;
    _mtkView.delegate = self;
    _mtkView.enableSetNeedsDisplay = NO;
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
                                    initWithRect:_mtkView.bounds
                                    options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect | NSTrackingActiveAlways)
                                    owner:self userInfo:nil];
    [_mtkView addTrackingArea:trackingArea];
    NSScreen *screen = _window.screen ?: [NSScreen mainScreen];
    CGFloat scale = screen.backingScaleFactor ?: 1.0;
    CGSize sizePts = _mtkView.bounds.size;
    _mtkView.drawableSize = CGSizeMake(sizePts.width * scale, sizePts.height * scale);
    _window.contentView = _mtkView;
    NSString* resourcesPath = NSBundle.mainBundle.resourcePath;
    _pGameCoordinator = std::make_unique< GameCoordinator >((__bridge MTL::Device *)_mtkView.device,
                                                            (MTL::PixelFormat)_mtkView.colorPixelFormat,
                                                            (MTL::PixelFormat)_mtkView.depthStencilPixelFormat,
                                                            (NS::UInteger)_mtkView.drawableSize.width,
                                                            (NS::UInteger)_mtkView.drawableSize.height,
                                                            resourcesPath.UTF8String);
}

- (void)moveCameraX:(float)x Y:(float)y Z:(float)z
{
    _pGameCoordinator->moveCamera(simd::float3 {x, y, z});
}

- (void)rotateCameraYaw:(float)yaw Pitch:(float)pitch
{
    _pGameCoordinator->rotateCamera(yaw, pitch);
}

- (void)playSoundTestY
{
    _pGameCoordinator->playSoundTestY();
}

- (void)inventory:(BOOL)visible
{
    _pGameCoordinator->inventory(visible);
}

- (void)setInventory
{
    _pGameCoordinator->setInventory();
}

- (void)setInventoryBest
{
    _pGameCoordinator->setInventoryBest();
}

- (void)jump
{
    _pGameCoordinator->jump();
}

- (void)toggleVehicleMode
{
    if (_pGameCoordinator->m_gamePlayMode == GamePlayMode::FAB || _pGameCoordinator->m_gamePlayMode == GamePlayMode::Building)
        _pGameCoordinator->setGamePlayMode(GamePlayMode::DEV);
    else
        _pGameCoordinator->setGamePlayMode(GamePlayMode::FAB);
}

- (void)toggleVehicleBuildMode
{
    _pGameCoordinator->toggleVehicleBuildMode();
}

- (void)selectVehicleSlot:(int)slot
{
    _pGameCoordinator->selectVehicleSlot(slot);
}

- (void)rotateVehicleGhost
{
    _pGameCoordinator->rotateVehicleGhost();
}

- (void)vehicleMouseDown:(BOOL)rightClick
{
    _pGameCoordinator->vehicleMouseDown(rightClick);
}

- (void)vehicleMouseUp
{
    _pGameCoordinator->vehicleMouseUp();
}

- (void)fabPanelKey:(uint16_t)keyCode
{
    _pGameCoordinator->fabPanelKey(keyCode);
}

- (void)fabPanelDrag:(float)dx deltaY:(float)dy
{
    _pGameCoordinator->fabPanelDrag(dx, dy);
}

- (void)fabPanelScroll:(float)delta
{
    _pGameCoordinator->fabPanelScroll(delta);
}

- (void)handleScroll:(float)delta
{
    _pGameCoordinator->handleScroll(delta);
}

- (void)mouseMoved:(NSEvent *)event
{
    NSPoint cursorPos = [_mtkView convertPoint:[event locationInWindow] fromView:nil];
    CGSize drawableSize = _mtkView.drawableSize;
    CGSize boundsSize = _mtkView.bounds.size;
    
    float scaleX = drawableSize.width / boundsSize.width;
    float scaleY = drawableSize.height / boundsSize.height;
    
    float mouseX = (float)cursorPos.x * scaleX;
    float mouseY = (float)(boundsSize.height - cursorPos.y) * scaleY;
    
//    mouseX += event.deltaY;
    
//    float delta = [event scrollingDeltaY];
//    if ([event hasPreciseScrollingDeltas]) delta *= 0.1f;
//    [self rightMouseDown:event];
//    _pGameCoordinator->onMouseDragged(simd::float2{static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y)}, drawableSize.width, delta, YES);
    
    _pGameCoordinator->setMousePosition(mouseX, mouseY);
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    static CFTimeInterval lastTime = CACurrentMediaTime();
    CFTimeInterval now = CACurrentMediaTime();
    float dt = (float)(now - lastTime);
    lastTime = now;
    dt = fminf(dt, 0.1f);
    InputState input = [_window pollInputState];
    constexpr float moveSpeed = 5.0f;
    constexpr float lookSensitivity = 0.003f;
    
    simd::float3 move = input.moveDirection * moveSpeed * input.speedMultiplier * dt;
    if (simd::length(move) > 0.0001f)
        _pGameCoordinator->moveCamera(move);
    
    if (simd::length(input.lookDelta) > 0.0001f)
        _pGameCoordinator->rotateCamera(-input.lookDelta.x * lookSensitivity, -input.lookDelta.y * lookSensitivity);
    
    _pGameCoordinator->draw((__bridge MTK::View *)view);
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
    _pGameCoordinator->resizeMtkViewAndUpdateViewportWindow((NS::UInteger)size.width, (NS::UInteger)size.height);
}

@end
