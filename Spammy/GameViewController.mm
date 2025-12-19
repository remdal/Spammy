//
//  Renderer.m
//  Spammy
//
//  Created by Rémy on 12/12/2025.
//

#include "RMDLRendererSpammy.hpp"

#import "GameViewController.h"

@interface GameWindow : NSWindow <MTKViewDelegate>

@property (strong) RMDLGameApplicationLoupy* gameCoordinator;

@end

@implementation GameWindow

- (void)keyDown:(NSEvent *)event
{
    NSString *chars = [event charactersIgnoringModifiers];
    if (chars.length == 0) { return; }
    unichar character = [chars characterAtIndex:0];
    float moveSpeed = 0.065f;
    switch (character)
    {
        case 'w':
            [self.gameCoordinator moveCameraX : 0 Y : 0 Z : -moveSpeed];
            break;
        case 's':
            [self.gameCoordinator moveCameraX : 0 Y : 0 Z : moveSpeed];
            break;
        case 'a':
            [self.gameCoordinator moveCameraX : -moveSpeed Y : 0 Z : 0];
            break;
        case 'd':
            [self.gameCoordinator moveCameraX : moveSpeed Y : 0 Z : 0];
            break;
        case 'q':
            [self.gameCoordinator moveCameraX : 0 Y : moveSpeed Z : 0];
            break;
        case 'e':
            [self.gameCoordinator moveCameraX : 0 Y : -moveSpeed Z : 0];
            break;
        case ' ':
            [self.gameCoordinator moveCameraX : moveSpeed * moveSpeed Y : 0 Z : 0];
            break;
        case 'y':
            [self.gameCoordinator playSoundTestY];
            break;
        default:
            [super keyDown:event];
            break;
    }
}

- (void)mouseDragged:(NSEvent *)event
{
    float sensitivity = 0.009f;

    float deltaX = [event deltaX] * sensitivity;
    float deltaY = [event deltaY] * sensitivity;

    [self.gameCoordinator rotateCameraYaw : deltaX Pitch : deltaY];
}

- (void)mouseDown:(NSEvent *)event
{
    [super mouseDown : event];
}

@end

@implementation RMDLGameApplicationLoupy
{
    GameWindow*                             _window;
    std::unique_ptr< GameCoordinator > _pGameCoordinator;
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
    [_window makeKeyAndOrderFront:_window];
}

- (void)createView
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    _mtkView = [[MTKView alloc] initWithFrame:_window.contentLayoutRect device:device];
    _mtkView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    _mtkView.colorPixelFormat = MTLPixelFormatRGBA16Float;
    _mtkView.depthStencilPixelFormat = MTLPixelFormatInvalid; //MTLPixelFormatDepth32Float
    _mtkView.framebufferOnly = YES;
    _mtkView.paused = NO;
    _mtkView.delegate = self;
    _mtkView.enableSetNeedsDisplay = NO;
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
    //_pGameCoordinator->moveCamera( simd::float3 {x, y, z} );
}

- (void)rotateCameraYaw:(float)yaw Pitch:(float)pitch
{
    //_pGameCoordinator->rotateCamera(yaw, pitch);
}

- (void)playSoundTestY
{
    _pGameCoordinator->playSoundTestY();
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    _pGameCoordinator->draw((__bridge MTK::View *)view);
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
    _pGameCoordinator->resizeMtkView(size.width, size.height);
}

