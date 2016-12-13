# ATLAS
A Typeless Lambda cAlculus Simulator

Currently finished in C#, still developing in C++17, template is really hard.

##Usage

You should know the Nicolas de Bruijin representation of lambda first. (Using number to represent each variable)

the number *k* represents *the variable that enclosed by the kth lambda* 

for example:

\\a.a can be represented as \\.0

\\f.\\s.s can be represented as \\.\\.0 

and \\f.\\s.fs is same as \\.\\.10 

when it comes to free variable, first, give it a unique number that is greater than the number of *lambda*s, then when using it in the lambda expression, add it by the number of *lamnda*s which surround it.

for example:

Mapping of free variable to number:

y -> 3

then

\\w.y w is represented as \\.4 0, in which 4 comes from the origin number of free variable **3** + the number of *lambda* that surround the variable **1**

##Usage in code (C#)

construct your lambda with static class Operation or just *new* it.

```C#
var succ = Abs(Abs(Abs(Var(1) * (Var(2) * Var(1) * Var(0)))));
```

Abs means abstraction, namely function

you can use operator* to replace construction of Application

then you evaluate

```C#
Console.WriteLine($"one:  {(succ*zero).Eval()}");
```

or see a step by step evaluation

```C#
Console.WriteLine($"one:  {(succ*zero).EvalOnce()}");
```

the parameter Context is currently not used

all the method is pure method and objecs are immutable, so you need to store it in a temporary reference or use chained invocation.
