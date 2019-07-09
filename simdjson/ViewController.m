//
//  ViewController.m
//  mofongo
//
//  Created by Benoit Maison on 05/11/2018.
//  Copyright © 2018 Vision Smarts. All rights reserved.
//

#import "ViewController.h"

#import "MessageParser.h"

char * measure(size_t length, char* result, const char * filename);
    

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    [self performSelector:@selector(benchmark) withObject:nil afterDelay:1.0];
}

- (void)benchmark {
    NSBundle *b = [NSBundle mainBundle];
    NSString *dir = [b resourcePath];
    NSArray *parts1 = [NSArray arrayWithObjects:
                      dir,  @"update-center.json", (void *)nil];
    NSString *path1 = [NSString pathWithComponents:parts1];
    const char *cpath1 = [path1 fileSystemRepresentation];
    NSArray *parts2 = [NSArray arrayWithObjects:
                       dir,  @"gsoc-2018.json", (void *)nil];
    NSString *path2 = [NSString pathWithComponents:parts2];
    const char *cpath2 = [path2 fileSystemRepresentation];

    NSArray *parts3 = [NSArray arrayWithObjects:
                       dir,  @"twitter.json", (void *)nil];
    NSString *path3 = [NSString pathWithComponents:parts3];
    const char *cpath3 = [path3 fileSystemRepresentation];

    NSArray *parts4 = [NSArray arrayWithObjects:
                       dir,  @"github_events.json", (void *)nil];
    NSString *path4 = [NSString pathWithComponents:parts4];
    const char *cpath4 = [path4 fileSystemRepresentation];
    
    char result[10000];
    size_t length = 1 << 25;
    char * p;
    // get the processor pumping!
    measure(length, result, cpath1);
    measure(length, result, cpath2);
    measure(length, result, cpath3);

    
    //self.topText.text = [NSString stringWithUTF8String:result];
    p = measure(length, result, cpath2);
    self.topText.text = [NSString stringWithUTF8String:result];
    p = measure(length, p, cpath3);
    self.topText.text = [NSString stringWithUTF8String:result];
    p = measure(length, p, cpath4);
    self.topText.text = [NSString stringWithUTF8String:result];
    p = measure(length, p, cpath1);
    self.topText.text = [NSString stringWithUTF8String:result];
    //self.topText.text = [NSString stringWithUTF8String:result];
}

@end