@end
//
//@implementation TerraGameWindow
//
//- (void)keyDown:(NSEvent *)event {
//    NSString *chars = [event charactersIgnoringModifiers];
//    if (chars.length == 0) { return; }
//    unichar character = [chars characterAtIndex:0];
//    
//    if ([self.gameCoordinator respondsToSelector:@selector(handleKeyPress:)]) {
//        [self.gameCoordinator handleKeyPress:character];
//    }
//}
//
//- (void)mouseDragged:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleMouseDrag:)]) {
//        [self.gameCoordinator handleMouseDrag:event];
//    }
//}
//
//- (void)mouseDown:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleMouseDown:)]) {
//        [self.gameCoordinator handleMouseDown:event];
//    }
//}
//
//- (void)rightMouseDown:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleRightMouseDown:)]) {
//        [self.gameCoordinator handleRightMouseDown:event];
//    }
//}
//
//- (void)mouseUp:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleMouseUp:)]) {
//        [self.gameCoordinator handleMouseUp:event];
//    }
//}
//
//- (void)rightMouseUp:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleRightMouseUp:)]) {
//        [self.gameCoordinator handleRightMouseUp:event];
//    }
//}
//
//- (void)scrollWheel:(NSEvent *)event {
//    if ([self.gameCoordinator respondsToSelector:@selector(handleScroll:)]) {
//        [self.gameCoordinator handleScroll:event];
//    }
//}
//
//@end
//
//@implementation TerraGameAppDelegate {
//    TerraGameWindow* _window;
//    std::unique_ptr<GameCoordinator> _pGameRenderer;
//}
//
//- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
//    return YES;
//}
//
//- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
//    [self createWindow];
//    [self createView];
//}
//
//- (void)applicationWillTerminate:(NSNotification *)notification {
//    _pGameRenderer.reset();
//}
//
//- (void)createWindow {
//    NSWindowStyleMask mask = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled |
//                             NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
//    NSRect contentRect = NSMakeRect(0, 0, 1280, 720);
//    NSScreen* screen = [NSScreen mainScreen];
//    contentRect.origin.x = (screen.frame.size.width / 2) - (contentRect.size.width / 2);
//    contentRect.origin.y = (screen.frame.size.height / 2) - (contentRect.size.height / 2);
//    
//    _window = [[TerraGameWindow alloc] initWithContentRect:contentRect
//                                                 styleMask:mask
//                                                   backing:NSBackingStoreBuffered
//                                                     defer:NO
//                                                    screen:screen];
//    _window.releasedWhenClosed = NO;
//    _window.minSize = NSMakeSize(640, 360);
//    _window.gameCoordinator = self;
//    _window.title = @"TerraBlock - Constructeur de Véhicules";
//    [_window makeKeyAndOrderFront:nil];
//    [_window setIsVisible:YES];
//    [_window makeMainWindow];
//}
//
//- (void)createView {
//    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
//    _mtkView = [[MTKView alloc] initWithFrame:_window.contentLayoutRect device:device];
//    _mtkView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
//    _mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
//    _mtkView.depthStencilPixelFormat = MTLPixelFormatInvalid;
//    _mtkView.framebufferOnly = YES;
//    _mtkView.paused = NO;
//    _mtkView.delegate = self;
//    _mtkView.enableSetNeedsDisplay = NO;
//    
//    NSScreen *screen = _window.screen ?: [NSScreen mainScreen];
//    CGFloat scale = screen.backingScaleFactor ?: 1.0;
//    CGSize sizePts = _mtkView.bounds.size;
//    _mtkView.drawableSize = CGSizeMake(sizePts.width * scale, sizePts.height * scale);
//    _window.contentView = _mtkView;
//
//    MTL::Device* _pDevice = (__bridge MTL::Device *)device;
//    MTL::PixelFormat pixelFormat = MTL::PixelFormatBGRA8Unorm;
//    NS::UInteger w = (NS::UInteger)_mtkView.drawableSize.width;
//    NS::UInteger h = (NS::UInteger)_mtkView.drawableSize.height;
//    
//    _pGameRenderer = std::make_unique<GameCoordinator>(_pDevice, pixelFormat, w, h);
//}
//
//- (void)handleKeyPress:(unichar)character {
//    _pGameRenderer->handleKeyPress(character);
//}
//
//- (void)handleMouseDrag:(NSEvent*)event {
//    NSPoint loc = [_mtkView convertPoint:[event locationInWindow] fromView:nil];
//    _pGameRenderer->handleMouseMove(loc.x, loc.y);
//}
//
//- (void)handleMouseDown:(NSEvent*)event {
//    NSPoint loc = [_mtkView convertPoint:[event locationInWindow] fromView:nil];
//    _pGameRenderer->handleMouseMove(loc.x, loc.y);
//    _pGameRenderer->handleMouseDown(false);
//}
//
//- (void)handleRightMouseDown:(NSEvent*)event {
//    NSPoint loc = [_mtkView convertPoint:[event locationInWindow] fromView:nil];
//    _pGameRenderer->handleMouseMove(loc.x, loc.y);
//    _pGameRenderer->handleMouseDown(true);
//}
//
//- (void)handleMouseUp:(NSEvent*)event {
//    _pGameRenderer->handleMouseUp();
//}
//
//- (void)handleRightMouseUp:(NSEvent*)event {
//    _pGameRenderer->handleMouseUp();
//}
//
//- (void)handleScroll:(NSEvent*)event {
//    _pGameRenderer->handleScroll([event deltaY]);
//}
//
//- (void)drawInMTKView:(nonnull MTKView *)view {
//    _pGameRenderer->draw((__bridge MTK::View *)view);
//}
//
//- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
//    _pGameRenderer->setViewSize(size.width, size.height);
//}
//
//@end
