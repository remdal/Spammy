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

- (void)drawInMTKView:(MTKView *)view;
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size;

- (void)rotateCameraYaw:(float)yaw Pitch:(float)pitch;
- (void)moveCameraX:(float)x Y:(float)y Z:(float)z;

- (void)mouseMoved:(NSEvent *)event;

- (void)playSoundTestY;
- (void)jump;
- (void)inventory:(BOOL)visible;
- (void)setInventory;
- (void)setInventoryBest;
- (void)boh;

- (void)toggleVehicleMode;
- (void)toggleVehicleBuildMode;
- (void)selectVehicleSlot:(int)slot;
- (void)rotateVehicleGhost;
- (void)vehicleMouseDown:(BOOL)rightClick;
- (void)vehicleMouseUp;
- (void)handleScroll:(float)delta;

- (void)fabPanelKey:(uint16_t)keyCode;
- (void)fabPanelDrag:(float)dx deltaY:(float)dy;
- (void)fabPanelScroll:(float)delta;

@end
