#include "RMDLFontLoader.h"

#import <MetalKit/MetalKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

#include <stdio.h>

CGRect calculateReferenceBounds(char c, CTFontRef font, CGColorRef color, CGContextRef ctx)
{
    NSString* str = [NSString stringWithFormat:@"%c", c];
    NSMutableAttributedString* attributedString = [[NSMutableAttributedString alloc] initWithString:str];
    [attributedString addAttribute:NSFontAttributeName value:(__bridge id)font range:NSMakeRange(0,1)];
    [attributedString addAttribute:NSForegroundColorAttributeName value:(__bridge id)color range:NSMakeRange(0,1)];
    
    CTLineRef lineOfText = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributedString);
    
    CGContextSetTextPosition(ctx, 0, 0);
    CGRect rect = CTLineGetImageBounds(lineOfText, ctx);
    
    CFRelease(lineOfText);
    return (rect);
}

FontAtlas newFontAtlas(MTL::Device* pDevice)
{
    FontAtlas fontAtlas;

    const uint32_t bitmapW = 1024;
    const uint32_t bitmapH = 1024;
    const uint32_t bitmapChannels = 4;
    const uint32_t bitmapSize = bitmapW * bitmapH * bitmapChannels;
    const uint32_t bitsPerComponent = 8;
    const uint32_t bytesPerRow = bitmapW * bitmapChannels;

    uint8_t* bitmap = new uint8_t[bitmapSize];
    ft_memset(bitmap, 0x0, bitmapSize);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(bitmap,
                                             bitmapW,
                                             bitmapH,
                                             bitsPerComponent,
                                             bytesPerRow,
                                             colorSpace,
                                             kCGImageAlphaPremultipliedLast);
    assert(ctx);
    
    CGFloat margin       = (CGFloat)bitmapW * 0.120/2.0;
    CGColorRef color     = CGColorCreateGenericRGB(1.0, 0.0, 0.0, 1.0);
    CGFloat fontSize     = (CGFloat)bitmapW * 0.08; // Each character takes 8% of the bitmap
    CFStringRef fontName = CFSTR("PT Mono");
    
    CTFontRef font = CTFontCreateWithName(fontName, fontSize, nullptr);
    
    CGFloat x = margin;
    CGFloat y = margin;
    
    CGRect minBounds = calculateReferenceBounds('X', font, color, ctx);
    
    std::vector<CGColorRef> colors = {
        CGColorCreateGenericRGB(99/255.0, 180/255.0, 73/255.0, 1.0),
        CGColorCreateGenericRGB(255/255.0, 179/255.0, 36/255.0, 1.0),
        CGColorCreateGenericRGB(250/255.0, 131/255.0, 28/255.0, 1.0),
        CGColorCreateGenericRGB(226/255.0, 67/255.0, 59/255.0, 1.0),
        CGColorCreateGenericRGB(149/255.0, 72/255.0, 150/255.0, 1.0),
        CGColorCreateGenericRGB(7/255.0, 160/255.0, 222/255.0, 1.0)
    };
    
    for (int i = 0; i < sizeof(g_chars)/sizeof(g_chars[0]); ++i )
    {
        NSString* str = [NSString stringWithFormat:@"%c", g_chars[i]];
        NSMutableAttributedString* attributedString = [[NSMutableAttributedString alloc] initWithString:str];
        [attributedString addAttribute:NSFontAttributeName value:(__bridge id)font range:NSMakeRange(0,1)];
        
        CGColorRef charColor = colors[rand() % colors.size()];
        [attributedString addAttribute:NSForegroundColorAttributeName value:(__bridge id)charColor range:NSMakeRange(0,1)];
        
        CTLineRef lineOfText = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributedString);
        
        CGContextSetTextPosition(ctx, 0, 0);
        CGRect rect = CTLineGetImageBounds(lineOfText, ctx);
        
        // ImageBounds closely wraps the glyphs. To make the font behave
        // as a monospaced one, normalize the bounds to the reference one
        // and center the glyph:
        CGSize oldSize = rect.size;
        rect.size.width = std::max(rect.size.width, minBounds.size.width);
        rect.size.height = std::max(rect.size.height, minBounds.size.height);
        
        CGFloat dx = (rect.size.width - oldSize.width) * 0.5;
        CGFloat dy = (rect.size.height - oldSize.height) * 0.5;
        
        // Note: this adjustment centers all rendered glyphs into the sprite.
        // A realistic font would be character-dependent where, for example,
        // the UI renders the "." character is aligned to the bottom-left
        // marging.
        rect.origin.x -= dx;
        rect.origin.y -= dy;
    
#define DEBUG_CHAR_BOUNDS 0
#if DEBUG_CHAR_BOUNDS
        CGContextSetStrokeColorWithColor(ctx, color);
        
        CGRect boundsRect = CGRectMake(x+rect.origin.x,
                                       y+rect.origin.y,
                                       rect.size.width,
                                       rect.size.height);
        
        CGContextAddRect(ctx, boundsRect);
        CGContextDrawPath(ctx, kCGPathStroke);
#endif // DEBUG_CHAR_BOUNDS
        
        CGContextSetTextPosition(ctx, x, y);
        CTLineDraw(lineOfText, ctx);
        
        // Calculate and store UVs
        
        FontAtlas::CharUVs& uvs = fontAtlas.charToUVs[g_chars[i] - g_chars[0]];
        uvs.sw = simd_make_float2((x+rect.origin.x) / bitmapW,
                                  (-(y+rect.origin.y)+bitmapH) / bitmapH);
        
        uvs.se = simd_make_float2((x+rect.origin.x+rect.size.width) / bitmapW,
                                  (-(y+rect.origin.y)+bitmapH) / bitmapH);
        
        uvs.ne = simd_make_float2((x+rect.origin.x+rect.size.width) / bitmapW,
                                  (-(y+rect.origin.y+rect.size.height)+bitmapH) / bitmapH);
        
        uvs.nw = simd_make_float2((x+rect.origin.x) / bitmapW,
                                  (-(y+rect.origin.y+rect.size.height)+bitmapH) / bitmapH);

        // Calculate next character location in atlas:
        
        x += rect.size.width + margin;
        if ((x + rect.size.width + margin/2) >= (CGFloat)bitmapW)
        {
            x = margin;
            y += rect.size.height + margin;
        }
        
        CFRelease(lineOfText);
    }
    
    for (CGColorRef color : colors)
    {
        CFRelease(color);
    }
    
    // Create a Metal texture with the font atlas
    
    auto pTextureDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    pTextureDesc->setWidth(bitmapW);
    pTextureDesc->setHeight(bitmapH);
    pTextureDesc->setTextureType(MTL::TextureType2DArray);
    pTextureDesc->setUsage(MTL::TextureUsageShaderRead);
    pTextureDesc->setArrayLength(1);
    pTextureDesc->setStorageMode(MTL::StorageModeShared);
    pTextureDesc->setMipmapLevelCount(1);
    
    fontAtlas.texture = NS::TransferPtr(pDevice->newTexture(pTextureDesc.get()));
    fontAtlas.texture->setLabel(MTLSTR("Font Atlas Texture"));
    fontAtlas.texture->replaceRegion(MTL::Region(0, 0, bitmapW, bitmapH), 0, 0,
                                       bitmap, bytesPerRow, bitmapW * bitmapH * bitmapChannels);
    
    // Release Core Graphics and Core Text objects
    CFRelease(color);
    CFRelease(font);
    CFRelease(ctx);
    CFRelease(colorSpace);
    delete [] bitmap;
    
    return fontAtlas;
}

