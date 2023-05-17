#pragma once

struct CRGB;
struct CHSV;

// extern void hsv2rgb_rainbow( const CHSV& hsv, CRGB& rgb);


struct CHSV {
    union {
        struct {
            union {
                uint8_t hue;
                uint8_t h; };
            union {
                uint8_t saturation;
                uint8_t sat;
                uint8_t s; };
            union {
                uint8_t value;
                uint8_t val;
                uint8_t v; };
        };
        uint8_t raw[3];
    };

    inline CHSV() // __attribute__((always_inline))
    {
    }

    inline CHSV( uint8_t ih, uint8_t is, uint8_t iv) // __attribute__((always_inline))
        : h(ih), s(is), v(iv)
    {
    }

    inline CHSV(const CHSV& rhs) // __attribute__((always_inline))
    {
        h = rhs.h;
        s = rhs.s;
        v = rhs.v;
    }

    inline CHSV& operator= (const CHSV& rhs) // __attribute__((always_inline))
    {
        h = rhs.h;
        s = rhs.s;
        v = rhs.v;
        return *this;
    }

    inline CHSV& setHSV(uint8_t ih, uint8_t is, uint8_t iv) // __attribute__((always_inline))
    {
        h = ih;
        s = is;
        v = iv;
        return *this;
    }
};

typedef enum {
    HUE_RED = 0,
    HUE_ORANGE = 32,
    HUE_YELLOW = 64,
    HUE_GREEN = 96,
    HUE_AQUA = 128,
    HUE_BLUE = 160,
    HUE_PURPLE = 192,
    HUE_PINK = 224
} HSVHue;

struct CRGB {
    union {
        struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
        uint8_t raw[3];
    };

    inline uint8_t& operator[] (uint8_t x) // __attribute__((always_inline))
    {
        return raw[x];
    }

    inline const uint8_t& operator[] (uint8_t x) const // __attribute__((always_inline))
    {
        return raw[x];
    }

    // default values are UNINITIALIZED
    inline CRGB() // __attribute__((always_inline))
    {
    }

    inline CRGB( uint8_t ir, uint8_t ig, uint8_t ib)  // __attribute__((always_inline))
        : r(ir), g(ig), b(ib)
    {
    }

    inline CRGB( uint32_t colorcode)  // __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }

    // inline CRGB( LEDColorCorrection colorcode) // __attribute__((always_inline))
    // : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    // {

    // }

    // inline CRGB( ColorTemperature colorcode) // __attribute__((always_inline))
    // : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    // {

    // }

