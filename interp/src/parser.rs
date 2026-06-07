use crate::ast::{BinaryOp, Expr, Program, Stmt, UnaryOp};
use crate::token::{Token, TokenKind};

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, current: 0}
    }

    pub fn parse_program(&mut self) -> Result<Program, String> {
        let mut statements: Vec<Stmt> = Vec::new();

        while !self.is_at_end() {
            statements.push(self.statement()?);
        }

        Ok(Program { statements })
    }

    fn statement(&mut self) -> Result<Stmt, String> {
        if self.matches(&TokenKind::Let) {
            return self.let_statement();
        }

        if self.matches(&TokenKind::Print) {
            let value: Expr = self.expression()?;
            self.consume(&TokenKind::Semicolon, "expected `;` after print statement")?;
            return Ok(Stmt::Print(value));
        }

        let expr: Expr = self.expression()?;
        self.consume(&TokenKind::Semicolon, "expected `;` after expression")?;
        Ok(Stmt::Expr(expr))
    }

    fn let_statement(&mut self) -> Result<Stmt, String> {
        let name = match self.advance().kind.clone() {
            TokenKind::Ident(name) => name,
            _ => return Err(self.error("expected identifier afet `let`")),
        };

        self.consume(&TokenKind::Equal, "expected `=` after variable name")?;
        let value = self.expression()?;
        self.consume(&TokenKind::Semicolon, "expected `;` after let statement")?;

        Ok(Stmt::Let { name, value })
    }

    fn expression(&mut self) -> Result<Expr, String> {
        self.term()
    }

    fn term(&mut self) -> Result<Expr, String> {
        let mut expr = self.factor()?;

        while self.matches_any(&[TokenKind::Plus, TokenKind::Minus]) {
            let op = match self.previous().kind {
                TokenKind::Plus => BinaryOp::Add,
                TokenKind::Minus => BinaryOp::Subtract,
                _ => unreachable!(),
            };
            let right = self.factor()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn factor(&mut self) -> Result<Expr, String> {
        let mut expr = self.unary()?;

        while self.matches_any(&[TokenKind::Star, TokenKind::Slash]) {
            let op = match self.previous().kind {
                TokenKind::Star => BinaryOp::Multiply,
                TokenKind::Slash => BinaryOp::Divide,
                _ => unreachable!(),
            };
            let right = self.unary()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op,
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn unary(&mut self) -> Result<Expr, String> {
        if self.matches_any(&[TokenKind::Minus, TokenKind::Plus]) {
            let op = match self.previous().kind {
                TokenKind::Minus => UnaryOp::Negate,
                TokenKind::Plus => UnaryOp::Plus,
                _ => unreachable!(),
            };
            let expr = self.unary()?;
            return Ok(Expr::Unary {
                op,
                expr: Box::new(expr),
            });
        }

        self.primary()
    }

    fn primary(&mut self) -> Result<Expr, String> {
        match self.advance().kind.clone() {
            TokenKind::Number(value) => Ok(Expr::Number(value)),
            TokenKind::Ident(name) => Ok(Expr::Variable(name)),
            TokenKind::LParen => {
                let expr = self.expression()?;
                self.consume(&TokenKind::RParen, "expected `)` after expression")?;
                Ok(expr)
            }
            _ => Err(self.error("expected expression")),
        }
    }

    fn consume(&mut self, kind: &TokenKind, message: &str) -> Result<(), String> {
        if self.check(kind) {
            self.advance();
            Ok(())
        } else {
            Err(self.error(message))
        }
    }

    fn matches(&mut self, kind: &TokenKind) -> bool {
        if self.check(kind) {
            self.advance();
            true
        } else {
            false
        }
    }

    fn matches_any(&mut self, kinds: &[TokenKind]) -> bool {
        for kind in kinds {
            if self.check(kind) {
                self.advance();
                return true;
            }
        }
        false
    }

    fn check(&self, kind: &TokenKind) -> bool {
        if self.is_at_end() {
            return false;
        }

        token_kind_matches(&self.peek().kind, kind)
    }

    fn advance(&mut self) -> &Token {
        if !self.is_at_end() {
            self.current += 1;
        }
        self.previous()
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.current]
    }

    fn previous(&self) -> &Token {
        &self.tokens[self.current - 1]
    }

    fn is_at_end(&self) -> bool {
        matches!(self.peek().kind, TokenKind::Eof)
    }

    fn error(&self, message: &str) -> String {
        format!("{message} at token {:?}", self.peek())
    }
}

fn token_kind_matches(a: &TokenKind, b: &TokenKind) -> bool {
    matches!(
        (a, b),
        (TokenKind::Let, TokenKind::Let)
        | (TokenKind::Print, TokenKind::Print)
        | (TokenKind::Plus, TokenKind::Plus)
        | (TokenKind::Minus, TokenKind::Minus)
        | (TokenKind::Star, TokenKind::Star)
        | (TokenKind::Slash, TokenKind::Slash)
        | (TokenKind::Equal, TokenKind::Equal)
        | (TokenKind::LParen, TokenKind::LParen)
        | (TokenKind::RParen, TokenKind::RParen)
        | (TokenKind::Semicolon, TokenKind::Semicolon)
        | (TokenKind::Eof, TokenKind::Eof)
        | (TokenKind::Number(_), TokenKind::Number(_))
        | (TokenKind::Ident(_), TokenKind::Ident(_))
    )
}