FiraCode newFiraCode( MTL::Device* pDevice )
{
    FiraCode firaCode;
    const uint32_t bitmapW = 4096;
    const uint32_t bitmapH = 4096;
    const uint32_t bitmapChannels = 4;
    const uint32_t bitmapSize = bitmapW * bitmapH * bitmapChannels;
    const uint32_t bitsPerComponent = 8;
    const uint32_t bytesPerRow = bitmapW * bitmapChannels;
    uint8_t* bitmap = new uint8_t[bitmapSize];
    ft_memcpy(bitmap, 0x0, bitmapSize);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(bitmap,
                                             bitmapW,
                                             bitmapH,
                                             bitsPerComponent,
                                             bytesPerRow,
                                             colorSpace,
                                             kCGImageAlphaPremultipliedLast);
    assert(ctx);
    CGFloat     margin      = (CGFloat)bitmapW * 0.120/2.0;
    CGColorRef  color       = CGColorCreateGenericRGB(1.0, 0.0, 0.0, 1.0);
    CGFloat     fontSize    = (CGFloat)bitmapW * 0.08;
    CFStringRef fontName    = CFSTR("Fira Code");
    CTFontRef   font        = CTFontCreateWithName(fontName, fontSize, nullptr);
    CGFloat     x           = margin;
    CGFloat     y           = margin;
    CGRect      minBounds   = calculateReferenceBounds('X', font, color, ctx);
    std::vector<CGColorRef> colors =
    {
        CGColorCreateGenericRGB(9/255.0, 180/255.0, 73/255.0, 1.0),
        CGColorCreateGenericRGB(254/255.0, 179/255.0, 36/255.0, 1.0),
        CGColorCreateGenericRGB(232/255.0, 131/255.0, 28/255.0, 1.0),
        CGColorCreateGenericRGB(226/255.0, 67/255.0, 59/255.0, 1.0),
        CGColorCreateGenericRGB(149/255.0, 72/255.0, 150/255.0, 1.0),
        CGColorCreateGenericRGB(1/255.0, 160/255.0, 2/255.0, 1.0)
    };
    for (int i = 0; i < sizeof(g_chars)/sizeof(g_chars[0]); ++i)
    {
        NSString* str = [NSString stringWithFormat:@"%c", g_chars[i]];
        NSMutableAttributedString* attributedString = [[NSMutableAttributedString alloc] initWithString:str];
        [attributedString addAttribute:NSFontAttributeName value:(__bridge id)font range:NSMakeRange(0,1)];
        CGColorRef charColor = colors[rand() % colors.size()];
        [attributedString addAttribute:NSForegroundColorAttributeName value:(__bridge id)charColor range:NSMakeRange(0,1)];
        CTLineRef lineOfText = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributedString);
        CGContextSetTextPosition(ctx, 0, 0);
        CGRect rect = CTLineGetImageBounds(lineOfText, ctx);
        CGSize oldSize = rect.size;
        rect.size.width = std::max(rect.size.width, minBounds.size.width);
        rect.size.height = std::max(rect.size.height, minBounds.size.height);
        CGFloat dx = (rect.size.width - oldSize.width) * 0.5;
        CGFloat dy = (rect.size.height - oldSize.height) * 0.5;
        rect.origin.x -= dx;
        rect.origin.y -= dy;
        CGContextSetTextPosition(ctx, x, y);
        CTLineDraw(lineOfText, ctx);
        FiraCode::CharUVs& uvs = firaCode.charToUVs[g_chars[i] - g_chars[0]];
        uvs.sw = simd_make_float2((x+rect.origin.x) / bitmapW,
                                  (-(y+rect.origin.y)+bitmapH) / bitmapH);
        
        uvs.se = simd_make_float2((x+rect.origin.x+rect.size.width) / bitmapW,
                                  (-(y+rect.origin.y)+bitmapH) / bitmapH);
        
        uvs.ne = simd_make_float2((x+rect.origin.x+rect.size.width) / bitmapW,
                                  (-(y+rect.origin.y+rect.size.height)+bitmapH) / bitmapH);
        
        uvs.nw = simd_make_float2((x+rect.origin.x) / bitmapW,
                                  (-(y+rect.origin.y+rect.size.height)+bitmapH) / bitmapH);
        x += rect.size.width + margin;
        if ((x + rect.size.width + margin/2) >= (CGFloat)bitmapW)
        {
            x = margin;
            y += rect.size.height + margin;
        }
        CFRelease(lineOfText);
    }
    for (CGColorRef color : colors)
    {
        CFRelease(color);
    }
    auto pTextureDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    pTextureDesc->setWidth(bitmapW);
    pTextureDesc->setHeight(bitmapH);
    pTextureDesc->setTextureType(MTL::TextureType2DArray);
    pTextureDesc->setUsage(MTL::TextureUsageShaderRead);
    pTextureDesc->setArrayLength(1);
    pTextureDesc->setStorageMode(MTL::StorageModeShared);
    pTextureDesc->setMipmapLevelCount(1);
    firaCode.texture = NS::TransferPtr(pDevice->newTexture(pTextureDesc.get()));
    firaCode.texture->setLabel(MTLSTR("Fira Code Texture"));
    firaCode.texture->replaceRegion(MTL::Region(0, 0, bitmapW, bitmapH), 0, 0, bitmap, bytesPerRow, bitmapW * bitmapH * bitmapChannels);
    CFRelease(color);
    CFRelease(font);
    CFRelease(ctx);
    CFRelease(colorSpace);
    delete [] bitmap;
    return (firaCode);
}