    inline CRGB(const CRGB& rhs) // __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
    }

    // inline CRGB(const CHSV& rhs) // __attribute__((always_inline))
    // {
    //     hsv2rgb_rainbow( rhs, *this);
    // }

    inline CRGB& operator= (const CRGB& rhs) // __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        return *this;
    }

    inline CRGB& operator= (const uint32_t colorcode) // __attribute__((always_inline))
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >>  8) & 0xFF;
        b = (colorcode >>  0) & 0xFF;
        return *this;
    }

    inline CRGB& setRGB (uint8_t nr, uint8_t ng, uint8_t nb) // __attribute__((always_inline))
    {
        r = nr;
        g = ng;
        b = nb;
        return *this;
    }

    // inline CRGB& setHSV (uint8_t hue, uint8_t sat, uint8_t val) // __attribute__((always_inline))
    // {
    //     hsv2rgb_rainbow( CHSV(hue, sat, val), *this);
    //     return *this;
    // }

    // inline CRGB& setHue (uint8_t hue) // __attribute__((always_inline))
    // {
    //     hsv2rgb_rainbow( CHSV(hue, 255, 255), *this);
    //     return *this;
    // }

    inline CRGB& operator= (const CHSV& rhs) // __attribute__((always_inline))
    {
        r = rhs.h;
        g = rhs.s;
        b = rhs.v;
        return *this;
    }

    inline CRGB& setColorCode (uint32_t colorcode) // __attribute__((always_inline))
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >>  8) & 0xFF;
        b = (colorcode >>  0) & 0xFF;
        return *this;
    }


    // inline CRGB& operator+= (const CRGB& rhs )
    // {
    //     r = qadd8( r, rhs.r);
    //     g = qadd8( g, rhs.g);
    //     b = qadd8( b, rhs.b);
    //     return *this;
    // }

    // inline CRGB& addToRGB (uint8_t d )
    // {
    //     r = qadd8( r, d);
    //     g = qadd8( g, d);
    //     b = qadd8( b, d);
    //     return *this;
    // }

    // inline CRGB& operator-= (const CRGB& rhs )
    // {
    //     r = qsub8( r, rhs.r);
    //     g = qsub8( g, rhs.g);
    //     b = qsub8( b, rhs.b);
    //     return *this;
    // }

    // inline CRGB& subtractFromRGB(uint8_t d )
    // {
    //     r = qsub8( r, d);
    //     g = qsub8( g, d);
    //     b = qsub8( b, d);
    //     return *this;
    // }

    // inline CRGB& operator-- ()  // __attribute__((always_inline))
    // {
    //     subtractFromRGB(1);
    //     return *this;
    // }

    // inline CRGB operator-- (int )  // __attribute__((always_inline))
    // {
    //     CRGB retval(*this);
    //     --(*this);
    //     return retval;
    // }

    // inline CRGB& operator++ ()  // __attribute__((always_inline))
    // {
    //     addToRGB(1);
    //     return *this;
    // }

    // inline CRGB operator++ (int )  // __attribute__((always_inline))
    // {
    //     CRGB retval(*this);
    //     ++(*this);
    //     return retval;
    // }

    inline CRGB& operator/= (uint8_t d )
    {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }

    inline CRGB& operator>>= (uint8_t d)
    {
      r >>= d;
      g >>= d;
      b >>= d;
      return *this;
    }

    // inline CRGB& operator*= (uint8_t d )
    // {
    //     r = qmul8( r, d);
    //     g = qmul8( g, d);
    //     b = qmul8( b, d);
    //     return *this;
    // }

    // inline CRGB& nscale8_video (uint8_t scaledown )
    // {
    //     nscale8x3_video( r, g, b, scaledown);
    //     return *this;
    // }

    // inline CRGB& operator%= (uint8_t scaledown )
    // {
    //     nscale8x3_video( r, g, b, scaledown);
    //     return *this;
    // }

    // inline CRGB& fadeLightBy (uint8_t fadefactor )
    // {
    //     nscale8x3_video( r, g, b, 255 - fadefactor);
    //     return *this;
    // }

    // inline CRGB& nscale8 (uint8_t scaledown )
    // {
    //     nscale8x3( r, g, b, scaledown);
    //     return *this;
    // }

    // inline CRGB& nscale8 (const CRGB & scaledown )
    // {
    //     r = ::scale8(r, scaledown.r);
    //     g = ::scale8(g, scaledown.g);
    //     b = ::scale8(b, scaledown.b);
    //     return *this;
    // }

    // inline CRGB scale8 (const CRGB & scaledown ) const
    // {
    //     CRGB out;
    //     out.r = ::scale8(r, scaledown.r);
    //     out.g = ::scale8(g, scaledown.g);
    //     out.b = ::scale8(b, scaledown.b);
    //     return out;
    // }

    // inline CRGB& fadeToBlackBy (uint8_t fadefactor )
    // {
    //     nscale8x3( r, g, b, 255 - fadefactor);
    //     return *this;
    // }

    inline CRGB& operator|= (const CRGB& rhs )
    {
        if( rhs.r > r) r = rhs.r;
        if( rhs.g > g) g = rhs.g;
        if( rhs.b > b) b = rhs.b;
        return *this;
    }

    inline CRGB& operator|= (uint8_t d )
    {
        if( d > r) r = d;
        if( d > g) g = d;
        if( d > b) b = d;
        return *this;
    }

    inline CRGB& operator&= (const CRGB& rhs )
    {
        if( rhs.r < r) r = rhs.r;
        if( rhs.g < g) g = rhs.g;
        if( rhs.b < b) b = rhs.b;
        return *this;
    }

    inline CRGB& operator&= (uint8_t d )
    {
        if( d < r) r = d;
        if( d < g) g = d;
        if( d < b) b = d;
        return *this;
    }

    inline operator bool() const // __attribute__((always_inline))
    {
        return r || g || b;
    }

    inline CRGB operator- ()
    {
        CRGB retval;
        retval.r = 255 - r;
        retval.g = 255 - g;
        retval.b = 255 - b;
        return retval;
    }

//     inline uint8_t getLuma ( )  const {
//         //Y' = 0.2126 R' + 0.7152 G' + 0.0722 B'
//         //     54            183       18 (!)

