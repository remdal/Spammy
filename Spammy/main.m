//
//  main.m
//  Spammy
//
//  Created by Rémy on 12/12/2025.
//

#import <Cocoa/Cocoa.h>

#import "GameViewController.h"

int main( int argc, const char * argv[] )
{
    NSApplication* application = [NSApplication sharedApplication];
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    RMDLGameApplicationLoupy* gameApplication = [[RMDLGameApplicationLoupy alloc] init];
    application.delegate = gameApplication;
    return (NSApplicationMain(argc, argv));
}

//#import <Cocoa/Cocoa.h>
//
//#import "GameViewController.h"
//
//int main( int argc, const char* argv[] )
//{
//    NSApplication* application = [NSApplication sharedApplication];
//    @autoreleasepool {
//    }
//    TerraGameAppDelegate* gameApplication = [[TerraGameAppDelegate alloc] init];
//    application.delegate = gameApplication;
//    return (NSApplicationMain(argc, argv));
//    return (0);
//}
