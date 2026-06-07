use crate::token::{Token, TokenKind};

pub struct Lexer<'a> {
    chars: Vec<char>,
    start: usize,
    current: usize,
    _source: &'a str,
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a str) -> Self {
        Self {
            chars: source.chars().collect(),
            start: 0,
            current: 0,
            _source: source,
        }
    }

    pub fn tokenize(&mut self) -> Result<Vec<Token>, String> {
        let mut tokens = Vec::new();

        while !self.is_at_end() {
            self.start = self.current;
            let token = self.scan_token()?;
            if let Some(token) = token {
                tokens.push(token);
            }
        }

        tokens.push(Token {
            kind: TokenKind::Eof,
            lexeme: String::new(),
            pos: self.current,
        });

        Ok(tokens)
    }

    fn scan_token(&mut self) -> Result<Option<Token>, String> {
        let c = self.advance();
        let token = match c {
            '(' => Some(self.simple(TokenKind::LParen)),
            ')' => Some(self.simple(TokenKind::RParen)),
            '+' => Some(self.simple(TokenKind::Plus)),
            '-' => Some(self.simple(TokenKind::Minus)),
            '*' => Some(self.simple(TokenKind::Star)),
            '/' => Some(self.simple(TokenKind::Slash)),
            '=' => Some(self.simple(TokenKind::Equal)),
            ';' => Some(self.simple(TokenKind::Semicolon)),
            ' ' | '\r' | '\t' | '\n' => None,
            c if c.is_ascii_digit() => Some(self.number()),
            c if is_ident_start(c) => Some(self.identifier()),
            _ => return Err(format!("unexpected character `{c}` at {}", self.current - 1)),
        };

        Ok(token)
    }

    fn simple(&self, kind: TokenKind) -> Token {
        Token {
            kind,
            lexeme: self.current_lexeme(),
            pos: self.start,
        }
    }

    fn number(&mut self) -> Token {
        while self.peek().is_ascii_digit() {
            self.advance();
        }

        if self.peek() == '.' && self.peek_next().is_ascii_digit() {
            self.advance();
            while self.peek().is_ascii_digit() {
                self.advance();
            }
        }

        let lexeme = self.current_lexeme();
        let value = lexeme.parse::<f64>().unwrap();
        Token {
            kind: TokenKind::Number(value),
            lexeme,
            pos: self.start,
        }

    }

    fn identifier(&mut self) -> Token {
        while is_ident_continue(self.peek()) {
            self.advance();
        }

        let lexeme = self.current_lexeme();
        let kind = match lexeme.as_str() {
            "let" => TokenKind::Let,
            "print" => TokenKind::Print,
            _ => TokenKind::Ident(lexeme.clone()),
        };

        Token {
            kind,
            lexeme,
            pos: self.start,
        }
    }

    fn current_lexeme(&self) -> String {
        self.chars[self.start..self.current].iter().collect()
    }

    fn advance(&mut self) -> char {
        let c = self.chars[self.current];
        self.current += 1;
        c
    }

    fn peek(&self) -> char {
        self.chars.get(self.current).copied().unwrap_or('\0')
    }

    fn peek_next(&self) -> char {
        self.chars.get(self.current + 1).copied().unwrap_or('\0')
    }

    fn is_at_end(&self) -> bool {
        self.current >= self.chars.len()
    }
}

fn is_ident_start(c: char) -> bool {
    c.is_ascii_alphabetic() || c == '_'
}

fn is_ident_continue(c: char) -> bool {
    c.is_ascii_alphanumeric() || c == '_'
}