//         uint8_t luma = scale8_LEAVING_R1_DIRTY( r, 54) + \
//         scale8_LEAVING_R1_DIRTY( g, 183) + \
//         scale8_LEAVING_R1_DIRTY( b, 18);
//         cleanup_R1();
//         return luma;
//     }

//     inline uint8_t getAverageLight( )  const {
// #if FASTLED_SCALE8_FIXED == 1
//         const uint8_t eightyfive = 85;
// #else
//         const uint8_t eightyfive = 86;
// #endif
//         uint8_t avg = scale8_LEAVING_R1_DIRTY( r, eightyfive) + \
//         scale8_LEAVING_R1_DIRTY( g, eightyfive) + \
//         scale8_LEAVING_R1_DIRTY( b, eightyfive);
//         cleanup_R1();
//         return avg;
//     }

    inline void maximizeBrightness( uint8_t limit = 255 )  {
        uint8_t max = red;
        if( green > max) max = green;
        if( blue > max) max = blue;
        uint16_t factor = ((uint16_t)(limit) * 256) / max;
        red =   (red   * factor) / 256;
        green = (green * factor) / 256;
        blue =  (blue  * factor) / 256;
    }

    // inline uint8_t getParity()
    // {
    //     uint8_t sum = r + g + b;
    //     return (sum & 0x01);
    // }

    // inline void setParity( uint8_t parity)
    // {
    //     uint8_t curparity = getParity();

    //     if( parity == curparity) return;

    //     if( parity ) {
    //         // going 'up'
    //         if( (b > 0) && (b < 255)) {
    //             if( r == g && g == b) {
    //                 r++;
    //                 g++;
    //             }
    //             b++;
    //         } else if( (r > 0) && (r < 255)) {
    //             r++;
    //         } else if( (g > 0) && (g < 255)) {
    //             g++;
    //         } else {
    //             if( r == g && g == b) {
    //                 r ^= 0x01;
    //                 g ^= 0x01;
    //             }
    //             b ^= 0x01;
    //         }
    //     } else {
    //         // going 'down'
    //         if( b > 1) {
    //             if( r == g && g == b) {
    //                 r--;
    //                 g--;
    //             }
    //             b--;
    //         } else if( g > 1) {
    //             g--;
    //         } else if( r > 1) {
    //             r--;
    //         } else {
    //             if( r == g && g == b) {
    //                 r ^= 0x01;
    //                 g ^= 0x01;
    //             }
    //             b ^= 0x01;
    //         }
    //     }
    // }

    typedef enum {
        // AliceBlue=0xF0F8FF,
        // Amethyst=0x9966CC,
        // AntiqueWhite=0xFAEBD7,
        // Aqua=0x00FFFF,
        // Aquamarine=0x7FFFD4,
        // Azure=0xF0FFFF,
        // Beige=0xF5F5DC,
        // Bisque=0xFFE4C4,
        Black=0x000000,
        // BlanchedAlmond=0xFFEBCD,
        Blue=0x0000FF,
        // BlueViolet=0x8A2BE2,
        // Brown=0xA52A2A,
        // BurlyWood=0xDEB887,
        // CadetBlue=0x5F9EA0,
        // Chartreuse=0x7FFF00,
        // Chocolate=0xD2691E,
        // Coral=0xFF7F50,
        // CornflowerBlue=0x6495ED,
        // Cornsilk=0xFFF8DC,
        // Crimson=0xDC143C,
        // Cyan=0x00FFFF,
        // DarkBlue=0x00008B,
        // DarkCyan=0x008B8B,
        // DarkGoldenrod=0xB8860B,
        // DarkGray=0xA9A9A9,
        // DarkGrey=0xA9A9A9,
        // DarkGreen=0x006400,
        // DarkKhaki=0xBDB76B,
        // DarkMagenta=0x8B008B,
        // DarkOliveGreen=0x556B2F,
        // DarkOrange=0xFF8C00,
        // DarkOrchid=0x9932CC,
        // DarkRed=0x8B0000,
        // DarkSalmon=0xE9967A,
        // DarkSeaGreen=0x8FBC8F,
        // DarkSlateBlue=0x483D8B,
        // DarkSlateGray=0x2F4F4F,
        // DarkSlateGrey=0x2F4F4F,
        // DarkTurquoise=0x00CED1,
        // DarkViolet=0x9400D3,
        // DeepPink=0xFF1493,
        // DeepSkyBlue=0x00BFFF,
        // DimGray=0x696969,
        // DimGrey=0x696969,
        // DodgerBlue=0x1E90FF,
        // FireBrick=0xB22222,
        // FloralWhite=0xFFFAF0,
        // ForestGreen=0x228B22,
        // Fuchsia=0xFF00FF,
        // Gainsboro=0xDCDCDC,
        // GhostWhite=0xF8F8FF,
        // Gold=0xFFD700,
        // Goldenrod=0xDAA520,
        // Gray=0x808080,
        // Grey=0x808080,
        // Green=0x008000,
        // GreenYellow=0xADFF2F,
        // Honeydew=0xF0FFF0,
        // HotPink=0xFF69B4,
        // IndianRed=0xCD5C5C,
        // Indigo=0x4B0082,
        // Ivory=0xFFFFF0,
        // Khaki=0xF0E68C,
        // Lavender=0xE6E6FA,
        // LavenderBlush=0xFFF0F5,
        // LawnGreen=0x7CFC00,
        // LemonChiffon=0xFFFACD,
        // LightBlue=0xADD8E6,
        // LightCoral=0xF08080,
        // LightCyan=0xE0FFFF,
        // LightGoldenrodYellow=0xFAFAD2,
        // LightGreen=0x90EE90,
        // LightGrey=0xD3D3D3,
        // LightPink=0xFFB6C1,
        // LightSalmon=0xFFA07A,
        // LightSeaGreen=0x20B2AA,
        // LightSkyBlue=0x87CEFA,
        // LightSlateGray=0x778899,
        // LightSlateGrey=0x778899,
        // LightSteelBlue=0xB0C4DE,
        // LightYellow=0xFFFFE0,
        // Lime=0x00FF00,
        // LimeGreen=0x32CD32,
        // Linen=0xFAF0E6,
        // Magenta=0xFF00FF,
        // Maroon=0x800000,
        // MediumAquamarine=0x66CDAA,
        // MediumBlue=0x0000CD,
        // MediumOrchid=0xBA55D3,
        // MediumPurple=0x9370DB,
        // MediumSeaGreen=0x3CB371,
        // MediumSlateBlue=0x7B68EE,
        // MediumSpringGreen=0x00FA9A,
        // MediumTurquoise=0x48D1CC,
        // MediumVioletRed=0xC71585,
        // MidnightBlue=0x191970,
        // MintCream=0xF5FFFA,
        // MistyRose=0xFFE4E1,
        // Moccasin=0xFFE4B5,
        // NavajoWhite=0xFFDEAD,
        // Navy=0x000080,
        // OldLace=0xFDF5E6,
        // Olive=0x808000,
        // OliveDrab=0x6B8E23,
        // Orange=0xFFA500,
        // OrangeRed=0xFF4500,
        // Orchid=0xDA70D6,
        // PaleGoldenrod=0xEEE8AA,
        // PaleGreen=0x98FB98,
        // PaleTurquoise=0xAFEEEE,
        // PaleVioletRed=0xDB7093,
        // PapayaWhip=0xFFEFD5,
        // PeachPuff=0xFFDAB9,
        // Peru=0xCD853F,
        // Pink=0xFFC0CB,
        // Plaid=0xCC5533,
        // Plum=0xDDA0DD,
        // PowderBlue=0xB0E0E6,
        // Purple=0x800080,
        Red=0xFF0000,
        // RosyBrown=0xBC8F8F,
        // RoyalBlue=0x4169E1,
        // SaddleBrown=0x8B4513,
        // Salmon=0xFA8072,
        // SandyBrown=0xF4A460,
        // SeaGreen=0x2E8B57,
        // Seashell=0xFFF5EE,
        // Sienna=0xA0522D,
        // Silver=0xC0C0C0,
        // SkyBlue=0x87CEEB,
        // SlateBlue=0x6A5ACD,
        // SlateGray=0x708090,
        // SlateGrey=0x708090,
        // Snow=0xFFFAFA,
        // SpringGreen=0x00FF7F,
        // SteelBlue=0x4682B4,
        // Tan=0xD2B48C,
        // Teal=0x008080,
        // Thistle=0xD8BFD8,
        // Tomato=0xFF6347,
        // Turquoise=0x40E0D0,
        // Violet=0xEE82EE,
        // Wheat=0xF5DEB3,
        White=0xFFFFFF,
        // WhiteSmoke=0xF5F5F5,
        Yellow=0xFFFF00,
        // YellowGreen=0x9ACD32,

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight=0xFFE42D,
        // If you are using no color correction, use this
        FairyLightNCC=0xFF9D2A

    } HTMLColorCode;
};


