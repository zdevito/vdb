//
//  vdbAppDelegate.h
//  vdb
//
//  Created by Zachary DeVito on 2/7/12.
//  Copyright 2012 Stanford. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface vdbAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
