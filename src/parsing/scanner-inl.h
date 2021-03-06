// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_PARSING_SCANNER_INL_H_
#define V8_PARSING_SCANNER_INL_H_

#include "src/char-predicates-inl.h"
#include "src/parsing/scanner.h"
#include "src/unicode-cache-inl.h"

namespace v8 {
namespace internal {

// Make sure tokens are stored as a single byte.
STATIC_ASSERT(sizeof(Token::Value) == 1);

// Table of one-character tokens, by character (0x00..0x7F only).
// clang-format off
static const Token::Value one_char_tokens[] = {
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::LPAREN,       // 0x28
  Token::RPAREN,       // 0x29
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::COMMA,        // 0x2C
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::COLON,        // 0x3A
  Token::SEMICOLON,    // 0x3B
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::CONDITIONAL,  // 0x3F
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::LBRACK,     // 0x5B
  Token::ILLEGAL,
  Token::RBRACK,     // 0x5D
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::ILLEGAL,
  Token::LBRACE,       // 0x7B
  Token::ILLEGAL,
  Token::RBRACE,       // 0x7D
  Token::BIT_NOT,      // 0x7E
  Token::ILLEGAL
};
// clang-format on

// ----------------------------------------------------------------------------
// Keyword Matcher

#define KEYWORDS(KEYWORD_GROUP, KEYWORD)                    \
  KEYWORD_GROUP('a')                                        \
  KEYWORD("async", Token::ASYNC)                            \
  KEYWORD("await", Token::AWAIT)                            \
  KEYWORD_GROUP('b')                                        \
  KEYWORD("break", Token::BREAK)                            \
  KEYWORD_GROUP('c')                                        \
  KEYWORD("case", Token::CASE)                              \
  KEYWORD("catch", Token::CATCH)                            \
  KEYWORD("class", Token::CLASS)                            \
  KEYWORD("const", Token::CONST)                            \
  KEYWORD("continue", Token::CONTINUE)                      \
  KEYWORD_GROUP('d')                                        \
  KEYWORD("debugger", Token::DEBUGGER)                      \
  KEYWORD("default", Token::DEFAULT)                        \
  KEYWORD("delete", Token::DELETE)                          \
  KEYWORD("do", Token::DO)                                  \
  KEYWORD_GROUP('e')                                        \
  KEYWORD("else", Token::ELSE)                              \
  KEYWORD("enum", Token::ENUM)                              \
  KEYWORD("export", Token::EXPORT)                          \
  KEYWORD("extends", Token::EXTENDS)                        \
  KEYWORD_GROUP('f')                                        \
  KEYWORD("false", Token::FALSE_LITERAL)                    \
  KEYWORD("finally", Token::FINALLY)                        \
  KEYWORD("for", Token::FOR)                                \
  KEYWORD("function", Token::FUNCTION)                      \
  KEYWORD_GROUP('i')                                        \
  KEYWORD("if", Token::IF)                                  \
  KEYWORD("implements", Token::FUTURE_STRICT_RESERVED_WORD) \
  KEYWORD("import", Token::IMPORT)                          \
  KEYWORD("in", Token::IN)                                  \
  KEYWORD("instanceof", Token::INSTANCEOF)                  \
  KEYWORD("interface", Token::FUTURE_STRICT_RESERVED_WORD)  \
  KEYWORD_GROUP('l')                                        \
  KEYWORD("let", Token::LET)                                \
  KEYWORD_GROUP('n')                                        \
  KEYWORD("new", Token::NEW)                                \
  KEYWORD("null", Token::NULL_LITERAL)                      \
  KEYWORD_GROUP('p')                                        \
  KEYWORD("package", Token::FUTURE_STRICT_RESERVED_WORD)    \
  KEYWORD("private", Token::FUTURE_STRICT_RESERVED_WORD)    \
  KEYWORD("protected", Token::FUTURE_STRICT_RESERVED_WORD)  \
  KEYWORD("public", Token::FUTURE_STRICT_RESERVED_WORD)     \
  KEYWORD_GROUP('r')                                        \
  KEYWORD("return", Token::RETURN)                          \
  KEYWORD_GROUP('s')                                        \
  KEYWORD("static", Token::STATIC)                          \
  KEYWORD("super", Token::SUPER)                            \
  KEYWORD("switch", Token::SWITCH)                          \
  KEYWORD_GROUP('t')                                        \
  KEYWORD("this", Token::THIS)                              \
  KEYWORD("throw", Token::THROW)                            \
  KEYWORD("true", Token::TRUE_LITERAL)                      \
  KEYWORD("try", Token::TRY)                                \
  KEYWORD("typeof", Token::TYPEOF)                          \
  KEYWORD_GROUP('v')                                        \
  KEYWORD("var", Token::VAR)                                \
  KEYWORD("void", Token::VOID)                              \
  KEYWORD_GROUP('w')                                        \
  KEYWORD("while", Token::WHILE)                            \
  KEYWORD("with", Token::WITH)                              \
  KEYWORD_GROUP('y')                                        \
  KEYWORD("yield", Token::YIELD)

V8_INLINE Token::Value KeywordOrIdentifierToken(const uint8_t* input,
                                                int input_length) {
  DCHECK_GE(input_length, 1);
  const int kMinLength = 2;
  const int kMaxLength = 10;
  if (input_length < kMinLength || input_length > kMaxLength) {
    return Token::IDENTIFIER;
  }
  switch (input[0]) {
    default:
#define KEYWORD_GROUP_CASE(ch) \
  break;                       \
  case ch:
#define KEYWORD(keyword, token)                                           \
  {                                                                       \
    /* 'keyword' is a char array, so sizeof(keyword) is */                \
    /* strlen(keyword) plus 1 for the NUL char. */                        \
    const int keyword_length = sizeof(keyword) - 1;                       \
    STATIC_ASSERT(keyword_length >= kMinLength);                          \
    STATIC_ASSERT(keyword_length <= kMaxLength);                          \
    DCHECK_EQ(input[0], keyword[0]);                                      \
    DCHECK(token == Token::FUTURE_STRICT_RESERVED_WORD ||                 \
           0 == strncmp(keyword, Token::String(token), sizeof(keyword))); \
    if (input_length == keyword_length && input[1] == keyword[1] &&       \
        (keyword_length <= 2 || input[2] == keyword[2]) &&                \
        (keyword_length <= 3 || input[3] == keyword[3]) &&                \
        (keyword_length <= 4 || input[4] == keyword[4]) &&                \
        (keyword_length <= 5 || input[5] == keyword[5]) &&                \
        (keyword_length <= 6 || input[6] == keyword[6]) &&                \
        (keyword_length <= 7 || input[7] == keyword[7]) &&                \
        (keyword_length <= 8 || input[8] == keyword[8]) &&                \
        (keyword_length <= 9 || input[9] == keyword[9]) &&                \
        (keyword_length <= 10 || input[10] == keyword[10])) {             \
      return token;                                                       \
    }                                                                     \
  }
      KEYWORDS(KEYWORD_GROUP_CASE, KEYWORD)
  }
  return Token::IDENTIFIER;
#undef KEYWORDS
#undef KEYWORD
#undef KEYWORD_GROUP_CASE
}

V8_INLINE Token::Value Scanner::ScanIdentifierOrKeyword() {
  LiteralScope literal(this);
  return ScanIdentifierOrKeywordInner(&literal);
}

// Character flags for the fast path of scanning a keyword or identifier token.
enum class ScanFlags : uint8_t {
  kTerminatesLiteral = 1 << 0,
  // "Cannot" rather than "can" so that this flag can be ORed together across
  // multiple characters.
  kCannotBeKeyword = 1 << 1,
  kStringTerminator = 1 << 2,
  kNeedsSlowPath = 1 << 3,
};
constexpr uint8_t GetScanFlags(char c) {
  return
      // Keywords are all lowercase and only contain letters.
      // Note that non-identifier characters do not set this flag, so
      // that it plays well with kTerminatesLiteral
      // TODO(leszeks): We could probably get an even tighter measure
      // here if not all letters are present in keywords.
      (IsAsciiIdentifier(c) && !IsInRange(c, 'a', 'z')
           ? static_cast<uint8_t>(ScanFlags::kCannotBeKeyword)
           : 0) |
      // Anything that isn't an identifier character will terminate the
      // literal, or at least terminates the literal fast path processing
      // (like an escape).
      (!IsAsciiIdentifier(c)
           ? static_cast<uint8_t>(ScanFlags::kTerminatesLiteral)
           : 0) |
      // Possible string termination characters.
      ((c == '\'' || c == '"' || c == '\n' || c == '\r' || c == '\\')
           ? static_cast<uint8_t>(ScanFlags::kStringTerminator)
           : 0) |
      // Escapes are processed on the slow path.
      (c == '\\' ? static_cast<uint8_t>(ScanFlags::kNeedsSlowPath) : 0);
}
inline bool TerminatesLiteral(uint8_t scan_flags) {
  return (scan_flags & static_cast<uint8_t>(ScanFlags::kTerminatesLiteral));
}
inline bool CanBeKeyword(uint8_t scan_flags) {
  return !(scan_flags & static_cast<uint8_t>(ScanFlags::kCannotBeKeyword));
}
inline bool NeedsSlowPath(uint8_t scan_flags) {
  return (scan_flags & static_cast<uint8_t>(ScanFlags::kNeedsSlowPath));
}
inline bool MayTerminateString(uint8_t scan_flags) {
  return (scan_flags & static_cast<uint8_t>(ScanFlags::kStringTerminator));
}
// Table of precomputed scan flags for the 128 ASCII characters, for branchless
// flag calculation during the scan.
static constexpr const uint8_t character_scan_flags[128] = {
#define CALL_GET_SCAN_FLAGS(N) GetScanFlags(N),
    INT_0_TO_127_LIST(CALL_GET_SCAN_FLAGS)
#undef CALL_GET_SCAN_FLAGS
};

V8_INLINE Token::Value Scanner::ScanIdentifierOrKeywordInner(
    LiteralScope* literal) {
  DCHECK(unicode_cache_->IsIdentifierStart(c0_));
  bool escaped = false;

  STATIC_ASSERT(arraysize(character_scan_flags) == kMaxAscii + 1);
  if (V8_LIKELY(static_cast<uint32_t>(c0_) <= kMaxAscii)) {
    if (V8_LIKELY(c0_ != '\\')) {
      uint8_t scan_flags = character_scan_flags[c0_];
      DCHECK(!TerminatesLiteral(scan_flags));
      AddLiteralChar(static_cast<char>(c0_));
      AdvanceUntil([this, &scan_flags](uc32 c0) {
        if (V8_UNLIKELY(static_cast<uint32_t>(c0) > kMaxAscii)) {
          // A non-ascii character means we need to drop through to the slow
          // path.
          // TODO(leszeks): This would be most efficient as a goto to the slow
          // path, check codegen and maybe use a bool instead.
          scan_flags |= static_cast<uint8_t>(ScanFlags::kNeedsSlowPath);
          return true;
        }
        uint8_t char_flags = character_scan_flags[c0];
        scan_flags |= char_flags;
        if (TerminatesLiteral(char_flags)) {
          return true;
        } else {
          AddLiteralChar(static_cast<char>(c0));
          return false;
        }
      });

      if (V8_LIKELY(!NeedsSlowPath(scan_flags))) {
        if (CanBeKeyword(scan_flags)) {
          // Could be a keyword or identifier.
          Vector<const uint8_t> chars = next().literal_chars.one_byte_literal();
          Token::Value token =
              KeywordOrIdentifierToken(chars.start(), chars.length());
          if (token == Token::IDENTIFIER ||
              token == Token::FUTURE_STRICT_RESERVED_WORD) {
            literal->Complete();
          }
          return token;
        } else {
          literal->Complete();
          return Token::IDENTIFIER;
        }
      }
    } else {
      // Special case for escapes at the start of an identifier.
      escaped = true;
      uc32 c = ScanIdentifierUnicodeEscape();
      DCHECK(!unicode_cache_->IsIdentifierStart(-1));
      if (c == '\\' || !unicode_cache_->IsIdentifierStart(c)) {
        return Token::ILLEGAL;
      }
      AddLiteralChar(c);
    }
  }

  return ScanIdentifierOrKeywordInnerSlow(literal, escaped);
}

V8_INLINE Token::Value Scanner::SkipWhiteSpace() {
  int start_position = source_pos();

  // We won't skip behind the end of input.
  DCHECK(!unicode_cache_->IsWhiteSpaceOrLineTerminator(kEndOfInput));

  // Advance as long as character is a WhiteSpace or LineTerminator.
  while (unicode_cache_->IsWhiteSpaceOrLineTerminator(c0_)) {
    if (!next().after_line_terminator && unibrow::IsLineTerminator(c0_)) {
      next().after_line_terminator = true;
    }
    Advance();
  }

  // Return whether or not we skipped any characters.
  if (source_pos() == start_position) {
    DCHECK_NE('0', c0_);
    return Token::ILLEGAL;
  }

  return Token::WHITESPACE;
}

V8_INLINE Token::Value Scanner::ScanSingleToken() {
  Token::Value token;
  do {
    next().location.beg_pos = source_pos();

    if (static_cast<unsigned>(c0_) <= 0x7F) {
      Token::Value token = one_char_tokens[c0_];
      if (token != Token::ILLEGAL) {
        Advance();
        return token;
      }
    }

    switch (c0_) {
      case '"':
      case '\'':
        return ScanString();

      case '<':
        // < <= << <<= <!--
        Advance();
        if (c0_ == '=') return Select(Token::LTE);
        if (c0_ == '<') return Select('=', Token::ASSIGN_SHL, Token::SHL);
        if (c0_ == '!') {
          token = ScanHtmlComment();
          continue;
        }
        return Token::LT;

      case '>':
        // > >= >> >>= >>> >>>=
        Advance();
        if (c0_ == '=') return Select(Token::GTE);
        if (c0_ == '>') {
          // >> >>= >>> >>>=
          Advance();
          if (c0_ == '=') return Select(Token::ASSIGN_SAR);
          if (c0_ == '>') return Select('=', Token::ASSIGN_SHR, Token::SHR);
          return Token::SAR;
        }
        return Token::GT;

      case '=':
        // = == === =>
        Advance();
        if (c0_ == '=') return Select('=', Token::EQ_STRICT, Token::EQ);
        if (c0_ == '>') return Select(Token::ARROW);
        return Token::ASSIGN;

      case '!':
        // ! != !==
        Advance();
        if (c0_ == '=') return Select('=', Token::NE_STRICT, Token::NE);
        return Token::NOT;

      case '+':
        // + ++ +=
        Advance();
        if (c0_ == '+') return Select(Token::INC);
        if (c0_ == '=') return Select(Token::ASSIGN_ADD);
        return Token::ADD;

      case '-':
        // - -- --> -=
        Advance();
        if (c0_ == '-') {
          Advance();
          if (c0_ == '>' && next().after_line_terminator) {
            // For compatibility with SpiderMonkey, we skip lines that
            // start with an HTML comment end '-->'.
            token = SkipSingleHTMLComment();
            continue;
          }
          return Token::DEC;
        }
        if (c0_ == '=') return Select(Token::ASSIGN_SUB);
        return Token::SUB;

      case '*':
        // * *=
        Advance();
        if (c0_ == '*') return Select('=', Token::ASSIGN_EXP, Token::EXP);
        if (c0_ == '=') return Select(Token::ASSIGN_MUL);
        return Token::MUL;

      case '%':
        // % %=
        return Select('=', Token::ASSIGN_MOD, Token::MOD);

      case '/':
        // /  // /* /=
        Advance();
        if (c0_ == '/') {
          uc32 c = Peek();
          if (c == '#' || c == '@') {
            Advance();
            Advance();
            token = SkipSourceURLComment();
            continue;
          }
          token = SkipSingleLineComment();
          continue;
        }
        if (c0_ == '*') {
          token = SkipMultiLineComment();
          continue;
        }
        if (c0_ == '=') return Select(Token::ASSIGN_DIV);
        return Token::DIV;

      case '&':
        // & && &=
        Advance();
        if (c0_ == '&') return Select(Token::AND);
        if (c0_ == '=') return Select(Token::ASSIGN_BIT_AND);
        return Token::BIT_AND;

      case '|':
        // | || |=
        Advance();
        if (c0_ == '|') return Select(Token::OR);
        if (c0_ == '=') return Select(Token::ASSIGN_BIT_OR);
        return Token::BIT_OR;

      case '^':
        // ^ ^=
        return Select('=', Token::ASSIGN_BIT_XOR, Token::BIT_XOR);

      case '.':
        // . Number
        Advance();
        if (IsDecimalDigit(c0_)) return ScanNumber(true);
        if (c0_ == '.') {
          if (Peek() == '.') {
            Advance();
            Advance();
            return Token::ELLIPSIS;
          }
        }
        return Token::PERIOD;

      case '`':
        Advance();
        return ScanTemplateSpan();

      case '#':
        return ScanPrivateName();

      default:
        if (unicode_cache_->IsIdentifierStart(c0_) ||
            (CombineSurrogatePair() &&
             unicode_cache_->IsIdentifierStart(c0_))) {
          return ScanIdentifierOrKeyword();
        }
        if (IsDecimalDigit(c0_)) return ScanNumber(false);
        if (c0_ == kEndOfInput) {
          return source_->has_parser_error() ? Token::ILLEGAL : Token::EOS;
        }
        token = SkipWhiteSpace();
        continue;
    }
    // Continue scanning for tokens as long as we're just skipping whitespace.
  } while (token == Token::WHITESPACE);

  return token;
}

void Scanner::Scan(TokenDesc* next_desc) {
  DCHECK_EQ(next_desc, &next());

  next_desc->literal_chars.Drop();
  next_desc->raw_literal_chars.Drop();
  next_desc->invalid_template_escape_message = MessageTemplate::kNone;

  next_desc->token = ScanSingleToken();
  DCHECK_IMPLIES(has_parser_error(), next_desc->token == Token::ILLEGAL);
  next_desc->location.end_pos = source_pos();

#ifdef DEBUG
  SanityCheckTokenDesc(current());
  SanityCheckTokenDesc(next());
  SanityCheckTokenDesc(next_next());
#endif
}

void Scanner::Scan() { Scan(next_); }

}  // namespace internal
}  // namespace v8

#endif  // V8_PARSING_SCANNER_INL_H_