inline __attribute__((always_inline)) bool operator== (const CRGB& lhs, const CRGB& rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}

inline __attribute__((always_inline)) bool operator!= (const CRGB& lhs, const CRGB& rhs)
{
    return !(lhs == rhs);
}

inline __attribute__((always_inline)) bool operator< (const CRGB& lhs, const CRGB& rhs)
{
    uint16_t sl, sr;
    sl = lhs.r + lhs.g + lhs.b;
    sr = rhs.r + rhs.g + rhs.b;
    return sl < sr;
}

inline __attribute__((always_inline)) bool operator> (const CRGB& lhs, const CRGB& rhs)
{
    uint16_t sl, sr;
    sl = lhs.r + lhs.g + lhs.b;
    sr = rhs.r + rhs.g + rhs.b;
    return sl > sr;
}

inline __attribute__((always_inline)) bool operator>= (const CRGB& lhs, const CRGB& rhs)
{
    uint16_t sl, sr;
    sl = lhs.r + lhs.g + lhs.b;
    sr = rhs.r + rhs.g + rhs.b;
    return sl >= sr;
}

inline __attribute__((always_inline)) bool operator<= (const CRGB& lhs, const CRGB& rhs)
{
    uint16_t sl, sr;
    sl = lhs.r + lhs.g + lhs.b;
    sr = rhs.r + rhs.g + rhs.b;
    return sl <= sr;
}


