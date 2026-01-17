//
//  Renderer.h
//  Spammy
//
//  Created by Rémy on 12/12/2025.
//

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface RMDLGameApplicationLoupy : NSObject <NSApplicationDelegate, NSWindowDelegate, MTKViewDelegate>

@property (nonatomic, strong, readonly) MTKView *mtkView;

- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender;

- (void)keyDown:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)mouseUp:(NSEvent *)event;
- (void)mouseDragged:(NSEvent *)event;

- (void)drawInMTKView:(MTKView *)view;
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size;

- (void)rotateCameraYaw:(float)yaw Pitch:(float)pitch;

- (void)playSoundTestY;
- (void)jump;
- (void)inventory:(BOOL)visible;

@end