/* Cube: 36 indices.
Screen Quad: 6 indices.
Char ! UVs: (-5.45296e-31, 1.4013e-45) ...
Char " UVs: (-3.10097e-21, 2.8026e-45) ...
Char # UVs: (4.15101e+28, 1.4013e-45) ...
Char $ UVs: (-2.75632e-21, 2.8026e-45) ...
Char % UVs: (-5.45296e-31, 1.4013e-45) ...
Char & UVs: (-3.10624e-21, 2.8026e-45) ...
Char ' UVs: (4.2039e-45, 0) ...
Char ( UVs: (4.15177e+28, 1.4013e-45) ...
Char ) UVs: (0, 0) ...
Char * UVs: (5.74532e-44, 0) ...
Char + UVs: (1.31722e-43, 0) ...
Char , UVs: (-5.45296e-31, 1.4013e-45) ...
Char - UVs: (-5.45296e-31, 1.4013e-45) ...
Char . UVs: (-2.67626e-21, 2.8026e-45) ...
Char / UVs: (4.15113e+28, 1.4013e-45) ...
Char 0 UVs: (3.60134e-43, 2.35099e-38) ...
Char 1 UVs: (0, 0) ...
Char 2 UVs: (4.06377e-42, 0) ...
Char 3 UVs: (2.38221e-44, 0) ...
Char 4 UVs: (4.15177e+28, 1.4013e-45) ...
Char 5 UVs: (-2.67955e-21, 2.8026e-45) ...
Char 6 UVs: (-2.69144e-21, 2.8026e-45) ...
Char 7 UVs: (-5.45296e-31, 1.4013e-45) ...
Char 8 UVs: (-5.45296e-31, 1.4013e-45) ...
Char 9 UVs: (-2.69889e-21, 2.8026e-45) ...
Char : UVs: (8.40779e-45, 0) ...
Char ; UVs: (4.15177e+28, 1.4013e-45) ...
Char < UVs: (1.17149e-42, 0) ...
Char = UVs: (3.84797e-42, 0) ...
Char > UVs: (1.68156e-44, 0) ...
Char ? UVs: (-5.45296e-31, 1.4013e-45) ...
Char @ UVs: (-2.90838e-21, 2.8026e-45) ...
Char A UVs: (2.8026e-44, 0) ...
Char B UVs: (4.15177e+28, 1.4013e-45) ...
Char C UVs: (-5.45296e-31, 1.4013e-45) ...
Char D UVs: (-2.67837e-21, 2.8026e-45) ...
Char E UVs: (4.15147e+28, 1.4013e-45) ...
Char F UVs: (-5.45296e-31, 1.4013e-45) ...
Char G UVs: (-2.89403e-21, 2.8026e-45) ...
Char H UVs: (1.91698e-42, 0) ...
Char I UVs: (1.4013e-44, 0) ...
Char J UVs: (4.15177e+28, 1.4013e-45) ...
Char K UVs: (2.85874e-20, 0) ...
Char L UVs: (-2.67626e-21, 2.8026e-45) ...
Char M UVs: (4.15159e+28, 1.4013e-45) ...
Char N UVs: (3.60134e-43, 2.35099e-38) ...
Char O UVs: (0, 0) ...
Char P UVs: (4.1518e+28, 1.4013e-45) ...
Char Q UVs: (0, 0) ...
Char R UVs: (-1.0584e+36, 1.4013e-45) ...
Char S UVs: (3.60134e-43, 2.35099e-38) ...
Char T UVs: (0, 0) ...
Char U UVs: (32, 0) ...
Char V UVs: (-1.05804e+36, 1.4013e-45) ...
Char W UVs: (0, 0) ...
Char X UVs: (1.12104e-44, 0) ...
Char Y UVs: (4.15245e+28, 1.4013e-45) ...
Char Z UVs: (-5.1769e-31, 1.4013e-45) ...
Char [ UVs: (1.99972e-38, 1.4013e-45) ...
Char \ UVs: (1.99972e-38, 1.4013e-45) ...
Char ] UVs: (1.4013e-45, 0) ...
Char ^ UVs: (0, 0) ...
Char _ UVs: (-1.05804e+36, 1.4013e-45) ...
Char ` UVs: (-5.19464e+36, 1.4013e-45) ...
Char a UVs: (9.40395e-38, 8.96831e-44) ...
Char b UVs: (0, 0) ...
Char c UVs: (0, 0) ...
Char d UVs: (5.88545e-43, 0) ...
Char e UVs: (4.15187e+28, 1.4013e-45) ...
Char f UVs: (0, 0) ...
Char g UVs: (0, 0) ...
Char h UVs: (9.03229e-40, 2.52435e-29) ...
Char i UVs: (0, 0) ...
Char j UVs: (0, 0) ...
Char k UVs: (0, 0) ...
Char l UVs: (9.02929e-40, 2.52435e-29) ...
Char m UVs: (4.15245e+28, 1.4013e-45) ...
Char n UVs: (0, 0) ...
Char o UVs: (0, 0) ...
Char p UVs: (0, 0) ...
Char q UVs: (0, 0) ...
Char r UVs: (4.15204e+28, 1.4013e-45) ...
Char s UVs: (4.15209e+28, 1.4013e-45) ...
Char t UVs: (4.15206e+28, 1.4013e-45) ...
Char u UVs: (4.15206e+28, 1.4013e-45) ...
Char v UVs: (2.12683e-38, 1.4013e-45) ...
Char w UVs: (4.15206e+28, 1.4013e-45) ...
Char x UVs: (0, 0) ...
Char y UVs: (-1.05665e+36, 1.4013e-45) ...
Char z UVs: (0, 1.4013e-45) ...
Char { UVs: (0, 0) ...
Char | UVs: (-1.05665e+36, 1.4013e-45) ...
Char } UVs: (-1.05665e+36, 1.4013e-45) ...
Char ~ UVs: (0, 0) ...
TextMesh 'Hello': 30 indices. */

