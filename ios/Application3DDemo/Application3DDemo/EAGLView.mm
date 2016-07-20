//
//  EAGLView.m
//  AppleCoder-OpenGLES-00
//
//  Created by Simon Maurice on 18/03/09.
//  Copyright Simon Maurice 2009. All rights reserved.
//


#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"
#include "CZDefine.h"
#include "Application3D.h"

#define USE_DEPTH_BUFFER 1
#define USE_PIC_BACKGROUND 0
//#define SHOW_RENDER_TIME
//declare private methods, so they can be used everywhere in this file
@interface EAGLView (PrivateMethods)
- (void)createFramebuffer;
- (void)deleteFramebuffer;
@end

@implementation EAGLView
{
    CZ3D::Application3D app3d;
    NSString *modelPath;
    BOOL modelLoaded;
}

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (!self) {
        return nil;
    }
    
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    //we don't want a transparent surface
    eaglLayer.opaque = TRUE;
    
    //here we configure the properties of our canvas, most important is the color depth RGBA8 !
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                    nil];
    
    //create an OpenGL ES 2 context
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    //if this failed or we cannot set the context for some reason, quit
    if (!context || ![EAGLContext setCurrentContext:context]) {
        NSLog(@"Could not create context!");
       // [self release];
        return nil;
    }
    
    NSString *configPath = [[[NSBundle mainBundle]bundlePath]stringByAppendingString:@"/Application3DResources.bundle/scene_violin.cfg"];

    NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    
    app3d.init([[[[NSBundle mainBundle]bundlePath] stringByAppendingString:@"/Application3DResources.bundle/glsl/"] UTF8String],[configPath UTF8String]);
    app3d.setDocDirectory([docPath UTF8String]);
    app3d.setBackgroundColor(1, 1, 1, 1);
    
#if USE_PIC_BACKGROUND
    // background image
    NSString *backImgPath = [[[NSBundle mainBundle]bundlePath] stringByAppendingPathComponent:@"background.jpg"];
    std::string cstrImagePath = std::string([backImgPath UTF8String]);
    CZImage *backgroundImage = CZLoadTexture(cstrImagePath);
    app3d.setBackgroundImage(backgroundImage);
#endif
    
    modelLoaded = NO;
    
    return self;
}// We have to implement this method


+ (Class)layerClass
{
    return [CAEAGLLayer class];
}
//our EAGLView is the view in our MainWindow which will be automatically loaded to be displayed.
//when the EAGLView gets loaded, it will be initialized by calling this method.


//on iOS, all rendering goes into a renderbuffer,
//which is then copied to the window by "presenting" it.
//here we create it!
- (void)createFramebuffer
{
    // Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
    glGenFramebuffers(1, &viewFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);
    
    // Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
    glGenRenderbuffers(1, &viewRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
    // Create the color renderbuffer and call the rendering context to allocate the storage on our Core Animation layer.
    // The width, height, and format of the renderbuffer storage are derived from the bounds and properties of the CAEAGLLayer object
    // at the moment the renderbufferStorage:fromDrawable: method is called.
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, viewRenderbuffer);
    
    // Retrieve the height and width of the color renderbuffer.
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
    
    app3d.setRenderBufferSize(backingWidth, backingHeight);
    
    if (USE_DEPTH_BUFFER) {
        // Perform similar steps to create and attach a depth renderbuffer.
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    }
    
    glGenFramebuffers(1, &msaaFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);
    glGenRenderbuffers(1, &msaaRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderbuffer);
    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_RGBA8_OES, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaRenderbuffer);
    
    if (USE_DEPTH_BUFFER) {
        // Perform similar steps to create and attach a depth renderbuffer.
        glGenRenderbuffers(1, &msaaDepthbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthbuffer);
        glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthbuffer);
    }
    
    // Test the framebuffer for completeness.
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return ;
    }
}

