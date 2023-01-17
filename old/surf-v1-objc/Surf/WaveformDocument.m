//
//  WaveformDocument.m
//  Surf
//
//  Created by Jevin Sweval on 1/5/23.
//

#import "WaveformDocument.h"

@interface WaveformDocument ()
@property (atomic, strong) NSURL *url;
@property (atomic, strong) NSData *data;
@end

@implementation WaveformDocument

- (instancetype)init {
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
    }
    return self;
}

+ (BOOL)autosavesInPlace {
    return YES;
}


- (void)makeWindowControllers {
    // Override to return the Storyboard file name of the document.
    [self addWindowController:[[NSStoryboard storyboardWithName:@"Main" bundle:nil] instantiateControllerWithIdentifier:@"Document Window Controller"]];
}


- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    // Read-only, not implemented
    [NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
    return nil;
}


- (BOOL)isEntireFileLoaded {
    return NO;
}


- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    NSLog(@"data: %@ type: %@\n", data, typeName);
    return YES;
}


- (BOOL)readFromURL:(NSURL *)url
              error:(NSError * _Nullable *)outError {
    self.url = url;
    self.data = [NSData dataWithContentsOfURL:self.url options:NSDataReadingMappedIfSafe error:outError];
    if (outError || !self.data) {
        return NO;
    }
    return YES;
}

@end
