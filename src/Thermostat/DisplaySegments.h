#pragma once

#include <TM1637Display.h>  // https://github.com/avishorp/TM1637

class DisplaySegments
{
  public:
    DisplaySegments(int pinClk, int pinDio)
      : _display(pinClk, pinDio)
      
    {}

    // Declare an enum so the functions with overloads to take multiple argments can
    // tell the difference from a letter vs the optional position to begin the text.
    enum Position {
      PosFirst,
      PosSecond,
      PosThird,
      PosForth,
    };

    enum Brightness : uint8_t {
      LedMin,
      Led1,
      Led2,
      Led3,
      Led4,
      Led5,
      Led6,
      LedMax
    };

    static const char Degree = 0xB0;

    TM1637Display& TM1637()
    {
      return _display;
    };

    bool showChar(char c, Position pos = Position::PosFirst)
    {
      bool ret = true;
      uint8_t segments = _getSegments(c);
      if ( Segments_ERR == segments )
      {
        ret = false;
      }
      _display.setSegments(&segments, 1, pos);
      return ret;
    }

    bool showText(char * text, Position pos = Position::PosFirst)
    {
      bool ret = true;
      uint8_t displaySegments[_digits];
      for ( int i = pos; i < _digits; ++i )
      {
        char c = *text ? *text++ : ' ';
        displaySegments[i] = _getSegments(c);
        if ( Segments_ERR == displaySegments[i] )
        {
          ret = false;
        }
      }
      _display.setSegments(displaySegments + pos, _digits - pos, pos);
      return ret;
    }

    bool scrollText(char * text, unsigned long msDelay)
    {
      bool ret = true;
      for ( Position pos = Position::PosForth; pos > Position::PosFirst; pos = pos - 1 )
      {
        if ( !showText(text, pos) )
        {
          ret = false;
        }
        delay(msDelay);
      }
      for ( char * remainder = text; *remainder; ++remainder )
      {
        if ( !showText(remainder) )
        {
          ret = false;
        }
        delay(msDelay);
      }
      _display.clear();
      return ret;
    }

    void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, Position pos = Position::PosFirst)
    {
      _display.showNumberDec(num, leading_zero, length, pos);
    }

    void setBrightness(Brightness brightness, bool on = true)
    {
      _display.setBrightness(brightness, on);
    }

    void clear()
    {
      _display.clear();
    }

  private:
    _getSegments(char c)
    {
      // Would be interesting to create a 255 sized array for all possible characters
      // rather than this a bunch of one off handling of letters, numbers, and special characters.
      static const uint8_t letters[] =
      {
        Segments_A,
        Segments_b,
        Segments_c,
        Segments_d,
        Segments_E,
        Segments_F,
        Segments_g,
        Segments_h,
        Segments_i,
        Segments_j,
        Segments_K,
        Segments_L,
        Segments_M,
        Segments_n,
        Segments_o,
        Segments_P,
        Segments_q,
        Segments_r,
        Segments_S,
        Segments_t,
        Segments_u,
        Segments_V,
        Segments_W,
        Segments_X,
        Segments_Y,
        Segments_Z,
      };

      if ( (c >= 'A') && (c <= 'Z') )
      {
        return letters[c - 'A'];
      }
      else if ( (c >= 'a') && (c <= 'z') )
      {
        return letters[c - 'a'];
      }
      else if ( (c >= '0') && (c <= '9') )
      {
        return _display.encodeDigit(c - '0');
      }
      switch ( c )
      {
        case ' ':
          return Segments_space;
        case '-':
          return Segments_dash;
        case '_':
          return Segments_underscore;
        case '/':
          return Segments_forwardslash;
        case '\\':
          return Segments_backslash;
        case '!':
          return Segments_exclamation;
        case '?':
          return Segments_question;
        case '=':
          return Segments_equals;
        case '[':
          return Segments_bracket_open;
        case ']':
          return Segments_bracket_close;
        case '(':
          return Segments_paran_open;
        case ')':
          return Segments_paran_close;
        case Degree:
          return Segments_degree;
        default:
          return Segments_ERR;
      }
    }

  private:
    TM1637Display _display;

    static const size_t _digits = 4;

    static const uint8_t Segments_A = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_b = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_C = SEG_A | SEG_D | SEG_E | SEG_F;  // Avoid as not to confuse with [
    static const uint8_t Segments_c = SEG_D | SEG_E | SEG_G;
    static const uint8_t Segments_d = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
    static const uint8_t Segments_E = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_F = SEG_A | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_G = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F;  // Avoid as g looks better
    static const uint8_t Segments_g = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
    static const uint8_t Segments_H = SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;  // Avoid as not to confuse with X
    static const uint8_t Segments_h = SEG_C | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_I = SEG_B | SEG_C;  // Avoid as to not confuse with 1 (one)
    static const uint8_t Segments_i = SEG_A | SEG_C;
    static const uint8_t Segments_j = SEG_A | SEG_C | SEG_D; 
    static const uint8_t Segments_K = SEG_A | SEG_C | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_L = SEG_D | SEG_E | SEG_F;
    static const uint8_t Segments_l = SEG_E | SEG_F;  // Avoid as to not confuse with 1 or I
    static const uint8_t Segments_M = SEG_A | SEG_C | SEG_E;
    static const uint8_t Segments_N = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F;  // Avoid as n looks better
    static const uint8_t Segments_n = SEG_C | SEG_E | SEG_G;
    static const uint8_t Segments_O = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;  // Avoid as to not confuse with 0 (zero)
    static const uint8_t Segments_o = SEG_C | SEG_D | SEG_E | SEG_G;
    static const uint8_t Segments_P = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_q = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G;
    static const uint8_t Segments_r = SEG_E | SEG_G;
    static const uint8_t Segments_S = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G;
    static const uint8_t Segments_t = SEG_D | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_U = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;  // Avoid as not to confuse with V
    static const uint8_t Segments_u = SEG_C | SEG_D | SEG_E;
    static const uint8_t Segments_V = Segments_U;
    static const uint8_t Segments_W = SEG_B | SEG_D | SEG_F;
    static const uint8_t Segments_X = Segments_H;
    static const uint8_t Segments_Y = SEG_B | SEG_E | SEG_F | SEG_G;
    static const uint8_t Segments_Z = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G;
    static const uint8_t Segments_space = 0x00;
    static const uint8_t Segments_dash = SEG_G;
    static const uint8_t Segments_underscore = SEG_D;
    static const uint8_t Segments_forwardslash = SEG_C | SEG_F;
    static const uint8_t Segments_backslash = SEG_B | SEG_E;
    static const uint8_t Segments_exclamation = SEG_D | SEG_F;
    static const uint8_t Segments_question = SEG_A | SEG_B | SEG_D | SEG_G;
    static const uint8_t Segments_equals = SEG_D | SEG_G;
    static const uint8_t Segments_bracket_open = Segments_C;
    static const uint8_t Segments_bracket_close = SEG_A | SEG_B | SEG_C | SEG_D;
    static const uint8_t Segments_paran_open = Segments_bracket_open;
    static const uint8_t Segments_paran_close = Segments_bracket_close;
    static const uint8_t Segments_degree = SEG_A | SEG_B | SEG_F | SEG_G;

    static const uint8_t Segments_ERR = 0x80;
};