// __attribute__((always_inline))
// inline CRGB operator+( const CRGB& p1, const CRGB& p2)
// {
//     return CRGB( qadd8( p1.r, p2.r),
//                  qadd8( p1.g, p2.g),
//                  qadd8( p1.b, p2.b));
// }

// __attribute__((always_inline))
// inline CRGB operator-( const CRGB& p1, const CRGB& p2)
// {
//     return CRGB( qsub8( p1.r, p2.r),
//                  qsub8( p1.g, p2.g),
//                  qsub8( p1.b, p2.b));
// }

// __attribute__((always_inline))
// inline CRGB operator*( const CRGB& p1, uint8_t d)
// {
//     return CRGB( qmul8( p1.r, d),
//                  qmul8( p1.g, d),
//                  qmul8( p1.b, d));
// }

// __attribute__((always_inline))
inline CRGB operator/( const CRGB& p1, uint8_t d)
{
    return CRGB( p1.r/d, p1.g/d, p1.b/d);
}


// __attribute__((always_inline))
inline CRGB operator&( const CRGB& p1, const CRGB& p2)
{
    return CRGB( p1.r < p2.r ? p1.r : p2.r,
                 p1.g < p2.g ? p1.g : p2.g,
                 p1.b < p2.b ? p1.b : p2.b);
}

// __attribute__((always_inline))
inline CRGB operator|( const CRGB& p1, const CRGB& p2)
{
    return CRGB( p1.r > p2.r ? p1.r : p2.r,
                 p1.g > p2.g ? p1.g : p2.g,
                 p1.b > p2.b ? p1.b : p2.b);
}

// __attribute__((always_inline))
// inline CRGB operator%( const CRGB& p1, uint8_t d)
// {
//     CRGB retval( p1);
//     retval.nscale8_video( d);
//     return retval;
// }



enum EOrder {
    RGB=0012,
    RBG=0021,
    GRB=0102,
    GBR=0120,
    BRG=0201,
    BGR=0210
};

struct CHSV rgb2hsv_approximate(struct CRGB rgb) { return CHSV(rgb.r, rgb.g, rgb.b); }

#define TypicalLEDStrip 333
typedef bool WS2812B;

typedef struct CFastLED {
  int brightness;

  CFastLED setBrightness(int b) { brightness = b; return *this; }
  CFastLED setCorrection(int) { return *this; }

  void show() {};

  template<typename T1, uint8_t T2, EOrder T3>
  CFastLED addLeds(struct CRGB *data, int nLedsOrOffset, int nLedsIfOffset = 0) { return *this; }

} CFastLED;
