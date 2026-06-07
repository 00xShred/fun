#[derive(Debug, Clone, PartialEq)]
pub enum TokenKind {
    Let,
    Print,
    Ident(String),
    Number(f64),
    Plus,
    Minus,
    Star,
    Slash,
    Equal,
    LParen,
    RParen,
    Semicolon,
    Eof,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Token {
    pub kind: TokenKind,
    pub lexeme: String,
    pub pos: usize,
}