Font createFont(MTL::Device* device)
{
    Font fontAtlas;
    const uint32_t bitmapW = 4096;
    const uint32_t bitmapH = 4096;
    const uint32_t bitmapChannels = 4;
    const uint32_t bitmapSize = bitmapW * bitmapH * bitmapChannels;
    const uint32_t bytesPerRow = bitmapW * bitmapChannels;
    uint8_t* bitmap = new uint8_t[bitmapSize];
    ft_memset(bitmap, 0x0, bitmapSize);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(bitmap, bitmapW, bitmapH, 8, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
    CGFloat margin = (CGFloat)bitmapW * 0.120/2.0;
    CGColorRef color = CGColorCreateGenericRGB(1.0, 0.0, 0.0, 1.0);
    CGFloat fontSize = (CGFloat)bitmapW * 0.05;
    CFStringRef fontName = CFSTR("PT Mono"); // Fira Code, Helvetica,
    CTFontRef font = CTFontCreateWithName(fontName, fontSize, nullptr);
    CGFloat x = margin;
    CGFloat y = margin;
    CGRect minBounds = calculateReferenceBounds('X', font, color, ctx);

    static const char printableChars[] =
        " !\"#$%&'()*+,-./0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~";

    for (size_t i = 0; i < strlen(printableChars); ++i)
    {
        char c = printableChars[i];
        
        NSString* str = [NSString stringWithFormat:@"%c", c];
        NSMutableAttributedString* attributedString = [[NSMutableAttributedString alloc] initWithString:str];
        [attributedString addAttribute:NSFontAttributeName value:(__bridge id)font range:NSMakeRange(0, 1)];
        [attributedString addAttribute:NSForegroundColorAttributeName value:(__bridge id)color range:NSMakeRange(0, 1)];
        CTLineRef lineOfText = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributedString);
        CGContextSetTextPosition(ctx, 0, 0);
        CGRect rect = CTLineGetImageBounds(lineOfText, ctx);
        CGSize oldSize = rect.size;
        rect.size.width = std::max(rect.size.width, minBounds.size.width);
        rect.size.height = std::max(rect.size.height, minBounds.size.height);
        CGFloat dx = (rect.size.width - oldSize.width) * 0.5;
        CGFloat dy = (rect.size.height - oldSize.height) * 0.5;
        rect.origin.x -= dx;
        rect.origin.y -= dy;
        CGContextSetTextPosition(ctx, x, y);
        CTLineDraw(lineOfText, ctx);
        Font::CharUV& uvs = fontAtlas.charUV[c];
        uvs.nw = simd_make_float2((x + rect.origin.x) / bitmapW,
                                  (-(y + rect.origin.y) + bitmapH) / bitmapH);
        uvs.ne = simd_make_float2((x + rect.origin.x + rect.size.width) / bitmapW,
                                  (-(y + rect.origin.y) + bitmapH) / bitmapH);
        uvs.se = simd_make_float2((x + rect.origin.x + rect.size.width) / bitmapW,
                                  (-(y + rect.origin.y + rect.size.height) + bitmapH) / bitmapH);
        uvs.sw = simd_make_float2((x + rect.origin.x) / bitmapW,
                                  (-(y + rect.origin.y + rect.size.height) + bitmapH) / bitmapH);
        uvs.width = rect.size.width;
        uvs.height = rect.size.height;

        x += rect.size.width + margin;
        if ((x + rect.size.width + margin / 2) >= (CGFloat)bitmapW)
        {
            x = margin;
            y += rect.size.height + margin;
        }
        CFRelease(lineOfText);
    }

    auto textureDesc = NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    textureDesc->setWidth(bitmapW);
    textureDesc->setHeight(bitmapH);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setUsage(MTL::TextureUsageShaderRead);
    textureDesc->setStorageMode(MTL::StorageModeShared);
    
    fontAtlas.texture = NS::TransferPtr(device->newTexture(textureDesc.get()));
    fontAtlas.texture->setLabel(NS::String::string("UI Font Atlas", NS::UTF8StringEncoding));
    fontAtlas.texture->replaceRegion(MTL::Region(0, 0, bitmapW, bitmapH), 0, 0, bitmap, bytesPerRow, bitmapW * bitmapH * bitmapChannels);//MTL::Region(0, 0, bitmapW, bitmapH), 0, bitmap, bytesPerRow);

    CFRelease(font);
    CGContextRelease(ctx);
    CGColorSpaceRelease(colorSpace);
    delete[] bitmap;
    return fontAtlas;
}