//deleting the framebuffer and all the buffers it contains
- (void)deleteFramebuffer
{
    //we need a valid and current context to access any OpenGL methods
    glDeleteFramebuffers(1, &viewFramebuffer);
    glDeleteFramebuffers(1, &msaaFramebuffer);
    viewFramebuffer = 0;
    msaaFramebuffer = 0;
    glDeleteRenderbuffers(1, &viewRenderbuffer);
    glDeleteRenderbuffers(1, &msaaRenderbuffer);
    viewRenderbuffer = 0;
    msaaRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        glDeleteRenderbuffers(1, &msaaDepthbuffer);
        depthRenderbuffer = 0;
        msaaDepthbuffer = 0;
    }
}


//this is where all the magic happens!
- (void)drawFrame
{
    //we need a context for rendering
    if (context != nil)
    {
        //make it the current context for rendering
        [EAGLContext setCurrentContext:context];
        
        //if our framebuffers have not been created yet, do that now!
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderbuffer);
        app3d.frame();
        //we need a lesson to be able to render something
        
        //perform the actual drawing!
        
        //finally, get the color buffer we rendered to, and pass it to iOS
        //so it can display our awesome results!
        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, msaaFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, viewFramebuffer);
        glResolveMultisampleFramebufferAPPLE();
        
        glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
        
        // Assuming you allocated a color renderbuffer to point at a Core Animation layer, you present its contents by making it the current renderbuffer
        // and calling the presentRenderbuffer: method on your rendering context.
        [context presentRenderbuffer:GL_RENDERBUFFER];
        CZCheckGLError();
    }
    else
        NSLog(@"Context not set!");
}

//our render loop just tells the iOS device that we want to keep refreshing our view all the time
- (void)startRenderLoop
{
    /* [EAGLContext setCurrentContext:context];
     [self deleteFramebuffer];
     [self createFramebuffer];*/
    
#ifdef SHOW_RENDER_TIME
    animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawFrame) userInfo:nil repeats:YES];
#endif
}


//we have to be able to stop the render loop
- (void)stopRenderLoop
{
#ifdef SHOW_RENDER_TIME
    [animationTimer invalidate];
#endif
}

- (void)rotateWithX:(float)x Y:(float)y
{
    app3d.rotate(x, y);
    [self drawFrame];
}

- (void) moveWithX:(float)x Y:(float)y
{
    app3d.translate(x, y);
    [self drawFrame];
}

- (void) scale:(float)s
{
    app3d.scale(s);
    [self drawFrame];
}

- (void) reset
{
    app3d.reset();
    [self drawFrame];
}

// for debug
- (BOOL) loadModel:(NSString*)path
{
    BOOL result = NO;
    modelPath = path;
    
    if (context != nil)
    {
        [EAGLContext setCurrentContext:context];
    
        result = app3d.loadObjModel([path UTF8String]);
    }
    
    //[self drawFrame];
    
    modelLoaded = result;
    return result;
}

- (BOOL) clearObjModel
{
    return app3d.clearObjModel();
}

- (BOOL) createShape
{
    BOOL result = NO;
    if (context != nil)
    {
        [EAGLContext setCurrentContext:context];
        
        result = app3d.createShape("cube");
    }
    return result;
}

- (void) setCameraPositionWithX:(float)x Y:(float)y Z:(float) z
{
    app3d.setCameraPosition(x, y, z);
    [self drawFrame];
}
- (void) setLigthDirectionWithX:(float)x Y:(float)y Z:(float) z
{
    app3d.setLigthDirection(x, y, z);
    [self drawFrame];
}
- (void) setAmbientColorWithX:(unsigned char)x Y:(unsigned char)y Z:(unsigned char) z
{
    app3d.setAmbientColor(x, y, z);
    [self drawFrame];
}
- (void) setDiffuseColorWithX:(unsigned char)x Y:(unsigned char)y Z:(unsigned char) z
{
    app3d.setDiffuseColor(x, y, z);
    [self drawFrame];
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:context];
    [self deleteFramebuffer];
    [self createFramebuffer];
    if(modelLoaded) [self drawFrame];
}

//cleanup our view
- (void)dealloc
{
    [self deleteFramebuffer];
    //[context release];
    //[super dealloc];
}
/*
 // Only override drawRect: if you perform custom drawing.
 // An empty implementation adversely affects performance during animation.
 - (void)drawRect:(CGRect)rect
 {
 // Drawing code
 }
 */

@end

