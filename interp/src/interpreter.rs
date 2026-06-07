use crate::ast::{BinaryOp, Expr, Program, Stmt, UnaryOp};
use std::collections::HashMap;

pub struct Interpreter {
    env: HashMap<String, f64>,
}

impl Interpreter {
    pub fn new() -> Self {
        Self {
            env: HashMap::new(),
        }
    }

    pub fn run(&mut self, program: &Program) -> Result<Option<f64>, String> {
        let mut last = None;

        for stmt in &program.statements {
            last = Some(self.execute(stmt)?);
        }

        Ok(last)
    }

    fn execute(&mut self, stmt: &Stmt) -> Result<f64, String> {
        match stmt {
            Stmt::Let { name, value } => {
                let value = self.eval(value)?;
                self.env.insert(name.clone(), value);
                Ok(value)
            }
            Stmt::Print(expr) => {
                let value = self.eval(expr)?;
                println!("{value}");
                Ok(value)
            }
            Stmt::Expr(expr) => self.eval(expr),
        }
    }

    fn eval(&mut self, expr: &Expr) -> Result<f64, String> {
        match expr {
            Expr::Number(value) => Ok(*value),
            Expr::Variable(name) => self
                .env
                .get(name)
                .copied()
                .ok_or_else(|| format!("undefined variable `{name}`")),
            Expr::Unary { op, expr } => {
                let value = self.eval(expr)?;
                match op {
                    UnaryOp::Negate => Ok(-value),
                    UnaryOp::Plus => Ok(value),
                }
            }
            Expr::Binary { left, op, right } => {
                let left = self.eval(left)?;
                let right = self.eval(right)?;
                match op {
                    BinaryOp::Add => Ok(left + right),
                    BinaryOp::Subtract => Ok(left - right),
                    BinaryOp::Multiply => Ok(left * right),
                    BinaryOp::Divide => Ok(left / right),
                }
            }
        }
    }
}
